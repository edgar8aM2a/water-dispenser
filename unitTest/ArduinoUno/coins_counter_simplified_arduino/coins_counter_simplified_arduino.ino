// coins_counter_simplified_arduino.ino
// Esqueleto basado en tu coins_counter_test.py

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- Pines (ajusta según tu hardware) ---
const int PIN_BTN_GALON     = 4;
const int PIN_BTN_GARRAFON  = 5;
const int PIN_BTN_LIMPIAR   = 6;
const int PIN_BTN_CANCELAR  = 7;
const int PIN_MONEDAS       = 2;  // interrupción
const int PIN_BOMBA         = 8;

// --- Precios ---
const int PRECIO_GALON    = 10;
const int PRECIO_GARRAFON = 20;

// --- Estado ---
volatile int credito = 0;

// ISR: cada pulso = $10 (ajusta según tu monedero)
void monedaInsertada() {
  credito += 10;
}

void mostrarEnLcd(const char* linea1, const char* linea2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(linea1);
  lcd.setCursor(0, 1);
  lcd.print(linea2);
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

  // Monedero
  pinMode(PIN_MONEDAS, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_MONEDAS), monedaInsertada, FALLING);

  // LCD
  lcd.init();
  lcd.backlight();
  mostrarEnLcd("Sistema listo", "Credito: $0");
}

void loop() {
  // Copia local de credito (evita leer volátil muchas veces)
  noInterrupts();
  int creditoLocal = credito;
  interrupts();

  // Muestra crédito en LCD de vez en cuando
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "Credito: $%d", creditoLocal);

  mostrarEnLcd("Listo...", buffer);
  delay(200);

  // --- Botón Galón ---
  if (digitalRead(PIN_BTN_GALON) == LOW && creditoLocal >= PRECIO_GALON) {
    mostrarEnLcd("Dispensando", "Galon...");
    dispensarSegundos(5);  // ajusta según tu bomba
    registrarVenta("Galon");

    noInterrupts();
    credito -= PRECIO_GALON;
    interrupts();
  }

  // --- Botón Garrafón ---
  else if (digitalRead(PIN_BTN_GARRAFON) == LOW && creditoLocal >= PRECIO_GARRAFON) {
    mostrarEnLcd("Dispensando", "Garrafon...");
    dispensarSegundos(100); // ejemplo
    registrarVenta("Garrafon");

    noInterrupts();
    credito -= PRECIO_GARRAFON;
    interrupts();
  }

  // --- Botón Limpiar ---
  else if (digitalRead(PIN_BTN_LIMPIAR) == LOW) {
    mostrarEnLcd("Limpieza", "Garrafon...");
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
    mostrarEnLcd("Cancelado", "Credito: $0");
    delay(800);
  }
}

