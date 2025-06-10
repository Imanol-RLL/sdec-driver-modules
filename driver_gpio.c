#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/device.h>

#define DEVICE_NAME "control_gpio"
#define CLASS_NAME "clase_gpio"
#define GPIO_A 532
#define GPIO_B 533

static struct class* gpio_class = NULL;

static int selected_signal = 1; // 1 para GPIO_A, 2 para GPIO_B
static int current_gpio_value = 0;

/**
 * read_gpio_timer - Función llamada periódicamente para leer el valor GPIO.
 */
static void read_gpio_timer(struct timer_list* t) {
    printk(KERN_INFO "Temporizador: leyendo el valor del pin GPIO...\n");

    if (selected_signal == 1)
        current_gpio_value = gpio_get_value(GPIO_A);
    else
        current_gpio_value = gpio_get_value(GPIO_B);

    printk(KERN_INFO "Lectura GPIO %d -> valor: %d\n", selected_signal, current_gpio_value);

    mod_timer(&gpio_timer, jiffies + TIMER_INTERVAL);
}

