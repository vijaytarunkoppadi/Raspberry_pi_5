from gpiozero import LED  # It includes gpio pin library
from time import sleep    # Its a time library 

led = LED(17)              # port 17 pin assigned as LED

while True:             
  led.on()                 #  LED on function call
  sleep(1)                 # One sec time delay
  led.off()                # LED off function call
  sleep(1)                 # One sec time delay 
