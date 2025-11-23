// relays_banks_arduino.ino
// Control manual de dos relés: bomba y hopper

const int PIN_BOMBA  = 8;   // Cambia según tu conexión
const int PIN_HOPPER = 9;

void setup() {
  Serial.begin(9600);

  pinMode(PIN_BOMBA, OUTPUT);
  pinMode(PIN_HOPPER, OUTPUT);

  // Asumiendo relés activos en LOW (como en tu Pi)
  digitalWrite(PIN_BOMBA, HIGH);
  digitalWrite(PIN_HOPPER, HIGH);

  Serial.println("Control manual");
  Serial.println("  b -> bomba (3s)");
  Serial.println("  c -> hopper (3s)");
  Serial.println("  q -> (no hace nada especial, solo ejemplo)");
}

void loop() {
  if (Serial.available() > 0) {
    char opcion = Serial.read();

    if (opcion == 'b') {
      Serial.println("Activando bomba...");
      digitalWrite(PIN_BOMBA, LOW);
      delay(3000);
      digitalWrite(PIN_BOMBA, HIGH);
      Serial.println("Bomba apagada.");
    } 
    else if (opcion == 'c') {
      Serial.println("Activando hopper...");
      digitalWrite(PIN_HOPPER, LOW);
      delay(3000);
      digitalWrite(PIN_HOPPER, HIGH);
      Serial.println("Hopper apagado.");
    }
    else if (opcion == 'q') {
      Serial.println("Comando q recibido (aquí podrías parar algo).");
    }
    else {
      Serial.println("Comando no reconocido.");
    }
  }
}

