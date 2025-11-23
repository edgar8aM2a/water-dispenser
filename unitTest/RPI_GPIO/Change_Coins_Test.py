import RPi.GPIO as GPIO #Solo disponible en rasberry
import time

PIN_IR_HOPPER = 6

monedas_contadas = 0
def contar_pulso(channel):
	global monedas_contadas
	monedas_contadas+= 1
	print(f"Moneda detectada. Total: {monedas_contadas} pesos")
	
GPIO.setmode(GPIO.BCM)
GPIO.setup(PIN_IR_HOPPER, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.add_event_detect(PIN_IR_HOPPER, GPIO.FALLING, callback=contar_pulso,bouncetime=100)

print("esperando monedas del hopper")

try:
	while True:
		time.sleep(0.1)
except keyboardInterrupt:
	print("\n Finalizado")
	GPIO.cleanup()
