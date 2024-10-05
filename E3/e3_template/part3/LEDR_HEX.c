#include <linux/fs.h>               // struct file, struct file_operations
#include <linux/init.h>             // for __init, see code
#include <linux/module.h>           // for module init and exit macros
#include <linux/miscdevice.h>       // for misc_device_register and struct miscdev
#include <linux/uaccess.h>          // for copy_to_user, see code
#include <asm/io.h>                 // for mmap
#include "../include/address_map_arm_vm.h"

/*
****************************************************
GLOBAL VARS
*/
void *LW_virtual;			 // used to map physical addresses for the light-weight bridge
volatile int *HEX3_HEX0_ptr;  // virtual pointer to HEX displays
volatile int *HEX5_HEX4_ptr;
volatile int *LEDR_ptr;

//Bit patterns to display digits 0 - 9 on HEXs
static const char SEG[] = {
	0b00111111, //0
	0b00000110, //1
	0b01011011, //2
	0b01001111, //3
	0b01100110, //4
	0b01101101, //5
	0b01111101, //6
	0b00000111, //7
	0b01111111, //8
	0b01101111  //9
};
/*
****************************************************
*/


//Kernel functions for device driver /dev/LEDR
static int device_open_LEDR (struct inode *, struct file *);
static int device_release_LEDR (struct inode *, struct file *);
static ssize_t device_write_LEDR (struct file *, const char *, size_t, loff_t *);

//Kernel functions for device driver /dev/HEX
static int device_open_HEX (struct inode *, struct file *);
static int device_release_HEX (struct inode *, struct file *);
static ssize_t device_write_HEX (struct file *, const char *, size_t, loff_t *);


static struct file_operations chardev_LEDR_fops = {
    .owner = THIS_MODULE,
    .open = device_open_LEDR,
    .release = device_release_LEDR,
    .write = device_write_LEDR
};

static struct file_operations chardev_HEX_fops = {
    .owner = THIS_MODULE,
    .open = device_open_HEX,
    .release = device_release_HEX,
    .write = device_write_HEX
};


#define SUCCESS 0
#define DEV_NAME_LEDR "LEDR"
#define DEV_NAME_HEX "HEX"

//character device driver for LEDR
static struct miscdevice chardev_LEDR = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEV_NAME_LEDR,
    .fops = &chardev_LEDR_fops,
    .mode = 0666
};

//character device driver for HEX
static struct miscdevice chardev_HEX = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEV_NAME_HEX,
    .fops = &chardev_HEX_fops,
    .mode = 0666
};

static int chardev_LEDR_registered = 0;
static int chardev_HEX_registered = 0;

#define MAX_SIZE 256                     // we assume that no message will be longer than this
static char chardev_LEDR_msg[MAX_SIZE];  // the character array that can be read
static char chardev_HEX_msg[MAX_SIZE];   // the character array that can be read





//Initialization function for driver functions
//Init BOTH KEY and SW driver functions
static int __init init_drivers(void)
{

    //init LEDR driver
    int err_LEDR = misc_register (&chardev_LEDR);
    if (err_LEDR < 0) {
        printk (KERN_ERR "/dev/%s: misc_register() failed\n", DEV_NAME_LEDR);
    }
    else {
        printk (KERN_INFO "/dev/%s driver registered\n", DEV_NAME_LEDR);
        chardev_LEDR_registered = 1;
    }
    strcpy (chardev_LEDR_msg, "Hello from chardev_LEDR\n"); /* initialize the message */


    //init HEX driver
    int err_HEX = misc_register (&chardev_HEX);
    if (err_HEX < 0) {
        printk (KERN_ERR "/dev/%s: misc_register() failed\n", DEV_NAME_HEX);
    }
    else {
        printk (KERN_INFO "/dev/%s driver registered\n", DEV_NAME_HEX);
        chardev_HEX_registered = 1;
    }
    strcpy (chardev_HEX_msg, "Hello from chardev_HEX\n"); /* initialize the message */



    /*
    **************************************************
    initialize virtual pointers (addresses) to LEDR, HEX
    **************************************************
    */
    LW_virtual = ioremap(LW_BRIDGE_BASE, LW_BRIDGE_SPAN);
	HEX3_HEX0_ptr = LW_virtual + HEX3_HEX0_BASE;
    HEX5_HEX4_ptr = LW_virtual + HEX5_HEX4_BASE;
    LEDR_ptr = LW_virtual + LEDR_BASE;

    return err_LEDR && err_HEX;
}

//when drivers are removed from the kernel
static void __exit stop_drivers(void)
{
    if (chardev_LEDR_registered) {
        misc_deregister (&chardev_LEDR);
        printk (KERN_INFO "/dev/%s driver de-registered\n", DEV_NAME_LEDR);
    }

    if (chardev_HEX_registered) {
        misc_deregister (&chardev_HEX);
        printk (KERN_INFO "/dev/%s driver de-registered\n", DEV_NAME_HEX);
    }

    //Reset LEDR and HEX displays
    *LEDR_ptr = 0;
    *HEX3_HEX0_ptr = 0;
    *HEX5_HEX4_ptr = 0;

    //Unmap I/O memory from kernel address space
    iounmap(LW_virtual);
}





