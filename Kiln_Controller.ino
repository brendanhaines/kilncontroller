#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <max6675.h>
#include "Adafruit_MCP23017.h"

int MISO0 = 12;
int CLK0 = 13;
int CS0 = 16;

LiquidCrystal_I2C lcd(0x3f, 20, 4);
MAX6675 t0(CLK0, CS0, MISO0);
Adafruit_MCP23017 mcp;

void setup() {
  // Onboard LED
  pinMode(2,OUTPUT);

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

  // Clear LCD
  lcd.setCursor(0, 0);
  lcd.clear();
}

void loop() {
  // Get value
  lcd.setCursor(0, 0);
  lcd.print(t0.readFahrenheit());
  lcd.print("F ");
  lcd.print(t0.readCelsius());
  lcd.print("C");

  digitalWrite(2, LOW);
  delay(100);
  digitalWrite(2, HIGH);
  delay(1000);
}
