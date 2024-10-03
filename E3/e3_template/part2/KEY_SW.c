#include <linux/fs.h>               // struct file, struct file_operations
#include <linux/init.h>             // for __init, see code
#include <linux/module.h>           // for module init and exit macros
#include <linux/miscdevice.h>       // for misc_device_register and struct miscdev
#include <linux/uaccess.h>          // for copy_to_user, see code
#include <asm/io.h>                 // for mmap
#include "../include/address_map_arm_vm.h"

//GLOBAL VARS for virtual memory addresses to hardware
void *LW_virtual;			 // used to map physical addresses for the light-weight bridge
volatile int *KEY_ptr;       // virtual address for the KEY port
volatile int *SW_ptr;        // virtual address for the SW port
//vars for KEY and SW values
volatile int KEY_state;
volatile int SW_state;





//Kernel functions for device driver /dev/KEY
static int device_open_KEY (struct inode *, struct file *);
static int device_release_KEY (struct inode *, struct file *);
static ssize_t device_read_KEY (struct file *, char *, size_t, loff_t *);

//Kernel functions for device driver /dev/SW
static int device_open_SW (struct inode *, struct file *);
static int device_release_SW (struct inode *, struct file *);
static ssize_t device_read_SW (struct file *, char *, size_t, loff_t *);



static struct file_operations chardev_KEY_fops = {
    .owner = THIS_MODULE,
    .read = device_read_KEY,
    .open = device_open_KEY,
    .release = device_release_KEY
};

static struct file_operations chardev_SW_fops = {
    .owner = THIS_MODULE,
    .read = device_read_SW,
    .open = device_open_SW,
    .release = device_release_SW
};

#define SUCCESS 0
#define DEV_NAME_KEY "chardev_KEY"
#define DEV_NAME_SW "chardev_SW"

static struct miscdevice chardev_KEY = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEV_NAME_KEY,
    .fops = &chardev_KEY_fops,
    .mode = 0666
};

static struct miscdevice chardev_SW = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEV_NAME_SW,
    .fops = &chardev_SW_fops,
    .mode = 0666
};

static int chardev_KEY_registered = 0;
static int chardev_SW_registered = 0;

#define MAX_SIZE 256                    // we assume that no message will be longer than this
static char chardev_KEY_msg[MAX_SIZE];  // the character array that can be read
static char chardev_SW_msg[MAX_SIZE];   // the character array that can be read


//Initialization function for driver functions
static int __init init_drivers(void)
{

    //KEY driver
    int err_KEY = misc_register (&chardev_KEY);
    if (err_KEY < 0) {
        printk (KERN_ERR "/dev/%s: misc_register() failed\n", DEV_NAME_KEY);
    }
    else {
        printk (KERN_INFO "/dev/%s driver registered\n", DEV_NAME_KEY);
        chardev_KEY_registered = 1;
    }
    strcpy (chardev_KEY_msg, "Hello from chardev_KEY\n"); /* initialize the message */


    //SW driver
    int err_SW = misc_register (&chardev_SW);
    if (err_SW < 0) {
        printk (KERN_ERR "/dev/%s: misc_register() failed\n", DEV_NAME_SW);
    }
    else {
        printk (KERN_INFO "/dev/%s driver registered\n", DEV_NAME_SW);
        chardev_SW_registered = 1;
    }
    strcpy (chardev_SW_msg, "Hello from chardev_SW\n"); /* initialize the message */



    //initialize virtual pointers (addresses) to KEY, SW
    LW_virtual = ioremap(LW_BRIDGE_BASE, LW_BRIDGE_SPAN);
	KEY_ptr = LW_virtual + KEY_BASE;
    SW_ptr = LW_virtual + SW_BASE;

    //reset KEY Edge bits
	*(KEY_ptr + 2) = 0xF;

    return err_KEY && err_SW;
}

static void __exit stop_drivers(void)
{
    if (chardev_KEY_registered) {
        misc_deregister (&chardev_KEY);
        printk (KERN_INFO "/dev/%s driver de-registered\n", DEV_NAME_KEY);
    }

    if (chardev_SW_registered) {
        misc_deregister (&chardev_SW);
        printk (KERN_INFO "/dev/%s driver de-registered\n", DEV_NAME_SW);
    }

    iounmap(LW_virtual);

}






/* Called when a process opens chardev_KEY */
static int device_open_KEY(struct inode *inode, struct file *file) {
    return SUCCESS;
}

/* Called when a process closes chardev_KEY */
static int device_release_KEY(struct inode *inode, struct file *file) {
    return 0;
}

/* Called when a process reads from chardev_KEY. Provides character data from chardev_KEY_msg.
 * Returns, and sets *offset to, the number of bytes read. */
static ssize_t device_read_KEY(struct file *filp, char *buffer) {
    
    //check which key was pressed
    KEY_state = *(KEY_ptr + 3);
    
    sprintf(chardev_KEY_msg, KEY_state);




}



/* Called when a process opens chardev_SW */
static int device_open_SW(struct inode *inode, struct file *file) {
    return SUCCESS;
}

/* Called when a process closes chardev_SW */
static int device_release_SW(struct inode *inode, struct file *file) {
    return 0;
}

/* Called when a process reads from chardev_SW. Provides character data from chardev_SW_msg.
 * Returns, and sets *offset to, the number of bytes read. */
static ssize_t device_read_SW(struct file *filp, char *buffer) {
    
    //check current SW value
    SW_state = *SW_ptr;

    sprintf(chardev_SW_msg, SW_state);



}





MODULE_LICENSE("GPL");
module_init (init_drivers);
module_exit (stop_drivers);
