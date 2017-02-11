#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <max6675.h>
#include "Adafruit_MCP23017.h"

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

///// USER SETTINGS /////

// WiFi Settings
const char *ssid = "ESPap";
const char *password = "thereisnospoon";

// Temperature Offsets
int t1off = 0;
int t2off = 0;
int t3off = 0;

// Pinout
int CLK0 = 13;
int MISO0 = 12;
int CS1 = 16;
int CS2 = 14;
int CS3 = 15;

///// END USER SETTINGS /////

// Thermocouple stuff
MAX6675 t1(CLK0, CS1, MISO0);
MAX6675 t2(CLK0, CS2, MISO0);
MAX6675 t3(CLK0, CS3, MISO0);
int temp1, temp2, temp3;

// Output Stuff
LiquidCrystal_I2C lcd(0x3f, 20, 4);
Adafruit_MCP23017 mcp;

// WiFi Stuff
ESP8266WebServer server(80);

// Control Stuff
int onTemp = 70;
int offTemp = 80;

///// WEB SERVER /////
void handleRoot() {
  server.send(200, "text/html", ""
              "<h1>Kiln Controller V1</h1>"
              "<a href=\"/page2\">Page 2</a>"
              "");
}

void handlePage2() {
  server.send(200, "text/html", ""
              "<h1>Page 2</h1>"
              "<a href=\"/\">Home</a>"
              "");
}

///// SETUP /////
void setup() {
  // Onboard LED
  pinMode(2, OUTPUT);

  // I2C
  Wire.pins(4, 5);

  // LCD
  lcd.init();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing... ");

  // MCP23017 GPIO expander
  mcp.begin();
  mcp.pinMode(0, OUTPUT);
  mcp.pinMode(1, OUTPUT);
  mcp.pinMode(2, OUTPUT);
  mcp.pinMode(3, OUTPUT);
  mcp.pinMode(4, OUTPUT);
  mcp.digitalWrite(0, HIGH);
  mcp.digitalWrite(1, HIGH);
  mcp.digitalWrite(2, HIGH);
  mcp.digitalWrite(3, HIGH);
  delay(100);
  mcp.digitalWrite(0, LOW);
  mcp.digitalWrite(1, LOW);
  mcp.digitalWrite(2, LOW);
  mcp.digitalWrite(3, LOW);

  // Temperature Inital Values
  temp1 = t1.readFahrenheit();
  temp2 = t2.readFahrenheit();
  temp3 = t3.readFahrenheit();

  // Access Point Setup
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  server.on("/", handleRoot);
  server.on("/page2", handlePage2);
  server.begin();
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("SSID: "); lcd.print(ssid);
  lcd.setCursor(0, 1); lcd.print("PSWD: "); lcd.print(password);
  lcd.setCursor(0, 2); lcd.print("IP  : "); lcd.print(myIP);

  delay(1000);

  // Clear LCD
  lcd.setCursor(0, 0);
  lcd.clear();
}

void loop() {
  server.handleClient();

  // Get values
  temp1 = t1.readFahrenheit() + t1off;
  temp2 = t2.readFahrenheit() + t2off;
  temp3 = t3.readFahrenheit() + t3off;

  // Write to LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1: ");
  lcd.print(temp1);
  lcd.print(" F");
  lcd.setCursor(0, 1);
  lcd.print("2: ");
  lcd.print(temp2);
  lcd.print(" F");
  lcd.setCursor(0, 2);
  lcd.print("3: ");
  lcd.print(temp3);
  lcd.print(" F");

  // Heartbeat LED
  digitalWrite(2, LOW);
  delay(100);
  digitalWrite(2, HIGH);
  delay(900);
}
