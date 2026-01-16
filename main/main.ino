// coins_counter_final_arduino.ino
// Sistema completo: monedero + flujo + hopper + LCD + botones

#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

// --------------------- LCD ---------------------
LiquidCrystal_PCF8574 lcd(0x27);     // Cambia 0x27 si tu módulo usa otra dirección
const uint8_t LCD_WIDTH = 16;        // Ancho del display

// --------------------- Pines ---------------------
// Interrupciones externas
const int PIN_MONEDAS   = 2;   // INT0 - monedero
const int PIN_FLUJO     = 3;   // INT1 - sensor de flujo

// Botones (asumimos INPUT_PULLUP: LOW = presionado)
const int PIN_BTN_GALON     = 4;
const int PIN_BTN_GARRAFON  = 5;
const int PIN_BTN_LIMPIAR   = 6;
const int PIN_BTN_CANCELAR  = 7;

// Relés
const int PIN_BOMBA   = 8;    // Relé bomba
const int PIN_HOPPER  = 9;    // Relé hopper

// Sensor IR del hopper (solo polling)
const int PIN_IR_HOPPER = 11;

// --------------------- Precios / parámetros ---------------------
// Crédito en "unidades" (p. ej. 1 unidad = 1 peso, o 1 pulso = 1 crédito)
const int PRECIO_GALON    = 10;
const int PRECIO_GARRAFON = 20;

// Volumen por producto (en litros)
const float LITROS_GALON    = 3.8;   // ajusta según tu máquina
const float LITROS_GARRAFON = 19.0;  // ajusta según tu máquina

// Flujo: pulsos por litro (depende del sensor y de la instalación)
const float PULSOS_POR_LITRO = 450.0; // valor típico, calibra

// Hopper: cuántas unidades de crédito equivale cada pulso del IR
const int CREDITO_POR_PULSO_HOPPER = 1;

// Tiempo de limpieza (segundos)
const unsigned long TIEMPO_LIMPIEZA_SEG = 10;

// --------------------- Estado global ---------------------
volatile int credito = 0;                  // crédito acumulado (monedero)
volatile unsigned long pulsosFlujo = 0;    // pulsos del sensor de flujo

// --------------------- ISR ---------------------
// Monedero: cada pulso = 1 unidad de crédito (ajusta si quieres)
void isrMoneda() {
  credito += 1;
}

// Sensor de flujo: cuenta pulsos
void isrFlujo() {
  pulsosFlujo++;
}

// --------------------- Helpers LCD ---------------------
void lcdString(const char* message, uint8_t line) {
  // line = 0 o 1
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

// Muestra la pantalla de inicio / espera
void pantallaInicio() {
  lcdString("Inserte monedas", 0);

  char linea2[17];
  snprintf(linea2, sizeof(linea2), "Credito: %d", credito);
  lcdString(linea2, 1);
}

// --------------------- Bomba control por litros ---------------------
void dispensarPorLitros(float litrosObjetivo) {
  // Reiniciamos contador de flujo
  noInterrupts();
  pulsosFlujo = 0;
  interrupts();

  digitalWrite(PIN_BOMBA, LOW);  // Enciende bomba (activo en LOW)

  unsigned long inicio = millis();
  const unsigned long TIMEOUT_MS = 60000; // 60s de seguridad, ajusta

  while (true) {
    noInterrupts();
    unsigned long pulsos = pulsosFlujo;
    interrupts();

    float litros = pulsos / PULSOS_POR_LITRO;

    // Mostrar progreso en LCD (segunda línea)
    char linea2[17];
    snprintf(linea2, sizeof(linea2), "L: %.2f/%.2f", litros, litrosObjetivo);
    lcdString(linea2, 1);

    if (litros >= litrosObjetivo) {
      break;
    }
    if (millis() - inicio > TIMEOUT_MS) {
      Serial.println("WARNING: Timeout en dispensarPorLitros");
      break;
    }

    // Opcional: permitir cancelar durante el llenado
    if (digitalRead(PIN_BTN_CANCELAR) == LOW) {
      Serial.println("Dispensado cancelado por usuario");
      break;
    }
  }

  digitalWrite(PIN_BOMBA, HIGH); // Apaga bomba
}

// --------------------- Bomba por tiempo (para limpiar) ---------------------
void dispensarSegundos(unsigned long segundos) {
  lcdString("Limpieza...", 0);

  digitalWrite(PIN_BOMBA, LOW);
  unsigned long inicio = millis();

  while (millis() - inicio < segundos * 1000UL) {
    // Podrías leer flujo o monitorear botón de cancelar si quieres
    if (digitalRead(PIN_BTN_CANCELAR) == LOW) {
      Serial.println("Limpieza cancelada por usuario");
      break;
    }
  }

  digitalWrite(PIN_BOMBA, HIGH);
}

// --------------------- Registro de venta ---------------------
void registrarVenta(const char* tipo) {
  Serial.print("Venta registrada: ");
  Serial.println(tipo);
}

// --------------------- Devolver cambio con hopper ---------------------
// Devuelve CUÁNTO crédito se devolvió efectivamente
int devolverCambio(int creditoADevolver) {
  if (creditoADevolver <= 0) return 0;

  lcdString("Devolviendo...", 0);

  int devuelto = 0;
  int estadoAnterior = digitalRead(PIN_IR_HOPPER);

  // Enciende hopper (activo en LOW)
  digitalWrite(PIN_HOPPER, LOW);

  unsigned long inicio = millis();
  const unsigned long TIMEOUT_MS = 20000; // 20s de seguridad

  while (devuelto < creditoADevolver) {
    int estadoActual = digitalRead(PIN_IR_HOPPER);

    // Contamos flanco de bajada como 1 unidad devuelta
    if (estadoAnterior == HIGH && estadoActual == LOW) {
      devuelto += CREDITO_POR_PULSO_HOPPER;

      // Actualizar LCD con lo devuelto
      char linea2[17];
      snprintf(linea2, sizeof(linea2), "Dev: %d/%d",
               devuelto, creditoADevolver);
      lcdString(linea2, 1);
    }

    estadoAnterior = estadoActual;

    // Timeout de seguridad
    if (millis() - inicio > TIMEOUT_MS) {
      Serial.println("WARNING: Timeout en devolverCambio");
      break;
    }

    // Permitir cancelar devolución (si presionan de nuevo cancelar)
    if (digitalRead(PIN_BTN_CANCELAR) == LOW) {
      Serial.println("Usuario cancela devolucion");
      break;
    }
  }

  // Apagar hopper
  digitalWrite(PIN_HOPPER, HIGH);

  return devuelto;
}

// --------------------- SETUP ---------------------
void setup() {
  Serial.begin(9600);

  // LCD
  lcd.begin(16, 2);
  lcd.setBacklight(255);
  lcd.clear();

  // Pines
  pinMode(PIN_MONEDAS, INPUT);
  pinMode(PIN_FLUJO, INPUT);

  pinMode(PIN_BTN_GALON,     INPUT_PULLUP);
  pinMode(PIN_BTN_GARRAFON,  INPUT_PULLUP);
  pinMode(PIN_BTN_LIMPIAR,   INPUT_PULLUP);
  pinMode(PIN_BTN_CANCELAR,  INPUT_PULLUP);

  pinMode(PIN_BOMBA,  OUTPUT);
  pinMode(PIN_HOPPER, OUTPUT);
  pinMode(PIN_IR_HOPPER, INPUT);

  // Relés apagados inicialmente (asumimos HIGH = OFF)
  digitalWrite(PIN_BOMBA, HIGH);
  digitalWrite(PIN_HOPPER, HIGH);

  // Interrupciones
  attachInterrupt(digitalPinToInterrupt(PIN_MONEDAS), isrMoneda, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_FLUJO),   isrFlujo,   RISING);

  pantallaInicio();
}

