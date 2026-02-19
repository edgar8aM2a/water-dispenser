const int PIN_MONEDA = 18;
static const int PIN_HOPPER = 5;

unsigned long contador = 0;
int estadoAnterior = HIGH;   // INPUT_PULLUP → inicia en HIGH

bool hopperOn = false;
unsigned long ultimoPulsoMs = 0;
const unsigned long FILTRO_MS = 10;  // ajusta: 5–20 ms

void setup() {
  Serial.begin(115200);

  pinMode(PIN_MONEDA, INPUT);
  pinMode(PIN_HOPPER, OUTPUT);

  // Estado seguro inicial
  digitalWrite(PIN_HOPPER, HIGH); // lógica negativa → apagado

  Serial.println("Listo.");
  Serial.println("Presiona 'q' para encender/apagar el hopper");
}

void loop() {

  // ----------- Control por teclado -----------
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'q' || c == 'Q') {
      hopperOn = !hopperOn;
      digitalWrite(PIN_HOPPER, hopperOn ? LOW : HIGH);
      Serial.println(hopperOn ? "HOPPER ON" : "HOPPER OFF");
    }
  }

  // ----------- Lectura de pulsos -------------
  int estadoActual = digitalRead(PIN_MONEDA);

  // Detecta flanco de bajada
  if (estadoAnterior == HIGH && estadoActual == LOW) {
    unsigned long ahora = millis();

    // Filtro por tiempo (anti-ruido)
    if (ahora - ultimoPulsoMs >= FILTRO_MS) {
      ultimoPulsoMs = ahora;
      contador++;
      Serial.print("Pulso valido. Total: ");
      Serial.println(contador);
    } else {
      Serial.println("Pulso ignorado (ruido)");
    }
  }

  estadoAnterior = estadoActual;
}


