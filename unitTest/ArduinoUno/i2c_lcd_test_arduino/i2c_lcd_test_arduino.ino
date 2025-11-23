#include <LiquidCrystal_PCF8574.h>

LiquidCrystal_PCF8574 lcd(0x27);  // Dirección típica

void setup() {
  lcd.begin(16, 2);   // Columnas, filas
  lcd.setBacklight(255);

  lcd.setCursor(0, 0);
  lcd.print("Hola Edgar!");
  lcd.setCursor(0, 1);
  lcd.print("LCD OK :)");
}

void loop() {}

