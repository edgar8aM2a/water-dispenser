import RPi.GPIO as GPIO #Solo disponible en rasberry
import time
from datetime import datetime

#Configuración de pines GPIO
PIN_BTN_galon = 17
PIN_BTN_GARRAFON = 27
PIN_BTN_LIMPIAR = 22
PIN_BTN_CANCELAR = 23
PIN_MONEDAS = 24
PIN_BOMBA = 18

#Precios
PRECIO_galon = 10
PRECIO_GARRAFON = 20

#Variables de estado
credito = 0

#Setup GPIO
GPIO.setmode(GPIO.BCM)
GPIO.setup([PIN_BTN_galon, PIN_BTN_GARRAFON, PIN_BTN_LIMPIAR, PIN_BTN_CANCELAR, PIN_MONEDAS], GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(PIN_BOMBA, GPIO.OUT)
GPIO.output(PIN_BOMBA, GPIO.LOW)

#Función para dispensar agua
def dispensar(segundos):
    print(f"→ DISPENSANDO agua por {segundos} segundos")
    GPIO.output(PIN_BOMBA, GPIO.HIGH)
    time.sleep(segundos)
    GPIO.output(PIN_BOMBA, GPIO.LOW)

#Callback para monedas
def moneda_insertada(channel):
    global credito
    credito += 10  # suponiendo que cada pulso vale 10 unidades
    print(f"Credito: {credito}")

#Log de ventas
def registrar_venta(tipo):
    ruta = "/home/Pandax/Documents/VendigMachineLogs/ventas.txt"
    hora = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    linea = f"{hora} - Venta: {tipo}\n"
    
    try:
        with open(ruta, "a") as archivo:
            archivo.write(linea)
        print(f"Venta registrada: {tipo}")
    except Exception as e:
        print(f"⚠️ Error al registrar venta: {e}")

GPIO.add_event_detect(PIN_MONEDAS, GPIO.FALLING, callback=moneda_insertada, bouncetime=300)

print("Sistema listo. Esperando interacción...")

try:
    while True:
        if GPIO.input(PIN_BTN_galon) == GPIO.LOW and credito >= PRECIO_galon:
            dispensar(5)  # 5 seg por 1L aprox.
            registrar_venta("Galon")
            credito -= PRECIO_galon

        elif GPIO.input(PIN_BTN_GARRAFON) == GPIO.LOW and credito >= PRECIO_GARRAFON:
            dispensar(100)  # 20L = 100 seg aprox. ajusta según tu bomba
            registrar_venta("Garrafon")
            credito -= PRECIO_GARRAFON

        elif GPIO.input(PIN_BTN_LIMPIAR) == GPIO.LOW:
            print("🧼 Limpieza garrafón")
            dispensar(4)

        elif GPIO.input(PIN_BTN_CANCELAR) == GPIO.LOW and credito > 0:
            print(f"↩️ Cancelado. Devolviendo {credito} unidades")
            credito = 0

        time.sleep(0.1)

except KeyboardInterrupt:
    print("Apagando sistema...")
    GPIO.cleanup()
