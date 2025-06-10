import pigpio
import time

pi = pigpio.pi()
if not pi.connected:
    exit("Could not connect to pigpiod")

gpio_pin = 13  # or any GPIO

frequency = 0.2  # Hz (0.5 Hz = 2 seconds per cycle)
period = 1 / frequency  # full period
half_period = period / 2  # high and low duration

print(f"Generating {frequency} Hz square wave on GPIO {gpio_pin}")

try:
    while True:
        pi.write(gpio_pin, 1)
        time.sleep(half_period)
        pi.write(gpio_pin, 0)
        time.sleep(half_period)
except KeyboardInterrupt:
    print("Stopped by user")
    pi.write(gpio_pin, 0)
    pi.stop()
