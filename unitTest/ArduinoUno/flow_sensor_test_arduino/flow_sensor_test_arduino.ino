// flow_sensor_test_arduino.ino
// Mide caudal en L/min usando pulsos por segundo

const int PIN_FLUJO = 3;     // Debe ser 2 o 3 para interrupción en UNO
volatile unsigned long contador_pulsos = 0;

void contarPulsoFlujo() {
  contador_pulsos++;
}

void setup() {
  Serial.begin(9600);
  pinMode(PIN_FLUJO, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_FLUJO), contarPulsoFlujo, FALLING);

  Serial.println("Midiendo flujo...");
}

void loop() {
  // Resetear contador y medir 1 segundo
  contador_pulsos = 0;
  delay(1000);

  // Copiamos valor
  noInterrupts();
  unsigned long pulsos = contador_pulsos;
  interrupts();

  float flujo_L_min = pulsos / 7.5; // mismo factor que en tu script

  Serial.print(flujo_L_min, 2);
  Serial.print(" L/min (");
  Serial.print(pulsos);
  Serial.println(" pulsos)");
}
