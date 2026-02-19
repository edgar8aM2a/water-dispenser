#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

// Cambia 0x27 si tu módulo tiene otra dirección (por ejemplo 0x3F)
LiquidCrystal_PCF8574 lcd(0x27);

const uint8_t LCD_WIDTH = 16;

void lcdString(const char* message, uint8_t line) {
  lcd.setCursor(0, line);

  uint8_t i = 0;
  while (i < LCD_WIDTH && message[i] != '\0') {
    lcd.print(message[i]);
    i++;
  }
  while (i < LCD_WIDTH) {
    lcd.print(' ');
    i++;
  }
}

void setup() {
  // ESP32 I2C pins
  const int SDA_PIN = 21;
  const int SCL_PIN = 22;

  // Inicia I2C con pines específicos del ESP32
  Wire.begin(SDA_PIN, SCL_PIN);

  // Inicializa el LCD 16x2
  lcd.begin(16, 2);
  lcd.setBacklight(1);

  lcdString("Hola Edgar!", 0);
  lcdString("Tu LCD funciona", 1);
}

void loop() {
  // Nada
}