/*
--------------------------------------------------------------------------------------------------
Functions for LEDR device driver
*/
/* Called when a process opens chardev_KEY */
static int device_open_LEDR(struct inode *inode, struct file *file) {
    return SUCCESS;
}

/* Called when a process closes chardev_KEY */
static int device_release_LEDR(struct inode *inode, struct file *file) {
    return 0;
}

/* Called when a process writes to chardev_LEDR
 * Stores the data received into chardev_msg
 * and returns the number of bytes stored. */
static ssize_t device_write_LEDR(struct file *filp, const char *buffer, size_t length, loff_t *offset)
{
    int ledr_value;
    size_t bytes;
    bytes = length;

    if (bytes > MAX_SIZE - 1)    // can copy all at once, or not?
        bytes = MAX_SIZE - 1;
    if (copy_from_user (chardev_LEDR_msg, buffer, bytes) != 0)
        printk (KERN_ERR "Error: copy_from_user unsuccessful");
    chardev_LEDR_msg[bytes] = '\0';    // NULL terminate
    // Note: we do NOT update *offset; we just copy the data into chardev_LEDR_msg

    /*
    Takes input as HEXADECIMAL VALUE
    */
    sscanf (chardev_LEDR_msg, "%X", &ledr_value); //Parse command to get LEDR value to write
    *LEDR_ptr = ledr_value;                       //Display value on LEDR

    return bytes;
}
/*
--------------------------------------------------------------------------------------------------
*/




/*
--------------------------------------------------------------------------------------------------
Functions for HEX device driver
*/
/* Called when a process opens chardev_SW */
static int device_open_HEX(struct inode *inode, struct file *file) {
    return SUCCESS;
}

/* Called when a process closes chardev_SW */
static int device_release_HEX(struct inode *inode, struct file *file) {
    return 0;
}

/* Called when a process writes to chardev_HEX
 * Stores the data received into chardev_msg, and 
 * returns the number of bytes stored. */
static ssize_t device_write_HEX(struct file *filp, const char *buffer, size_t length, loff_t *offset)
{
    int input;
    int digit_array[6];
    int digit_divide = 100000;
    int i;
    static char str_value_input[8];
    int scan_success;
    int not_digit = 0;
    size_t bytes;
    bytes = length;

    if (bytes > MAX_SIZE - 1)    // can copy all at once, or not?
        bytes = MAX_SIZE - 1;
    if (copy_from_user (chardev_HEX_msg, buffer, bytes) != 0)
        printk (KERN_ERR "Error: copy_from_user unsuccessful");
    chardev_HEX_msg[bytes] = '\0';    // NULL terminate
    // Note: we do NOT update *offset; we just copy the data into chardev_msg


    scan_success = sscanf (chardev_HEX_msg, "%6d", &input);      //Scan for 6 digit integer

    sscanf (chardev_HEX_msg, "%6s", str_value_input);            //Scan command as string to double check
    int string_length = strlen(str_value_input);
    //printk (KERN_ERR "Length: %d", string_length);
    for(i = 0; i < string_length; i++){                          //DOUBLE CHECK: if there are any non-digit chars!
        if( str_value_input[i] != '0' && str_value_input[i] != '1' && str_value_input[i] != '2' &&
            str_value_input[i] != '3' && str_value_input[i] != '4' && str_value_input[i] != '5' &&
            str_value_input[i] != '6' && str_value_input[i] != '7' && str_value_input[i] != '8' &&
            str_value_input[i] != '9' ) not_digit = 1;
    }
    

    /*****************************************************
    //Bad argument if:
    // - command is not 6 digits
    // - command includes non-digit characters
    (CHANGE IF NECESSARY FOR PART4) = remove strlen(chardev_HEX_msg) == 7 && !not_digit

    //Only accept input as: DDDDDD --> 6 digit integer
    ******************************************************
    */
    if( strlen(chardev_HEX_msg) == 7 && scan_success && !not_digit && input >= 0 && input <= 999999 ){
        //Need to split all the digits of the number received (6 total)
        for(i = 0; i <= 5; i++){
            digit_array[i] = input / digit_divide;
            input = input % digit_divide;
            digit_divide = digit_divide / 10;
        }

        //Write 6 digits to 7-seg pointers
        *HEX3_HEX0_ptr = (SEG[digit_array[5]]) | (SEG[digit_array[4]] << 8) | (SEG[digit_array[3]] << 16) | (SEG[digit_array[2]]) << 24;
        *HEX5_HEX4_ptr = (SEG[digit_array[1]]) | (SEG[digit_array[0]] << 8);
    }
    else{
        printk (KERN_ERR "Bad argument for /dev/HEX %s", chardev_HEX_msg);
    }


    return bytes;
}
/*
--------------------------------------------------------------------------------------------------
*/





MODULE_LICENSE("GPL");
module_init (init_drivers);
module_exit (stop_drivers);