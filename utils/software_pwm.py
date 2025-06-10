import pigpio
import time

pi = pigpio.pi()
if not pi.connected:
    exit("Could not connect to pigpiod")

# Pines GPIO
gpio_pin_a = 18  # Software PWM
gpio_pin_b = 13  # Software PWM

# Configuración
frequency_a = 2    # Hz
duty_cycle_a = 128  # 50% of 255

frequency_b = 4    # Hz
duty_cycle_b = int(255 * 0.75)  # 75% of 255

# Establecer frecuencia y duty cycle
pi.set_PWM_frequency(gpio_pin_a, frequency_a)
pi.set_PWM_dutycycle(gpio_pin_a, duty_cycle_a)
print(f"Generating {frequency_a}Hz (50%) on GPIO {gpio_pin_a} using software PWM")

pi.set_PWM_frequency(gpio_pin_b, frequency_b)
pi.set_PWM_dutycycle(gpio_pin_b, duty_cycle_b)
print(f"Generating {frequency_b}Hz (75%) on GPIO {gpio_pin_b} using software PWM")

# Ejecutar por 10 segundos
time.sleep(10)

# Detener PWM
pi.set_PWM_dutycycle(gpio_pin_a, 0)
pi.set_PWM_dutycycle(gpio_pin_b, 0)
pi.stop()
