#include <linux/kernel.h>
#include <linux/fs.h>               // struct file, struct file_operations
#include <linux/init.h>             // for __init, see code
#include <linux/module.h>           // for module init and exit macros
#include <linux/miscdevice.h>       // for misc_device_register and struct miscdev
#include <linux/uaccess.h>          // for copy_to_user, see code
#include <asm/io.h>                 // for mmap

#include "../include/address_map_arm.h"
#include "../include/ADXL345.h"

// Declare global variables needed to use the accelerometer
volatile unsigned int * I2C0_ptr;       // virtual address for I2C communication
volatile unsigned int * SYSMGR_ptr;     // virtual address for System Manager communication
volatile int16_t mg_per_lsb_char;


// Vars and prototypes for a char driver
#define SUCCESS 0
#define DEV_NAME_ACCEL "accel"

// Function prototypes
static int device_open_ACCEL (struct inode *, struct file *);
static int device_release_ACCEL (struct inode *, struct file *);
static ssize_t device_read_ACCEL (struct file *, char* , size_t , loff_t *);

static struct file_operations chardev_accel_fops = {
    .owner = THIS_MODULE,
    .open = device_open_ACCEL,
    .release = device_release_ACCEL,
    .read = device_read_ACCEL
};

// Character device driver for /dev/video
static struct miscdevice chardev_accel = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEV_NAME_ACCEL,
    .fops = &chardev_accel_fops,
    .mode = 0666
};

#define MAX_SIZE 300                         // we assume that no message will be longer than this
static char chardev_accel_msg[MAX_SIZE];     // the character array that can be read
static char chardev_read[MAX_SIZE];          // the character array that would be writen on
static int chardev_accel_registered = 0;





static int __init start_accel(void) {

    // Initialize the miscdevice data structures
    int err_accel = misc_register (&chardev_accel);
    if (err_accel < 0) {
        printk (KERN_ERR "/dev/%s: misc_register() failed\n", DEV_NAME_ACCEL);
    } else {
        printk (KERN_INFO "/dev/%s driver registered\n", DEV_NAME_ACCEL);
        chardev_accel_registered = 1;
    }

    
    I2C0_ptr = ioremap_nocache (I2C0_BASE, I2C0_SPAN);
    SYSMGR_ptr = ioremap_nocache (SYSMGR_BASE, SYSMGR_SPAN);

    if ((I2C0_ptr == NULL) || (SYSMGR_ptr == NULL))
        printk (KERN_ERR "Error: ioremap_nocache returned NULL!\n");

    pass_addrs((unsigned int*) SYSMGR_ptr, (unsigned int*) I2C0_ptr);


    Pinmux_Config();
    I2C0_Init();

    mg_per_lsb_char = calc_mg_per_lsb(XL345_10BIT, XL345_RANGE_16G);

    uint8_t accel_id;
    ADXL345_REG_READ(0x00, &accel_id);
    if (accel_id == 0xE5){
        ADXL345_Init();
        ADXL345_Calibrate();
    }

    return 0;
}



static void __exit stop_accel(void) {

    iounmap (I2C0_ptr);
    iounmap (SYSMGR_ptr);
    
    /* Remove the device from the kernel */
    if(chardev_accel_registered){
        misc_deregister(&chardev_accel);
        printk(KERN_INFO "/dev/%s driver de-registered\n", DEV_NAME_ACCEL);
    }
}

static int device_open_ACCEL(struct inode *inode, struct file *file){
    return SUCCESS;
}

static int device_release_ACCEL(struct inode *inode, struct file *file){
    return 0;
}

static ssize_t device_read_ACCEL(struct file *filp, char* buffer, size_t length, loff_t *offset) {

    int16_t XYZ_data[3];
    
    ADXL345_XYZ_Read(XYZ_data);
    
    if(sprintf(chardev_read, "%d %d %d %d %d\n", ADXL345_WasActivityUpdated(), XYZ_data[0], XYZ_data[1], XYZ_data[2], mg_per_lsb_char) < 0 ){
        printk(KERN_ERR "Error: copy_to_user unsuccessful");
    }

    size_t bytes;
    bytes = strlen(chardev_read) - (*offset);    // how many bytes not yet sent?
    bytes = bytes > length ? length : bytes;     // too much to send all at once?

    if (bytes)
        if (copy_to_user(buffer, &chardev_read[*offset], bytes) != 0)
            printk(KERN_ERR "Error: copy_to_user unsuccessful");
    *offset = bytes; // keep track of number of bytes sent to the user

    return bytes;
}

MODULE_LICENSE("GPL");
module_init (start_accel);
module_exit (stop_accel);