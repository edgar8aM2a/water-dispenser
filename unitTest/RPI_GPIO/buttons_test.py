import RPi.GPIO as GPIO
import time

# ----------------------------
# Configuración de pines
# ----------------------------
BUTTON_PINS = [17, 27, 22]  # Cambia estos números por los GPIO que uses

GPIO.setmode(GPIO.BCM)

# Configurar cada botón como entrada con pull-up interno
for pin in BUTTON_PINS:
    GPIO.setup(pin, GPIO.IN, pull_up_down=GPIO.PUD_UP)

print("Probando botones... Presiona Ctrl+C para salir.\n")

try:
    while True:
        for pin in BUTTON_PINS:
            if GPIO.input(pin) == GPIO.LOW:   # LOW = botón presionado
                print(f"Botón en GPIO {pin} presionado!")
                time.sleep(0.2)  # debounce simple
        time.sleep(0.01)

except KeyboardInterrupt:
    print("\nSaliendo...")

finally:
    GPIO.cleanup()
    print("GPIO liberados.")


