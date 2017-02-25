#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <max6675.h>
#include "Adafruit_MCP23017.h"

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>


/////////////////////////
///// USER SETTINGS /////
/////////////////////////


// WiFi Settings
const char *ssid = "KilnControlV1";
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


//////////////////////////////
///// END: USER SETTINGS /////
//////////////////////////////


// Thermocouple
MAX6675 t1(CLK0, CS1, MISO0);
MAX6675 t2(CLK0, CS2, MISO0);
MAX6675 t3(CLK0, CS3, MISO0);
int temp1, temp2, temp3;

// Control
int onTemp = 70;
int offTemp = 80;

// Output
LiquidCrystal_I2C lcd(0x3f, 20, 4);
Adafruit_MCP23017 mcp;

//////////////////////
///// WEB SERVER /////
//////////////////////

// WiFi
ESP8266WebServer server(80);

const String header = "<head><title>Kiln Controller V1</title></head>";
const String navbar = "<table border=\"1\"><nav><tr><td><a href=\"/\">Home</a></td><td><a href=\"/temps\">Temperature Display</a></td><td><a href=\"/settings\">Settings</a></td><td><a href=\"/config\">Configuration</a></td></tr></nav></table>";

void handleRoot() {
  server.send(200, "text/html",
              header + "<h1>Kiln Controller V1</h1>" + navbar
             );
}

void handleTemps() {
  server.send(200, "text/html",
              header + "<h1>Kiln Controller V1</h1><h2>Temperature Display</h2>" + navbar +
              "<br><img src=\"/tempgraph.svg\">"
             );
}

void handleSettings() {
  server.send(200, "text/html",
              header + "<h1>Kiln Controller V1</h1><h2>Settings</h2>" + navbar +
              "<form name=\"myform\" action=\"/settings\" method=\"post\">"
              "<br>" +
              "<button type=\"submit\">Submit</button>" +
              "</form>"
             );
}

void handleSettingsUpdate() {
  mcp.digitalWrite(1, HIGH);
}

