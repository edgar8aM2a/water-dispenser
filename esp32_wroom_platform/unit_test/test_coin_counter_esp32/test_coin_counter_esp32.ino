const int PIN_MONEDA = 14;

unsigned long contador = 0;
int estadoAnterior = LOW;

void setup() {
  Serial.begin(115200);

  pinMode(PIN_MONEDA, INPUT_PULLUP); 
  // Usa INPUT si tu módulo ya tiene pull-up/pull-down

  Serial.println("Listo. Inserta monedas...");
}

void loop() {
  int estadoActual = digitalRead(PIN_MONEDA);

  // Detecta flanco de bajada (pulso)
  if (estadoAnterior == HIGH && estadoActual == LOW) {
    contador++;
    Serial.print("Pulso detectado. Total: ");
    Serial.println(contador);
  }

  estadoAnterior = estadoActual;
}

