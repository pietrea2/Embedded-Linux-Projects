#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <asm/io.h>

#include "../include/address_map_arm.h"
#include "../include/ADXL345.h"

// Declare global variables needed to use the accelerometer
volatile unsigned int * I2C0_ptr; // virtual address for I2C communication
volatile unsigned int * SYSMGR_ptr; // virtual address for System Manager communication


/**    implement your part 4 driver here    **/


static int __init start_accel(void) {


    /**  Note: include this code in your __init function  **/
    I2C0_ptr = ioremap_nocache (I2C0_BASE, I2C0_SPAN);
    SYSMGR_ptr = ioremap_nocache (SYSMGR_BASE, SYSMGR_SPAN);

    if ((I2C0_ptr == NULL) || (SYSMGR_ptr == NULL))
        printk (KERN_ERR "Error: ioremap_nocache returned NULL!\n");

    pass_addrs((unsigned int*) SYSMGR_ptr, (unsigned int*) I2C0_ptr);

}

static void __exit stop_accel(void) {

    /**  Note: include this code in your __exit function  **/
    iounmap (I2C0_ptr);
    iounmap (SYSMGR_ptr);

}
