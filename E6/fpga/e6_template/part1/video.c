#include <linux/fs.h> // struct file, struct file_operations
#include <linux/init.h> // for __init, see code
#include <linux/module.h> // for module init and exit macros
#include <linux/miscdevice.h> // for misc_device_register and struct miscdev
#include <linux/uaccess.h> // for copy_to_user, see code
#include <linux/string.h>
#include <asm/io.h> // for mmap
#include "../include/address_map_arm.h"


/**  your part 1 kernel code here  **/

void *LW_virtual;   // used to access FPGA light-weight bridge
volatile int * pixel_ctrl_ptr; // virtual address of pixel buffer controller
int pixel_buffer;   // used for virtual address of pixel buffer
int resolution_x, resolution_y; // VGA screen size 

// Vars and prototypes for a char driver


/* Code to initialize the video driver */
static int __init start_video(void){
    // initialize the miscdevice data structures
    /* TODO */

    // generate a virtual address for the FPGA light-weight bridge
    LW_virtual = ioremap_nocache(LW_BRIDGE_BASE, LW_BRIDGE_SPAN);
    if (LW_virtual == 0)
        printk(KERN_ERR "Error: ioremap_nocache returned NULL\n");

    // Create virtual memory access to the pixel buffer controller
    pixel_ctrl_ptr = (unsigend int *) (LW_virtual + PIXEL_BUF_CTRL_BASE);
    get_screen_specs (pixel_ctrl_ptr); // determine X, Y screen size

    // Create virtual memory access to the pixel buffer
    pixel_buffer = (int) ioremap_nocache(FPGA_ONCHIP_BASE, FPGA_ONCHIP_SPAN);
    if (pixel_buffer == 0)
        printk(KERN_ERR "Error: ioremap_nocache returned NULL\n");
    
    /* Erase the pixel buffer */
    clear_screen();
    return 0;
}

void get_screen_specs(volatile int * pixel_ctrl_ptr){
    /* TODO */
}

void clear_screen(){
    /* TODO */
}

void plot_pixel(int x, int y, short int color){
    /* TODO */
}

static void __exit stop_video(void){
    /* unmap the physical-to-virtual mappings */
    iounmap (LW_virtual);
    iounmap ((void *) pixel_buffer);

    /* Remove the device from the kernel */
    /* TODO */
}

static int device_open(struct inode *inode, struct file *file){
    return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file){
    return 0;
}

static ssize_t device_read(struct file *filp, char* buffer,
    size_t length, loff_t *offset) {
        /* TODO */
}

static ssize_t device_write(struct file *filp, const char 
    *buffer, size_t length, loff_t *offset) {
        /* TODO */
}

MODULE_LICENSE("GPL");
module_init (start_video);
module_exit (stop_video);