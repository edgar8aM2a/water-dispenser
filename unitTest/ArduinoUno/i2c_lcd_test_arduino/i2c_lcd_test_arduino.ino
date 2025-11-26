#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

// Cambia 0x27 si tu módulo tiene otra dirección (por ejemplo 0x3F)
LiquidCrystal_PCF8574 lcd(0x27);

// Ancho del display (como LCD_WIDTH = 16 en tu Python)
const uint8_t LCD_WIDTH = 16;

// Función estilo "lcd_string" de tu script en Python
void lcdString(const char* message, uint8_t line) {
  // line = 0 para primera fila, 1 para segunda
  lcd.setCursor(0, line);

  // Escribimos hasta 16 caracteres, rellenando con espacios si hace falta
  uint8_t i = 0;
  while (i < LCD_WIDTH && message[i] != '\0') {
    lcd.print(message[i]);
    i++;
  }
  while (i < LCD_WIDTH) {  // rellena con espacios, como .ljust()
    lcd.print(' ');
    i++;
  }
}

void setup() {
  // Inicializa el LCD de 16x2
  lcd.begin(16, 2);
  // Enciende el backlight (255 = máximo brillo)
  lcd.setBacklight(255);

  // Equivalente a:
  // lcd_string("Hola Edgar!", LCD_LINE_1)
  // lcd_string("Tu LCD funciona", LCD_LINE_2)
  lcdString("Hola Edgar!", 0);        // línea 1
  lcdString("Tu LCD funciona", 1);    // línea 2
}

void loop() {
  // No hacemos nada en loop, el texto se queda en pantalla
}


