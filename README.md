## Drivers de Kernel
Los drivers de caracteres
es de las formas mas simples de comunicarse al kernel de linux, se trabaja en módulos que pueden agregarse y desacoplarse de forma controlada. dando independencia al sistema de sus drivers en caso de falla.
Estos módulos actúan como intermediarios entre el espacio de usuario y el hardware o servicios del sistema, ofreciendo una abstracción que simplifica enormemente el desarrollo y mantenimiento de controladores.
![Arquitectura de capas de drivers](https://github.com/Imanol-RLL/sdec-driver-modules/blob/main/res/Pasted%20image%2020250606223749.png)

Implementacion:

en C99 la implementacion mas simple de operaciones de archivos es 

```C
    struct file_operations fops = {
       .read = device_read,
       .write = device_write,
       .open = device_open,
       .release = device_release
    };
```
** Conceptos:
- **Major number**: Número principal que identifica el controlador de dispositivo
    
- **Minor number**: Número secundario que identifica dispositivos específicos manejados por el mismo controlador
    
- **sysfs**: Sistema de archivos virtual que representa información del kernel
    
- **nodo de dispositivo**: Archivo especial en /dev que permite interactuar con el dispositivo

Luego, haciendo uso de Major y Minor number, debemos registrar nuestro dispositivo en el sistema, de forma:

```C
void mychardev_init(void)
{
    int err, i;
    dev_t dev;

    // Asignar región de dispositivos de caracteres y asignar número Major
    err = alloc_chrdev_region(&dev, 0, MAX_DEV, "mychardev");

    dev_major = MAJOR(dev);

    // Crear clase en sysfs
    mychardev_class = class_create(THIS_MODULE, "mychardev");

    // Crear la cantidad necesaria de dispositivos
    for (i = 0; i < MAX_DEV; i++) {
        // Inicializar nuevo dispositivo
        cdev_init(&mychardev_data[i].cdev, &mychardev_fops);
        mychardev_data[i].cdev.owner = THIS_MODULE;

        // Añadir dispositivo al sistema donde "i" es el número Minor del nuevo dispositivo
        cdev_add(&mychardev_data[i].cdev, MKDEV(dev_major, i), 1);

        // Crear nodo de dispositivo /dev/mychardev-x donde "x" es "i", igual al número Minor
        device_create(mychardev_class, NULL, MKDEV(dev_major, i), NULL, "mychardev-%d", i);
    }
}
```


## Funciones de I/O
 Funciones de E/S para dispositivos de carácter
 Para interactuar con el archivo de dispositivo, necesitamos asignar funciones  a la estructura file_operations, que define cómo el kernel manejará las operaciones del dispositivo. 
```C
/*
 * Función: mychardev_open
 * Descripción: Se ejecuta cuando un proceso abre el dispositivo
 * Parámetros:
 *   - inode: Estructura que representa el archivo especial del dispositivo
 *   - file: Estructura que representa el archivo abierto
 * Retorna: 0 en éxito, código de error en caso contrario
 */
static int mychardev_open(struct inode *inode, struct file *file)
{
    printk("MYCHARDEV: Dispositivo abierto\n");
    return 0;
}

/*
 * Función: mychardev_release
 * Descripción: Se ejecuta cuando un proceso cierra el dispositivo
 * Parámetros:
 *   - inode: Estructura del archivo especial
 *   - file: Estructura del archivo abierto
 * Retorna: 0 en éxito, código de error en caso contrario
 */
static int mychardev_release(struct inode *inode, struct file *file)
{
    printk("MYCHARDEV: Dispositivo cerrado\n");
    return 0;
}

/*
 * Función: mychardev_ioctl
 * Descripción: Maneja operaciones de control específicas del dispositivo
 * Parámetros:
 *   - file: Estructura del archivo abierto
 *   - cmd: Comando de control específico del dispositivo
 *   - arg: Argumento pasado para el comando
 * Retorna: 0 en éxito, código de error en caso contrario
 */
static long mychardev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    printk("MYCHARDEV: Operación ioctl en dispositivo\n");
    return 0;
}

/*
 * Función: mychardev_read
 * Descripción: Lee datos desde el dispositivo hacia el espacio de usuario
 * Parámetros:
 *   - file: Estructura del archivo abierto
 *   - buf: Buffer en espacio de usuario donde se copiarán los datos
 *   - count: Cantidad de bytes a leer
 *   - offset: Posición en el archivo desde donde leer
 * Retorna: Número de bytes leídos, o código de error
 */
static ssize_t mychardev_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
    printk("MYCHARDEV: Lectura de dispositivo\n");
    return 0;
}

/*
 * Función: mychardev_write
 * Descripción: Escribe datos desde el espacio de usuario al dispositivo
 * Parámetros:
 *   - file: Estructura del archivo abierto
 *   - buf: Buffer en espacio de usuario con datos a escribir
 *   - count: Cantidad de bytes a escribir
 *   - offset: Posición en el archivo donde escribir
 * Retorna: Número de bytes escritos, o código de error
 */
static ssize_t mychardev_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
    printk("MYCHARDEV: Escritura en dispositivo\n");
    return 0;
}
```

## Conexión remota del dispositivo
Para este trabajo se implemento una overlay network de codigo libre llamada ZeroTier, de forma que cada integrante del equipo pudiera acceder a la raspberry pi.

su funcionamiento de forma simplificadad es

![Overlay Network](https://github.com/Imanol-RLL/sdec-driver-modules/blob/main/res/Pasted%20image%2020250606221251.png)


# Estructura de un Network ID de ZeroTier

``` mermaid
graph TD;
    A[8056c2e21c123456] --> B[Primeros 10 caracteres];
    A --> C[Últimos 6 caracteres];
    B --> D["Dirección ZeroTier del controlador"];
    C --> E["Número de red en el controlador"];
 ```

En detalle

 **ZeroTier crea una VPN mesh**:
    
    - Todos los dispositivos se conectan entre sí mediante túneles cifrados (UDP).
        
    - No requiere configuración de puertos en el router (funciona detrás de NAT).
        
 **Asignación de IPs privadas**:
    
    - ZeroTier asigna direcciones IP únicas en la red virtual (`10.147.20.0/24`).
        
 **Seguridad**:
    
    -  **Cifrado AES-256**: Todos los datos viajan encriptados.
        
    - **Autenticación centralizada**: Solo dispositivos autorizados en ZeroTier Central pueden unirse.
-


bibliografia:

https://harivemula.com/2021/09/18/routing-all-traffic-through-home-with-zerotier-on-travel/
https://olegkutkov.me/2018/03/14/simple-linux-character-device-driver/