void handleConfig() {
  server.send(200, "text/html",
              header + "<h1>Configuration</h1>" + navbar
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


/////////////////
///// SETUP /////
/////////////////


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
  temp1 = 0;
  temp2 = 0;
  temp3 = 0;

  // Access Point Setup
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  server.on("/", handleRoot);
  server.on("/temps", handleTemps);
  server.on("/settings", HTTP_GET, handleSettings);
  server.on("/settings", HTTP_POST, handleSettingsUpdate);
  server.on("/config", handleConfig);
  server.on ("/tempgraph.svg", tempGraph);
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
  int ntemp1 = t1.readFahrenheit() + t1off;
  int ntemp2 = t2.readFahrenheit() + t2off;
  int ntemp3 = t3.readFahrenheit() + t3off;
  temp1 = (ntemp1 != 2147483647 ? ntemp1 : temp1);
  temp2 = (ntemp2 != 2147483647 ? ntemp2 : temp2);
  temp3 = (ntemp3 != 2147483647 ? ntemp3 : temp3);

  // Write to LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printf("1:%4d%cF", temp1, 0xDF);
  lcd.setCursor(0, 1);
  lcd.printf("2:%4d%cF", temp2, 0xDF);
  lcd.setCursor(0, 2);
  lcd.printf("3:%4d%cF", temp3, 0xDF);
  // Heartbeat LED
  if (WiFi.softAPgetStationNum() == 0) {
    digitalWrite(2, LOW);
    delay(10);
    digitalWrite(2, HIGH);
    delay(990);
  } else {
    digitalWrite(2, LOW);
    delay(5);
    digitalWrite(2, HIGH);
    delay(100);
    digitalWrite(2, LOW);
    delay(1);
    digitalWrite(2, HIGH);
    delay(980);
  }
}


////////////////////
///// PLOTTING /////
////////////////////

/**
   x[] is x values
   y[] is y values corresponding to x values
   len is the length of both x and y
*/
void drawGraph(int x[], int y[], int len) {
  String out = "";
  char temp[100];

  // Find max/min x,y values
  int maxY = y[0];
  int minY = y[0];
  for (int i = 1; i < len; i++) {
    if (y[i] > maxY) {
      maxY = y[i];
    } else if (y[i] < minY) {
      minY = y[i];
    }
  }

  int maxX = x[0];
  int minX = x[0];
  for (int i = 1; i < len; i++) {
    if (x[i] > maxX) {
      maxX = x[i];
    } else if (x[i] < minX) {
      minY = x[i];
    }
  }

  float yscale = 92.0 / (maxY - minY);
  float xscale = 484.0 / (maxX - minX);


  // Chart area
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"560\" height=\"195\">\n";
  out += "<rect width=\"560\" height=\"195\" style=\"fill:rgb(232,240,255);stroke-width:2;stroke:rgb(0,0,0)\"/>\n";
  out += "<rect x=\"50\" y=\"10\" width=\"500\" height=\"110\" style=\"fill:white;stroke-width:1;stroke:rgb(0,0,0)\"/>";

  // Vertical axis labels
  out += "<text x=\"10\" y=\"66\" fill=\"black\">F</text>";
  sprintf(temp, "<text x=\"10\" y=\"25\" fill=\"black\">%d</text>", maxY);
  out += temp;
  sprintf(temp, "<text x=\"10\" y=\"117\" fill=\"black\">%d</text>", minY);
  out += temp;
  out += "<line x1=\"45\" y1=\"19\" x2=\"55\" y2=\"19\" style=\"stroke:black;stroke-width:1\"></line>";
  out += "<line x1=\"45\" y1=\"111\" x2=\"55\" y2=\"111\" style=\"stroke:black;stroke-width:1\"></line>";

  // Horizontal axis labels
  int horizLabelSpacing = (maxX - minX) / 4;
  out += "<text x=\"275\" y=\"185\" fill=\"black\">HH:MM</text>";
  sprintf(temp, "<text x=\"53\" y=\"130\" fill=\"black\" transform=\"rotate(90 53,130)\">%02d:%02d </text>", minX / 60, minX % 60);
  out += temp;
  sprintf(temp, "<text x=\"174\" y=\"130\" fill=\"black\" transform=\"rotate(90 174,130)\">%02d:%02d </text>", (1 * horizLabelSpacing + minX) / 60, (1 * horizLabelSpacing + minX) % 60);
  out += temp;
  sprintf(temp, "<text x=\"295\" y=\"130\" fill=\"black\" transform=\"rotate(90 295,130)\">%02d:%02d </text>", (2 * horizLabelSpacing + minX) / 60, (2 * horizLabelSpacing + minX) % 60);
  out += temp;
  sprintf(temp, "<text x=\"416\" y=\"130\" fill=\"black\" transform=\"rotate(90 416,130)\">%02d:%02d </text>", (3 * horizLabelSpacing + minX) / 60, (3 * horizLabelSpacing + minX) % 60);
  out += temp;
  sprintf(temp, "<text x=\"535\" y=\"130\" fill=\"black\" transform=\"rotate(90 535,130)\">%02d:%02d </text>", maxX / 60, maxX % 60);
  out += temp;
  out += "<line x1=\"58\" y1=\"115\" x2=\"58\" y2=\"125\" style=\"stroke:black;stroke-width:1\"></line>";
  out += "<line x1=\"179\" y1=\"115\" x2=\"179\" y2=\"125\" style=\"stroke:black;stroke-width:1\"></line>";
  out += "<line x1=\"300\" y1=\"115\" x2=\"300\" y2=\"125\" style=\"stroke:black;stroke-width:1\"></line>";
  out += "<line x1=\"421\" y1=\"115\" x2=\"421\" y2=\"125\" style=\"stroke:black;stroke-width:1\"></line>";
  out += "<line x1=\"540\" y1=\"115\" x2=\"540\" y2=\"125\" style=\"stroke:black;stroke-width:1\"></line>";
  out += "Sorry, your browser does not support inline SVG.";


  // Plot polyline
  out += "<g stroke=\"black\">\n";
  out += "<polyline points=\"";
  for (int i = 0; i < len; i++) {
    sprintf(temp, "%d,%d ", (int)(58 + (xscale * (x[i] - minX))), (int)(111 - (yscale * (y[i] - minY))));
    out += temp;
  }
  out += "\" style=\"fill:none;stroke:black;stroke-width:2\" />";
  out += "</g>\n</svg>\n";


  server.send ( 200, "image/svg+xml", out);
}

void tempGraph() {
  lcd.clear();
  int times[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100};
  int temps[] = {70, 75, 81, 86, 90, 94, 97, 100, 103, 107,
                 110, 113, 116, 130, 146, 160, 173, 186, 194, 205,
                 220, 240, 255, 268, 282, 300, 300, 336, 335
                };
  drawGraph(times, temps, 29);


  lcd.setCursor(0, 0);
  for (int i = 0; i < 6; i++) {
    lcd.print(times[i]);
    lcd.print(" ");
  }
  lcd.setCursor(0, 1);
  for (int i = 0; i < sizeof(times) / sizeof(int); i++) {
    lcd.print(temps[i]);
    lcd.print(" ");
  }
}
