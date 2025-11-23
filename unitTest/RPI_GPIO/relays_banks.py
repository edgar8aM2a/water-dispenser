import RPi.GPIO as GPIO #Solo disponible en rasberry
import time

PIN_BOMBA = 18
PIN_HOPPER = 16

GPIO.setmode(GPIO.BCM)
GPIO.setup(PIN_BOMBA, GPIO.OUT)
GPIO.setup(PIN_HOPPER, GPIO.OUT)
GPIO.output(PIN_HOPPER, GPIO.HIGH)
GPIO.output(PIN_BOMBA, GPIO.HIGH)

print("control manual")
print(""" Presiona:
		b 	bomba
		c	hopper
		q	salir""")
try:
	while True:
		opcion = input("comando ").strip().lower()
		
		if opcion == "b":
			print("activando bomba")
			GPIO.output(PIN_BOMBA, GPIO.LOW)
			time.sleep(3)
			GPIO.output(PIN_BOMBA, GPIO.HIGH)
			print("bomba apagada")
		elif opcion == "c":
			print("activando Hopper")
			GPIO.output(PIN_HOPPER, GPIO.LOW)
			time.sleep(3)
			GPIO.output(PIN_HOPPER, GPIO.HIGH)
			print("hopper apagada")
		elif opcion == 'q':
			print('saliendo')
			break
		else:
			print("no reconocido")
except keyboardInterrupt:
	print(" interrumpido por el usuario")

finally:
	GPIO.cleanup()
