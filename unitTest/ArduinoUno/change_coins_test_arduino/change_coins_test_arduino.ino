// change_coins_test_arduino.ino
// Cuenta pulsos de un sensor IR de monedas usando interrupción

const int PIN_IR_HOPPER = 2;  // Debe ser 2 o 3 en Arduino Uno
volatile unsigned long monedas_contadas = 0;

void contarPulso() {
  monedas_contadas++;
}

void setup() {
  Serial.begin(9600);

  pinMode(PIN_IR_HOPPER, INPUT_PULLUP);  // Asumiendo salida tipo colector abierto
  attachInterrupt(digitalPinToInterrupt(PIN_IR_HOPPER), contarPulso, FALLING);

  Serial.println("Esperando monedas del hopper...");
}

void loop() {
  // Copiamos la variable volátil con interrupciones bloqueadas brevemente
  noInterrupts();
  unsigned long copia = monedas_contadas;
  interrupts();

  Serial.print("Monedas detectadas: ");
  Serial.println(copia);

  delay(500);
}

