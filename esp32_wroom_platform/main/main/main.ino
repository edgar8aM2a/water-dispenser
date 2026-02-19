/*
  ESP32 - Máquina vending de agua (robusta) + LCD PCF8574

  Sensores:
   - Contador de moneda (pulso)     PIN 14
   - Contador de cambio (pulso)     PIN 18
   - Sensor de flujo (pulso)        PIN 19

  Actuadores:
   - LCD 16x2 I2C  SDA=21  SCL=22   (PCF8574)
   - Relay Bomba                   PIN 25
   - Hopper de cambio              PIN 5   (D5)

  Botones:
   - 20L (20 pesos)                PIN 32
   - Galón (10 pesos)              PIN 33
   - Litro (5 pesos)               PIN 26
   - Cancelar                      PIN 27
   - Parar bomba / Pausa           PIN 13

  IMPORTANTE (calibración):
   - PESOS_POR_PULSO_COIN / CHANGE: ajusta según tu hardware real
   - PULSOS_POR_LITRO: calibra con medición real
   - Si tu relay/hopper son activos en LOW, invierte en setPump/setHopper.
*/

#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

// ---------------- Pines ----------------
static const int PIN_COIN_PULSE   = 14;
static const int PIN_CHANGE_PULSE = 18;
static const int PIN_FLOW_PULSE   = 19;

static const int PIN_PUMP_RELAY   = 25;
static const int PIN_HOPPER       = 5;   // D5

static const int BTN_20L          = 32;
static const int BTN_GALON        = 33;
static const int BTN_LITRO        = 26;
static const int BTN_CANCEL       = 27;
static const int BTN_STOP_PUMP    = 13;

// ---------------- LCD ----------------
// Cambia 0x27 <-> 0x3F según tu backpack PCF8574
LiquidCrystal_PCF8574 lcd(0x27);

// ---------------- Calibraciones / reglas ----------------
static const int PESOS_POR_PULSO_COIN   = 1;  // AJUSTA
static const int PESOS_POR_PULSO_CHANGE = 1;  // AJUSTA
static const float PULSOS_POR_LITRO     = 450.0f; // EJEMPLO, AJUSTA

// ---------------- Productos ----------------
struct Product {
  const char* name;
  int price_pesos;
  float liters;
};

static const Product PROD_20L  = {"20L",  20, 20.0f};
static const Product PROD_GAL  = {"GAL",  10, 3.785f};
static const Product PROD_1L   = {"1L",    5, 1.0f};

// ---------------- Estado ----------------
enum State {
  ST_IDLE_SELECT = 0,
  ST_WAIT_MONEY,
  ST_DISPENSING,
  ST_PAUSED,
  ST_CHANGE_PAYOUT
};

volatile uint32_t isr_coin_pulses   = 0;
volatile uint32_t isr_change_pulses = 0;
volatile uint32_t isr_flow_pulses   = 0;

// Contadores “procesados”
uint32_t coin_pulses_used   = 0;
uint32_t change_pulses_used = 0;
uint32_t flow_pulses_used   = 0;

int credit_pesos = 0;
int change_due_pesos = 0;

const Product* selected = nullptr;
State st = ST_IDLE_SELECT;

// Dispensado
float target_liters = 0.0f;
uint32_t target_flow_pulses = 0;

// ---------------- ISR ----------------
void IRAM_ATTR onCoinPulse()   { isr_coin_pulses++; }
void IRAM_ATTR onChangePulse() { isr_change_pulses++; }
void IRAM_ATTR onFlowPulse()   { isr_flow_pulses++; }

// ---------------- Botones (debounce simple) ----------------
struct DebouncedButton {
  int pin;
  bool stable = true;      // pullup => HIGH = no presionado
  bool last_read = true;
  uint32_t last_ms = 0;
  uint32_t debounce_ms = 30;

  void begin(int p) {
    pin = p;
    pinMode(pin, INPUT_PULLUP);
    stable = digitalRead(pin);
    last_read = stable;
    last_ms = millis();
  }

  // retorna true SOLO cuando detecta “click” (transición a LOW estable)
  bool fell() {
    bool r = digitalRead(pin);
    if (r != last_read) {
      last_read = r;
      last_ms = millis();
    }
    if ((millis() - last_ms) > debounce_ms && stable != last_read) {
      stable = last_read;
      if (stable == LOW) return true;
    }
    return false;
  }

  bool isPressed() const { return stable == LOW; }
};

DebouncedButton b20, bgal, b1l, bcancel, bstop;

// ---------------- Actuadores ----------------
void setPump(bool on) {
  // Si tu relay es activo LOW, invierte:
  // digitalWrite(PIN_PUMP_RELAY, on ? LOW : HIGH);
  digitalWrite(PIN_PUMP_RELAY, on ? LOW : HIGH);
}

