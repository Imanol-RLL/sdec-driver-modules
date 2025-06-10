#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/device.h>

#define NOMBRE_DISPOSITIVO "control_gpio"
#define CLASE_DISPOSITIVO "clase_gpio"
#define PIN_GPIO_A 532
#define PIN_GPIO_B 533
#define INTERVALO_TEMPORIZADOR HZ  // 1 segundo

static int numero_mayor;
static struct class* clase = NULL;
static struct timer_list temporizador;

static int senal_seleccionada = 1; // 1 para PIN_GPIO_A, 2 para PIN_GPIO_B
static int estado_gpio = 0;

/**
 * temporizador_callback - Función de temporizador que lee el valor GPIO.
 */
static void temporizador_callback(struct timer_list* t) {
    printk(KERN_INFO "Temporizador: ejecutando lectura de GPIO\n");

    if (senal_seleccionada == 1)
        estado_gpio = gpio_get_value(PIN_GPIO_A);
    else
        estado_gpio = gpio_get_value(PIN_GPIO_B);

    printk(KERN_INFO "Valor actual del GPIO (%d): %d\n", senal_seleccionada, estado_gpio);
    mod_timer(&temporizador, jiffies + INTERVALO_TEMPORIZADOR);
}

/**
 * dispositivo_abierto - Función cuando se abre el dispositivo.
 */
static int dispositivo_abierto(struct inode *i, struct file *f) {
    printk(KERN_INFO "Dispositivo GPIO: abierto correctamente\n");
    return 0;
}

/**
 * dispositivo_cerrado - Función cuando se cierra el dispositivo.
 */
static int dispositivo_cerrado(struct inode *i, struct file *f) {
    printk(KERN_INFO "Dispositivo GPIO: cerrado correctamente\n");
    return 0;
}

/**
 * dispositivo_leer - Función de lectura del dispositivo.
 */
static ssize_t dispositivo_leer(struct file* f, char* buffer, size_t longitud, loff_t* offset) {
    char mensaje[128];
    int longitud_mensaje;

    printk(KERN_INFO "Lectura del dispositivo solicitada\n");

    longitud_mensaje = snprintf(mensaje, sizeof(mensaje), "Señal actual: %d\n", estado_gpio);

    if (*offset >= longitud_mensaje)
        return 0;

    if (longitud > longitud_mensaje - *offset)
        longitud = longitud_mensaje - *offset;

    if (copy_to_user(buffer, mensaje + *offset, longitud) != 0) {
        printk(KERN_ERR "Error al copiar datos al espacio de usuario\n");
        return -EFAULT;
    }

    *offset += longitud;
    return longitud;
}

/**
 * dispositivo_escribir - Permite al usuario seleccionar el GPIO a observar.
 */
static ssize_t dispositivo_escribir(struct file* f, const char* buffer, size_t longitud, loff_t* offset) {
    char entrada[64];

    printk(KERN_INFO "Escritura recibida por el dispositivo\n");

    if (longitud > sizeof(entrada) - 1)
        longitud = sizeof(entrada) - 1;

    if (copy_from_user(entrada, buffer, longitud) != 0) {
        printk(KERN_ERR "Error al copiar desde el espacio de usuario\n");
        return -EFAULT;
    }

    entrada[longitud] = '\0';

    if (kstrtoint(entrada, 10, &senal_seleccionada) != 0) {
        printk(KERN_ERR "Error: entrada inválida para selección de señal: %s\n", entrada);
        return -EFAULT;
    }

    return longitud;
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
