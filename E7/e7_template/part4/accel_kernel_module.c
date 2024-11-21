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
volatile uint8_t accel_id;


// Vars and prototypes for a char driver
#define SUCCESS 0
#define DEV_NAME_ACCEL "accel"

// Function prototypes
static int device_open_ACCEL (struct inode *, struct file *);
static int device_release_ACCEL (struct inode *, struct file *);
static ssize_t device_read_ACCEL (struct file *, char* , size_t , loff_t *);
static ssize_t device_write_ACCEL (struct file *, const char* , size_t , loff_t *);

static struct file_operations chardev_accel_fops = {
    .owner = THIS_MODULE,
    .open = device_open_ACCEL,
    .release = device_release_ACCEL,
    .read = device_read_ACCEL,
    .write = device_write_ACCEL
};

// Character device driver for /dev/video
static struct miscdevice chardev_accel = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEV_NAME_ACCEL,
    .fops = &chardev_accel_fops,
    .mode = 0666
};

#define MAX_SIZE 1000                         // we assume that no message will be longer than this
static char chardev_accel_msg[MAX_SIZE];     // the character array that can be read
static char chardev_read[MAX_SIZE];          // the character array that would be writen on
static char chardev_write[MAX_SIZE];          // the character array that would be writen on
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

    /**  Note: include this code in your __init function  **/
    I2C0_ptr = ioremap_nocache (I2C0_BASE, I2C0_SPAN);
    SYSMGR_ptr = ioremap_nocache (SYSMGR_BASE, SYSMGR_SPAN);

    if ((I2C0_ptr == NULL) || (SYSMGR_ptr == NULL))
        printk (KERN_ERR "Error: ioremap_nocache returned NULL!\n");

    pass_addrs((unsigned int*) SYSMGR_ptr, (unsigned int*) I2C0_ptr);


    Pinmux_Config();
    I2C0_Init();

    mg_per_lsb_char = calc_mg_per_lsb(XL345_10BIT, XL345_RANGE_16G);
    
    ADXL345_REG_READ(0x00, &accel_id);
    if (accel_id == 0xE5){

        ADXL345_Init();
        ADXL345_REG_WRITE(ADXL345_REG_POWER_CTL, XL345_STANDBY);
        ADXL345_REG_WRITE(ADXL345_REG_THRESH_TAP, 3 * 1000 /62.5);
        ADXL345_REG_WRITE(ADXL345_REG_DURATION, 0.2 * 1000 * 1000 / 625);
        ADXL345_REG_WRITE(ADXL345_REG_LATENCY, 0.02 * 1000 / 1.25);
        ADXL345_REG_WRITE(ADXL345_REG_WINDOW, 0.3 * 1000 / 1.25);
        ADXL345_REG_WRITE(ADXL345_REG_WINDOW, 0.3 * 1000 / 1.25);

        ADXL345_REG_WRITE(ADXL345_REG_INT_ENABLE,  XL345_SINGLETAP | XL345_DOUBLETAP | XL345_ACTIVITY | XL345_INACTIVITY | XL345_DATAREADY);	//enable interrupts
        ADXL345_REG_WRITE(ADXL345_REG_INT_MAP, XL345_SINGLETAP | XL345_DOUBLETAP | XL345_ACTIVITY | XL345_INACTIVITY | XL345_DATAREADY);	//enable interrupts
        ADXL345_REG_WRITE(ADXL345_REG_INT_SOURCE, XL345_SINGLETAP | XL345_DOUBLETAP | XL345_ACTIVITY | XL345_INACTIVITY | XL345_DATAREADY);	//enable interrupts
        ADXL345_REG_WRITE(ADXL345_REG_TAP_AXES, 0x07);
        ADXL345_REG_WRITE(ADXL345_REG_POWER_CTL, XL345_MEASURE);
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

static ssize_t device_write_ACCEL(struct file *filp, const char* buffer, size_t length, loff_t *offset) {
    
    size_t bytes;
    bytes = length;

    if (bytes > MAX_SIZE - 1)    // can copy all at once, or not?
        bytes = MAX_SIZE - 1;
    if (copy_from_user (chardev_write, buffer, bytes) != 0)
        printk (KERN_ERR "Error: copy_from_user unsuccessful");
    chardev_write[bytes] = '\0';    // NULL terminate
    // Note: we do NOT update *offset; we just copy the data into chardev_LEDR_msg

    uint8_t f, g;
    uint8_t r;
    int int_rate;

    if ( !strncmp(chardev_write, "device", 6) ) {                       // echo device > /dev/accel
        printk(KERN_INFO "ADXL345 device id = %X\n", accel_id);
    }
    else if ( !strncmp(chardev_write, "init", 4)){                      // echo init > /dev/accel
        printk(KERN_INFO "Re-initializing the device...\n");
        ADXL345_Init();
    }
    else if (!strncmp(chardev_write, "calibrate", 9)) {                 // echo calibrate > /dev/accel
        printk(KERN_INFO "Calibrating the device...\n");
        ADXL345_Calibrate();
    }
    else if (sscanf(chardev_write,"format %hhu %hhu",&f, &g) == 2) {    // echo format F G > /dev/accel
        
        uint8_t format;
        uint8_t range;

        // F = Data Format (resolution)
        if (f == 0){
            format = XL345_10BIT;
        } else if (f == 1){
            format = XL345_FULL_RESOLUTION;
        } else {
            printk(KERN_ERR "Wrong Data Format!!\n");
            return bytes;
        }

        // G = Range
        if (g == 2){
            range = XL345_RANGE_2G;
        } else if (g == 4){
            range = XL345_RANGE_4G;
        } else if (g == 8){
            range = XL345_RANGE_8G;
        } else if (g == 16){
            range = XL345_RANGE_16G;
        } else {
            printk(KERN_ERR "Wrong Data Format!!\n");
            return bytes;
        }

        printk(KERN_INFO "Changing device format and range...\n");
        
        ADXL345_REG_WRITE(ADXL345_REG_POWER_CTL, XL345_STANDBY);
        ADXL345_REG_WRITE(ADXL345_REG_DATA_FORMAT, range | format);
        ADXL345_REG_WRITE(ADXL345_REG_POWER_CTL, XL345_MEASURE);

    }
    else if (sscanf(chardev_write, "rate %hhu",&r)) {                   // echo rate R > /dev/accel
        /*
        *   NEVER DO FLOATING POINT OPERATIONS IN KERNEL SPACE !!!!!
        */

        //int_rate = (int)(r * 100);
        int_rate = r;
        uint8_t output_rate;
        if (int_rate == 10) {
            output_rate = XL345_RATE_0_10;
        }
        else if (int_rate == 20) {
            output_rate = XL345_RATE_0_20;
        }
        else if (int_rate == 39) {
            output_rate = XL345_RATE_0_39;
        }
        else if (int_rate == 78) {
            output_rate = XL345_RATE_0_78;
        }
        else if (int_rate == 156) {
            output_rate = XL345_RATE_1_56;
        }
        else if (int_rate == 313) {
            output_rate = XL345_RATE_3_13;
        }
        else if (int_rate == 625) {
            output_rate = XL345_RATE_6_25;
        }
        else if ( int_rate == 125) {
            output_rate = XL345_RATE_12_5;
        }
        else if (int_rate == 25) {
            output_rate = XL345_RATE_25;
        }
        else if (int_rate == 50) {
            output_rate = XL345_RATE_50;
        }
        else if (int_rate == 100) {
            output_rate = XL345_RATE_100;
        }
        else if (int_rate == 200) {
            output_rate = XL345_RATE_200;
        }
        else if (int_rate == 400) {
            output_rate = XL345_RATE_400;
        }
        else if (int_rate == 800) {
            output_rate = XL345_RATE_800;
        }
        else if (int_rate == 1600) {
            output_rate = XL345_RATE_1600;
        }
        else if (int_rate == 3200) {
            output_rate = XL345_RATE_3200;
        }
        else {
            printk(KERN_ERR "Wrong Output Rate !!!\n");
            return bytes;
        }

        printk(KERN_INFO "output rate = %hhu\n", r);

        ADXL345_REG_WRITE(ADXL345_REG_POWER_CTL, XL345_STANDBY);
        ADXL345_REG_WRITE(ADXL345_REG_BW_RATE, output_rate);
        ADXL345_REG_WRITE(ADXL345_REG_POWER_CTL, XL345_MEASURE);
    }
    else {
        printk(KERN_ERR "Wrong Command\n");
    }

    return bytes;

}

static ssize_t device_read_ACCEL(struct file *filp, char* buffer, size_t length, loff_t *offset) {

    if (*offset > 0) {
        *offset = 0;
        return 0;
    }
    
    uint8_t src;
    volatile uint8_t single_tap = 0;
    volatile uint8_t double_tap = 0;
    ADXL345_REG_READ(ADXL345_REG_INT_SOURCE, &src);

    if(src & XL345_SINGLETAP) {
        single_tap = XL345_SINGLETAP;
        printk(KERN_INFO "Single tap detected\n");
    }
    if(src & XL345_DOUBLETAP) {
        double_tap = XL345_DOUBLETAP;
        printk(KERN_INFO "Double tap detected\n"); 
    }

    int16_t XYZ_data[3];
    ADXL345_XYZ_Read(XYZ_data);
    
    if(snprintf(chardev_read, MAX_SIZE, "%02X %d %d %d %d\n", src, XYZ_data[0], XYZ_data[1], XYZ_data[2], mg_per_lsb_char) < 0) {
        printk(KERN_ERR "Error: snprintf unsuccessful");
        return -EFAULT;
    }

    size_t bytes = strlen(chardev_read) - (*offset);
    bytes = bytes > length ? length : bytes;

    if (bytes > 0) {
        if (copy_to_user(buffer, &chardev_read[*offset], bytes) != 0) {
            printk(KERN_ERR "Error: copy_to_user unsuccessful");
            return -EFAULT;
        }
        *offset = bytes;
    }

    return bytes;
}

MODULE_LICENSE("GPL");
module_init (start_accel);
module_exit (stop_accel);