#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <max6675.h>
#include "Adafruit_MCP23017.h"

int MISO0 = 12;
int CLK0 = 13;
int CS0 = 2;

LiquidCrystal_I2C lcd(0x27, 16, 2);
MAX6675 t0(CLK0, CS0, MISO0);
Adafruit_MCP23017 mcp;

void setup() {
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
  mcp.pinMode(8, OUTPUT);
  mcp.digitalWrite(8, HIGH);
  delay(100);
  mcp.digitalWrite(8, LOW);

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

  // LED stuff
  lcd.setCursor(0, 1);
  lcd.print("LED: ON ");
  mcp.digitalWrite(8, HIGH);
  delay(2000);
  lcd.setCursor(0, 1);
  lcd.print("LED: OFF");
  mcp.digitalWrite(8, LOW);
  delay(2000);

}
