// coins_counter_simplified_arduino.ino
// Esqueleto basado en tu coins_counter_test.py

#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

// Configuracion del display
LiquidCrystal_PCF8574 lcd(0x27);
const uint8_t LCD_WIDTH = 16; // Ancho del display 

// --- Pines  ---
// Contador de Monedas (INT)
const int PIN_MONEDAS       = 2;  // interrupción
// Sensor de flujo
const int PIN_FLUJO         = 3;
// Botones
const int PIN_BTN_GALON     = 4;
const int PIN_BTN_GARRAFON  = 5;
const int PIN_BTN_LIMPIAR   = 6;
const int PIN_BTN_CANCELAR  = 7;
// Relays_bank
const int PIN_BOMBA         = 8;
const int PIN_HOPPER        = 9;
// Contador de cambio (hopper) – por ahora solo declarado
const int PIN_IR_HOPPER     = 11;


// --- Precios ---
const int PRECIO_GALON    = 10;
const int PRECIO_GARRAFON = 20;

// --- Estado ---
volatile int credito = 0;

// ISR: cada pulso = $10 (ajusta según tu monedero)
void monedaInsertada() {
  credito += 1;
}

void dispensarSegundos(unsigned long segundos) {
  // En tu código Python haces algo tipo: dispensar(5) etc.
  digitalWrite(PIN_BOMBA, LOW);   // Asumiendo activo en LOW
  unsigned long inicio = millis();
  while (millis() - inicio < segundos * 1000UL) {
    // Podrías monitorear aquí el flujo si quieres
  }
  digitalWrite(PIN_BOMBA, HIGH);
}

void registrarVenta(const char* tipo) {
  // Aquí podrías contar ventas, guardar en EEPROM, etc.
  Serial.print("Venta registrada: ");
  Serial.println(tipo);
}

// Imprime un mensaje en una línea del LCD y rellena con espacios
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
  Serial.begin(9600);

  // Botones con pull-up
  pinMode(PIN_BTN_GALON,    INPUT_PULLUP);
  pinMode(PIN_BTN_GARRAFON, INPUT_PULLUP);
  pinMode(PIN_BTN_LIMPIAR,  INPUT_PULLUP);
  pinMode(PIN_BTN_CANCELAR, INPUT_PULLUP);

  // Bomba
  pinMode(PIN_BOMBA, OUTPUT);
  digitalWrite(PIN_BOMBA, HIGH);  // apagada

  // Hopper (más adelante puedes usarlo)
  pinMode(PIN_HOPPER, OUTPUT);
  digitalWrite(PIN_HOPPER, HIGH); // apagado

  // Monedero (interrupción)
  pinMode(PIN_MONEDAS, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_MONEDAS), monedaInsertada, FALLING);

  // LCD
  lcd.begin(16, 2);        // Inicializa el LCD de 16x2
  lcd.setBacklight(255);   // Enciende el backlight (255 = máximo brillo)

  lcdString("Hola Edgar!", 0);        // línea 1
  lcdString("Tu LCD funciona", 1);    // línea 2
  delay(1500);
}

void loop() {
  // Copia local de credito (evita leer volátil muchas veces)
  noInterrupts();
  int creditoLocal = credito;
  interrupts();

  // Mostrar estado en LCD
  char buffer[17];  // 16 + terminador
  snprintf(buffer, sizeof(buffer), "Credito: $%d", creditoLocal);

  lcdString("Listo...", 0);
  lcdString(buffer,   1);
  delay(200);

  // --- Botón Galón ---
  if (digitalRead(PIN_BTN_GALON) == LOW && creditoLocal >= PRECIO_GALON) {
    lcdString("Dispensando", 0);
    lcdString("Galon...",    1);
    dispensarSegundos(5);  // ajusta según tu bomba
    registrarVenta("Galon");

    noInterrupts();
    credito -= PRECIO_GALON;
    interrupts();
  }

  // --- Botón Garrafón ---
  else if (digitalRead(PIN_BTN_GARRAFON) == LOW && creditoLocal >= PRECIO_GARRAFON) {
    lcdString("Dispensando", 0);
    lcdString("Garrafon...", 1);
    dispensarSegundos(10); // ejemplo
    registrarVenta("Garrafon");

    noInterrupts();
    credito -= PRECIO_GARRAFON;
    interrupts();
  }

  // --- Botón Limpiar ---
  else if (digitalRead(PIN_BTN_LIMPIAR) == LOW) {
    lcdString("Limpieza",   0);
    lcdString("Garrafon...",1);
    dispensarSegundos(4);
  }

  // --- Botón Cancelar ---
  else if (digitalRead(PIN_BTN_CANCELAR) == LOW && creditoLocal > 0) {
    Serial.print("Cancelado. Devolviendo ");
    Serial.print(creditoLocal);
    Serial.println(" unidades.");
    noInterrupts();
    credito = 0;
    interrupts();
    lcdString("Cancelado",   0);
    lcdString("Credito: $0", 1);
    delay(800);
  }
}


