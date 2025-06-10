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


/**
 * dev_open - Se invoca al abrir el dispositivo.
 */
static int dev_open(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "Dispositivo GPIO: abierto correctamente\n");
    return 0;
}

/**
 * dev_close - Se invoca al cerrar el dispositivo.
 */
static int dev_close(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "Dispositivo GPIO: cerrado correctamente\n");
    return 0;
}

/**
 * dev_read - Permite al usuario leer el estado actual del GPIO seleccionado.
 */
static ssize_t dev_read(struct file* filep, char* buffer, size_t len, loff_t* offset) {
    char msg[128];
    int msg_len;

    printk(KERN_INFO "Lectura solicitada desde espacio de usuario\n");

    msg_len = snprintf(msg, sizeof(msg), "Señal activa: %d\n", current_gpio_value);

    if (*offset >= msg_len)
        return 0;

    if (len > msg_len - *offset)
        len = msg_len - *offset;

    if (copy_to_user(buffer, msg + *offset, len) != 0) {
        printk(KERN_ERR "Error: no se pudo copiar al espacio de usuario\n");
        return -EFAULT;
    }

    *offset += len;
    return len;
}

/**
 * dev_write - Permite al usuario escribir 1 o 2 para seleccionar el pin GPIO.
 */
static ssize_t dev_write(struct file* filep, const char* buffer, size_t len, loff_t* offset) {
    char input[64];

    printk(KERN_INFO "Escritura recibida desde espacio de usuario\n");

    if (len > sizeof(input) - 1)
        len = sizeof(input) - 1;

    if (copy_from_user(input, buffer, len) != 0) {
        printk(KERN_ERR "Error: no se pudo copiar desde el espacio de usuario\n");
        return -EFAULT;
    }

    input[len] = '\0';

    if (kstrtoint(input, 10, &selected_signal) != 0) {
        printk(KERN_ERR "Entrada inválida para selección de señal: %s\n", input);
        return -EFAULT;
    }

    printk(KERN_INFO "Señal seleccionada: GPIO %d\n", selected_signal);
    return len;
}


static struct file_operations operaciones_fichero = {
    .owner = THIS_MODULE,
    .open = dispositivo_abierto,
    .release = dispositivo_cerrado,
    .read = dispositivo_leer,
    .write = dispositivo_escribir,
};

/**
 * modulo_inicializar - Inicializa el módulo, GPIOs y dispositivo de carácter.
 */
static int __init modulo_inicializar(void) {
    struct device* dispositivo = NULL;

    printk(KERN_INFO "Inicializando el controlador GPIO personalizado\n");

    numero_mayor = register_chrdev(0, NOMBRE_DISPOSITIVO, &operaciones_fichero);
    if (numero_mayor < 0) {
        printk(KERN_ALERT "No se pudo registrar el número mayor\n");
        return numero_mayor;
    }

    clase = class_create(CLASE_DISPOSITIVO);
    if (IS_ERR(clase)) {
        unregister_chrdev(numero_mayor, NOMBRE_DISPOSITIVO);
        printk(KERN_ALERT "Error al crear la clase del dispositivo\n");
        return PTR_ERR(clase);
    }

    dispositivo = device_create(clase, NULL, MKDEV(numero_mayor, 0), NULL, NOMBRE_DISPOSITIVO);
    if (IS_ERR(dispositivo)) {
        class_destroy(clase);
        unregister_chrdev(numero_mayor, NOMBRE_DISPOSITIVO);
        printk(KERN_ALERT "Error al crear el dispositivo\n");
        return PTR_ERR(dispositivo);
    }

    // Verificación y configuración de los GPIOs
    if (!gpio_is_valid(PIN_GPIO_A))
        printk(KERN_ERR "GPIO %d no es válido\n", PIN_GPIO_A);

    if (!gpio_is_valid(PIN_GPIO_B))
        printk(KERN_ERR "GPIO %d no es válido\n", PIN_GPIO_B);

    if (gpio_request(PIN_GPIO_B, "GPIO_B") < 0)
        printk(KERN_ERR "No se pudo solicitar GPIO %d\n", PIN_GPIO_B);

    gpio_direction_input(PIN_GPIO_A);

    if (gpio_request(PIN_GPIO_A, "GPIO_A") < 0)
        printk(KERN_ERR "No se pudo solicitar GPIO %d\n", PIN_GPIO_A);

    gpio_direction_input(PIN_GPIO_B);

    // Configuración del temporizador
    timer_setup(&temporizador, temporizador_callback, 0);
    mod_timer(&temporizador, jiffies + INTERVALO_TEMPORIZADOR);

    printk(KERN_INFO "GPIOs configurados y temporizador activado\n");
    return 0;
}

/**
 * modulo_salir - Limpia recursos y elimina el dispositivo al salir.
 */
static void __exit modulo_salir(void) {
    printk(KERN_INFO "Saliendo del controlador GPIO\n");

    del_timer(&temporizador);
    gpio_free(PIN_GPIO_A);
    gpio_free(PIN_GPIO_B);

    device_destroy(clase, MKDEV(numero_mayor, 0));
    class_unregister(clase);
    class_destroy(clase);
    unregister_chrdev(numero_mayor, NOMBRE_DISPOSITIVO);

    printk(KERN_INFO "Controlador GPIO descargado correctamente\n");
}

module_init(modulo_inicializar);
module_exit(modulo_salir);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Facundo Galvagno");
MODULE_DESCRIPTION("Controlador GPIO personalizado con temporizador para Linux");
MODULE_VERSION("1.1");
