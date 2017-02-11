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
  server.send(200, "text/html",
              "<h1>Kiln Controller V1</h1>"
              "<a href=\"/temps\">Real Time Temperature Readout</a><br>"
              "<a href=\"/settings\">Temperature Setpoints</a><br>"
              "<a href=\"/config\">Configuration</a><br>"
             );
}

void handleTemps() {
  server.send(200, "text/html",
              "<h1>Temperature Readout</h1>"
              "<a href=\"/\">Home</a>"
             );
}

void handleSettings() {
  server.send(200, "text/html",
              "<h1>Settings</h1>"
              "<a href=\"/\">Home</a>"
             );
}

void handleConfig() {
  server.send(200, "text/html",
              "<h1>Configuration</h1>"
              "<a href=\"/\">Home</a>"
             );
}

void handleNotFound() {
  String temp = "File Not Found\n\n";
  temp += "URI: ";
  temp += server.uri();
  temp += "\nMethod: ";
  temp += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  temp += "\nArguments: ";
  temp += server.args();
  temp += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    temp += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", temp );
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
  server.on("/temps", handleTemps);
  server.on("/settings", handleSettings);
  server.on("/config", handleConfig);
  server.on ("/test.svg", tempGraph);
  server.onNotFound(handleNotFound);
  server.begin();
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("SSID: "); lcd.print(ssid);
  lcd.setCursor(0, 1); lcd.print("PSWD: "); lcd.print(password);
  lcd.setCursor(0, 2); lcd.print("IP  : "); lcd.print(myIP);
  delay(2000);

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
  if (WiFi.softAPgetStationNum() == 0) {
    digitalWrite(2, LOW);
    delay(100);
    digitalWrite(2, HIGH);
    delay(900);
  } else {
    digitalWrite(2, LOW);
    delay(50);
    digitalWrite(2, HIGH);
    delay(100);
    digitalWrite(2, LOW);
    delay(50);
    digitalWrite(2, HIGH);
    delay(800);
  }
}

void drawGraph(int xvalues[], int yvalues[], int width, int height, int r, int g, int b) {
  String out = "";
  char temp[100];

  sprintf(temp, "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"%d\" height=\"%d\">\n", width, height);
  out += temp;

  sprintf(temp, "<rect width=\"%d\" height=\"%d\" fill=\"rgb(%d,%d,%d)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n", width, height, r, g, b);
  out += temp;

  out += "<g stroke=\"black\">\n";
  int y = rand() % 130;
  //  for (int i = 0; i < size(times); i ++) {
  //    int y2 = rand() % 130;
  //    sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
  //    out += temp;
  //    y = y2;
  //  }
  out += "</g>\n</svg>\n";

  server.send ( 200, "image/svg+xml", out);
}

void tempGraph() {
  int times[] = {1, 2, 3, 4, 5, 6};
  int temps[] = {1, 2, 3, 1, 5, 3};
  drawGraph(times, temps, 400, 150, 232, 240, 255);
}