// --------------------- LOOP PRINCIPAL ---------------------
void loop() {
  // --------- Actualizar pantalla de crédito si hay cambio ---------
  static int ultimoCreditoMostrado = -1;
  noInterrupts();
  int creditoLocal = credito;
  interrupts();

  if (creditoLocal != ultimoCreditoMostrado) {
    pantallaInicio();
    ultimoCreditoMostrado = creditoLocal;
  }

  // --------- Lectura de botones (activos en LOW) ---------
  bool galonPresionado     = (digitalRead(PIN_BTN_GALON)    == LOW);
  bool garrafonPresionado  = (digitalRead(PIN_BTN_GARRAFON) == LOW);
  bool limpiarPresionado   = (digitalRead(PIN_BTN_LIMPIAR)  == LOW);
  bool cancelarPresionado  = (digitalRead(PIN_BTN_CANCELAR) == LOW);

  // Pequeño debounce sencillo
  if (galonPresionado || garrafonPresionado ||
      limpiarPresionado || cancelarPresionado) {
    delay(150); // simplón, pero funcional para pruebas
  }

  // --------- Lógica de selección de producto ---------

  // 1) Galón
  if (galonPresionado) {
    if (creditoLocal >= PRECIO_GALON) {
      lcdString("Dispensando galon", 0);

      dispensarPorLitros(LITROS_GALON);
      registrarVenta("GALON");

      // Descontar crédito
      noInterrupts();
      credito -= PRECIO_GALON;
      if (credito < 0) credito = 0;
      creditoLocal = credito;
      interrupts();

      pantallaInicio();
      ultimoCreditoMostrado = creditoLocal;
    } else {
      lcdString("Credito insuf.", 0);
      delay(1500);
      pantallaInicio();
    }
  }

  // 2) Garrafon
  if (garrafonPresionado) {
    if (creditoLocal >= PRECIO_GARRAFON) {
      lcdString("Dispensando garr", 0);

      dispensarPorLitros(LITROS_GARRAFON);
      registrarVenta("GARRAFON");

      noInterrupts();
      credito -= PRECIO_GARRAFON;
      if (credito < 0) credito = 0;
      creditoLocal = credito;
      interrupts();

      pantallaInicio();
      ultimoCreditoMostrado = creditoLocal;
    } else {
      lcdString("Credito insuf.", 0);
      delay(1500);
      pantallaInicio();
    }
  }

  // 3) Limpiar (no requiere crédito)
  if (limpiarPresionado) {
    dispensarSegundos(TIEMPO_LIMPIEZA_SEG);
    pantallaInicio();
  }

  // 4) Cancelar: devolver cambio
  if (cancelarPresionado) {
    // Hacer una copia del crédito actual
    noInterrupts();
    int creditoParaDevolver = credito;
    interrupts();

    if (creditoParaDevolver > 0) {
      int devuelto = devolverCambio(creditoParaDevolver);

      noInterrupts();
      credito -= devuelto;
      if (credito < 0) credito = 0;
      creditoLocal = credito;
      interrupts();

      lcdString("Listo", 0);
      char linea2[17];
      snprintf(linea2, sizeof(linea2), "Devuelto: %d", devuelto);
      lcdString(linea2, 1);
      delay(2000);
    } else {
      lcdString("Sin credito", 0);
      lcdString("a devolver", 1);
      delay(1500);
    }

    pantallaInicio();
    ultimoCreditoMostrado = creditoLocal;
  }

  // Pequeño delay para evitar loop demasiado rápido
  delay(20);
}
