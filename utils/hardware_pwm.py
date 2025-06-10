import pigpio
import time

pi = pigpio.pi()
# Script to generate a square wave using hardware PWM on Raspberry Pi
# Use GPIO26
gpio_pin = 26
frequency = 1000  # 1 kHz square wave
duty_cycle = 500000  # 50% duty (range: 0 to 1_000_000)

pi.hardware_PWM(gpio_pin, frequency, duty_cycle)


print("Generating square wave on GPIO pin {}".format(gpio_pin))

time.sleep(10)  # Output square wave for 10 seconds
pi.hardware_PWM(gpio_pin, 0, 0)  # Stop PWM
