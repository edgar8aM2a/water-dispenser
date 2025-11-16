import RPi.GPIO as GPIO #Solo disponible en rasberry
import time

PIN_FLUJO = 5

contador_pulsos = 0

def contar_pulso(channel):
	global contador_pulsos
	contador_pulsos += 1
	
GPIO.setmode(GPIO.BCM)
GPIO.setup(PIN_FLUJO, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.add_event_detect(PIN_FLUJO, GPIO.FALLING, callback=contar_pulso)

print("midiendo flujo...")

try:
	while True:
		contador_pulsos = 0
		time.sleep(1) #mide por 1 seg
		flujo_L_min = contador_pulsos / 7.5 
		print(f"{flujo_L_min:2f} L/min ({contador_pulsos}")
except keyboardInterrupt:
	print("Finalizado")
	GPIO.cleanup()
