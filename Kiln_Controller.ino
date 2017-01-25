#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <max6675.h>
#include "Adafruit_MCP23017.h"

int MISO0 = 12;
int CLK0 = 13;
int CS0 = 16;

LiquidCrystal_I2C lcd(0x27, 16, 2);
MAX6675 t0(CLK0, CS0, MISO0);
Adafruit_MCP23017 mcp;

void setup() {
  Wire.pins(4,5);
  
  // LCD
  lcd.init();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Initializing... ");

  // MCP23017 GPIO expander
  mcp.begin();
  mcp.pinMode(0, OUTPUT);
  mcp.digitalWrite(0, HIGH);
  delay(100);
  mcp.digitalWrite(0, LOW);

  // Clear LCD
  lcd.setCursor(0,0);
  lcd.print("                ");
}

void loop() {
  // Get value
  lcd.setCursor(0,0);
  lcd.print(t0.readFahrenheit());

  // LED stuff
  lcd.setCursor(0,1);
  lcd.print("LED: ON ");
  mcp.digitalWrite(0, HIGH);
  delay(1000);
  lcd.setCursor(0,1);
  lcd.print("LED: OFF");
  mcp.digitalWrite(0, LOW);
  delay(1000);
  
}
