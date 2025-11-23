// buttons_test_arduino.ino
// Prueba 3 botones con pull-up interno

const int BUTTON_PINS[] = {4, 5, 6};  // Cambia según tu cableado
const int NUM_BUTTONS = 3;

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);  // Igual que PUD_UP en RPi
  }

  Serial.println("Probando botones... Ctrl+Shift+M para abrir Serial Monitor.");
}

void loop() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    int state = digitalRead(BUTTON_PINS[i]);
    // LOW = botón presionado (igual que en tu script)
    if (state == LOW) {
      Serial.print("Boton en pin ");
      Serial.print(BUTTON_PINS[i]);
      Serial.println(" presionado!");
      delay(200);  // debounce sencillo
    }
  }
  delay(10);
}
