import pigpio
import time

pi = pigpio.pi()
# Script to generate a square wave using hardware PWM on Raspberry Pi

# Use GPIO26
gpio_pin_a = 12
frequency = 0.5  # 0.5Hz square wave
duty_cycle = 500000  # 50% duty (range: 0 to 1_000_000)

pi.hardware_PWM(gpio_pin_a, frequency, duty_cycle)
print("Generating square wave on GPIO pin {}".format(gpio_pin_a))

# Use GPIO19
gpio_pin_b = 13
frequency = 0.2  # Hz square wave
duty_cycle = 500000  # 50% duty (range: 0 to 1_000_000)
pi.hardware_PWM(gpio_pin_b, frequency, duty_cycle)
print("Generating square wave on GPIO pin {}".format(gpio_pin_b))

time.sleep(10)  # Output square wave for 10 seconds
pi.hardware_PWM(gpio_pin_a, 0, 0)  # Stop PWM
pi.hardware_PWM(gpio_pin_b, 0, 0)  # Stop PWM