void setHopper(bool on) {
  // Si tu driver es activo LOW, invierte:
  // digitalWrite(PIN_HOPPER, on ? LOW : HIGH);
  digitalWrite(PIN_HOPPER, on ? LOW : HIGH);
}

// ---------------- UI LCD ----------------
void lcdShow2(const String& l1, const String& l2) {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(l1);
  lcd.setCursor(0, 1); lcd.print(l2);
}

void lcdStatus() {
  // Nota: aquí usamos clear() para simplificar; si ves parpadeo, lo optimizamos luego.
  lcd.clear();

  if (st == ST_IDLE_SELECT) {
    lcd.setCursor(0, 0);
    lcd.print("Bienvenido");
    lcd.setCursor(0, 1);
    lcd.print("Credito:$");
    lcd.print(credit_pesos);
    return;
  }

  if (st == ST_WAIT_MONEY) {
    lcd.setCursor(0, 0);
    lcd.print("Sel:");
    lcd.print(selected ? selected->name : "???");
    lcd.print(" $");
    lcd.print(selected ? selected->price_pesos : 0);

    lcd.setCursor(0, 1);
    lcd.print("Cred:$");
    lcd.print(credit_pesos);
    lcd.print(" Can:27");
    return;
  }

  if (st == ST_DISPENSING || st == ST_PAUSED) {
    float disp_liters = (float)(flow_pulses_used) / PULSOS_POR_LITRO;

    lcd.setCursor(0, 0);
    lcd.print(st == ST_PAUSED ? "PAUSA " : "Sirviendo ");
    lcd.print(selected ? selected->name : "???");

    lcd.setCursor(0, 1);
    lcd.print(disp_liters, 2);
    lcd.print("/");
    lcd.print(target_liters, 2);
    lcd.print("L Stop:13");
    return;
  }

  if (st == ST_CHANGE_PAYOUT) {
    lcd.setCursor(0, 0);
    lcd.print("Entregando cambio");
    lcd.setCursor(0, 1);
    lcd.print("Falta:$");
    lcd.print(change_due_pesos);
    return;
  }
}

// ---------------- Lógica de dinero ----------------
void updateCreditFromCoinPulses() {
  uint32_t p;
  noInterrupts();
  p = isr_coin_pulses;
  interrupts();

  uint32_t new_pulses = p - coin_pulses_used;
  if (new_pulses == 0) return;

  coin_pulses_used += new_pulses;
  credit_pesos += (int)new_pulses * PESOS_POR_PULSO_COIN;
}

int changePaidFromPulses() {
  uint32_t p;
  noInterrupts();
  p = isr_change_pulses;
  interrupts();

  uint32_t new_pulses = p - change_pulses_used;
  if (new_pulses == 0) return 0;

  change_pulses_used += new_pulses;
  return (int)new_pulses * PESOS_POR_PULSO_CHANGE;
}

// ---------------- Flujo ----------------
void resetFlowCount() {
  noInterrupts();
  isr_flow_pulses = 0;
  interrupts();
  flow_pulses_used = 0;
}

void updateFlowUsed() {
  uint32_t p;
  noInterrupts();
  p = isr_flow_pulses;
  interrupts();

  flow_pulses_used = p;
}

// ---------------- Estados ----------------
void startTransaction(const Product* p) {
  selected = p;
  target_liters = p->liters;
  target_flow_pulses = (uint32_t)(target_liters * PULSOS_POR_LITRO);

  st = ST_WAIT_MONEY;
  lcdStatus();
}

void beginDispense() {
  resetFlowCount();
  setPump(true);
  st = ST_DISPENSING;
  lcdStatus();
}

void finishDispense() {
  setPump(false);

  int price = selected ? selected->price_pesos : 0;
  credit_pesos -= price;
  if (credit_pesos < 0) credit_pesos = 0;

  // Si sobró dinero, dar cambio
  if (credit_pesos > 0) {
    change_due_pesos = credit_pesos;
    credit_pesos = 0;
    st = ST_CHANGE_PAYOUT;
  } else {
    st = ST_IDLE_SELECT;
    selected = nullptr;
  }

  lcdStatus();
}

void manualStopDispense() {
  // Solo aplica si ya se está sirviendo o está en pausa
  if (st != ST_DISPENSING && st != ST_PAUSED) return;

  // Corta bomba y finaliza como si “ya terminó” (cobra producto completo)
  setPump(false);

  // Reusa la misma lógica de fin de despacho: descuenta precio y da cambio si sobra
  finishDispense();
}


void cancelTransaction() {
  setPump(false);

  // Devuelve todo el crédito (si así quieres que funcione cancelar)
  if (credit_pesos > 0) {
    change_due_pesos = credit_pesos;
    credit_pesos = 0;
    st = ST_CHANGE_PAYOUT;
  } else {
    st = ST_IDLE_SELECT;
    selected = nullptr;
  }

  lcdStatus();
}

void pauseOrResumePump() {
  if (st == ST_DISPENSING) {
    setPump(false);
    st = ST_PAUSED;
  } else if (st == ST_PAUSED) {
    setPump(true);
    st = ST_DISPENSING;
  }
  lcdStatus();
}

// ---------------- Setup/Loop ----------------
void setup() {
  Serial.begin(115200);

  // Actuadores
  pinMode(PIN_PUMP_RELAY, OUTPUT);
  pinMode(PIN_HOPPER, OUTPUT);
  setPump(false);
  setHopper(false);

  // Botones
  b20.begin(BTN_20L);
  bgal.begin(BTN_GALON);
  b1l.begin(BTN_LITRO);
  bcancel.begin(BTN_CANCEL);
  bstop.begin(BTN_STOP_PUMP);

  // Sensores
  pinMode(PIN_COIN_PULSE, INPUT_PULLUP);
  pinMode(PIN_CHANGE_PULSE, INPUT_PULLUP);
  pinMode(PIN_FLOW_PULSE, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(PIN_COIN_PULSE), onCoinPulse, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_CHANGE_PULSE), onChangePulse, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_FLOW_PULSE), onFlowPulse, FALLING);

  // LCD I2C
  Wire.begin(21, 22);

  // Inicialización LCD PCF8574
  lcd.begin(16, 2);
  lcd.setBacklight(1);   // 0..255 (si no enciende, prueba 1 en vez de 255)

  lcdShow2("MEGA Water Vending", "Listo");
  delay(800);

  lcdStatus();
}

void loop() {
  // 1) Actualizar crédito según monedas
  updateCreditFromCoinPulses();

  // 2) Leer botones
  if (bcancel.fell()) {
    if (st == ST_WAIT_MONEY) {
      cancelTransaction();          // rembolso por que aun no dispensa
    } else if (st == ST_DISPENSING || st == ST_PAUSED){
      /// Durante despacho: termina despacho ya (para evitar derrame)
      manualStopDispense();
    }
  }

  if (bstop.fell()) {
    if (st == ST_DISPENSING || st == ST_PAUSED) {
      manualStopDispense();
    }
  }

  // Selección de producto
  if (st == ST_IDLE_SELECT || st == ST_WAIT_MONEY) {
    if (b20.fell())  startTransaction(&PROD_20L);
    if (bgal.fell()) startTransaction(&PROD_GAL);
    if (b1l.fell())  startTransaction(&PROD_1L);
  }

  // 3) Lógica por estado
  switch (st) {
    case ST_IDLE_SELECT: {
      // Permite que el usuario meta monedas antes de elegir producto.
      enableInterrupt(digitalPinToInterrupt(PIN_COIN_PULSE));
      break;
    }

    case ST_WAIT_MONEY: {
      enableInterrupt(digitalPinToInterrupt(PIN_COIN_PULSE));
      if (selected && credit_pesos >= selected->price_pesos) {
        beginDispense();
      }
      break;
    }

    case ST_DISPENSING: {
      disableInterrupt(digitalPinToInterrupt(PIN_COIN_PULSE));//Disable leer pulsos de contador de monedas
      disableInterrupt(digitalPinToInterrupt(PIN_CHANGE_PULSE));
      updateFlowUsed();
      if (flow_pulses_used >= target_flow_pulses) {
        finishDispense();
      }
      break;
    }

    case ST_PAUSED: {
      updateFlowUsed(); // opcional
      break;
    }

    case ST_CHANGE_PAYOUT: {
      
      disableInterrupt(digitalPinToInterrupt(PIN_COIN_PULSE));//Disable leer pulsos de contador de monedas
      enableInterrupt(digitalPinToInterrupt(PIN_CHANGE_PULSE));
      setHopper(true);
      Serial.print("im here");  
      
      int paid = changePaidFromPulses();
      if (paid > 0) {
        change_due_pesos -= paid;
        if (change_due_pesos < 0) change_due_pesos = 0;
      }

      if (change_due_pesos == 0) {
        setHopper(false);
        selected = nullptr;
        st = ST_IDLE_SELECT;
        lcdStatus();
      }
      break;
    }
  }

  // 4) Refrescar LCD cada cierto tiempo
  static uint32_t last_ui = 0;
  if (millis() - last_ui > 600) { 
    last_ui = millis();
    lcdStatus();
  }

  delay(5);
}
