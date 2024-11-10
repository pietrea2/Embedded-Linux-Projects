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
int * pixel_buffer;   // used for virtual address of pixel buffer
int resolution_x, resolution_y; // VGA screen size 

// Vars and prototypes for a char driver
#define SUCCESS 0
#define DEV_NAME_VIDEO "video"

static int device_open (struct inode *, struct file *);
static int device_release (struct inode *, struct file *);
static ssize_t device_write (struct file *, const char *, size_t, loff_t *);
static ssize_t device_read(struct file *, char* , size_t , loff_t *);
void get_screen_specs(volatile int * );
void clear_screen(void);
void plot_pixel(int , int , short int );

static struct file_operations chardev_video_fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_release,
    .write = device_write,
    .read = device_read
};

//character device driver for LEDR
static struct miscdevice chardev_video = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEV_NAME_VIDEO,
    .fops = &chardev_video_fops,
    .mode = 0666
};

#define MAX_SIZE 256                     // we assume that no message will be longer than this
static char chardev_video_msg[MAX_SIZE];  // the character array that can be read
static char chardev_read[MAX_SIZE];     // the character array that would be writen on

/* Code to initialize the video driver */
static int __init start_video(void){
    // initialize the miscdevice data structures
    /* TODO */
    int err_video = misc_register (&chardev_video);
    if (err_video < 0) {
        printk (KERN_ERR "/dev/%s: misc_register() failed\n", DEV_NAME_VIDEO);
    } else {
        printk (KERN_INFO "/dev/%s driver registered\n", DEV_NAME_VIDEO);
        // chardev_LEDR_registered = 1;
    }


    // generate a virtual address for the FPGA light-weight bridge
    LW_virtual = ioremap_nocache(LW_BRIDGE_BASE, LW_BRIDGE_SPAN);
    if (LW_virtual == 0)
        printk(KERN_ERR "Error: ioremap_nocache returned NULL\n");

    // Create virtual memory access to the pixel buffer controller
    pixel_ctrl_ptr = (unsigned int *) (LW_virtual + PIXEL_BUF_CTRL_BASE);
    get_screen_specs (pixel_ctrl_ptr); // determine X, Y screen size

    // Create virtual memory access to the pixel buffer
    pixel_buffer = (int *)ioremap_nocache(FPGA_ONCHIP_BASE, FPGA_ONCHIP_SPAN);
    if (pixel_buffer == 0)
        printk(KERN_ERR "Error: ioremap_nocache returned NULL\n");
    printk(KERN_INFO "%p -a-a-a--a \n", pixel_buffer);
    
    /* Erase the pixel buffer */
    clear_screen();
    return 0;
}

void get_screen_specs(volatile int * pixel_ctrl_ptr){
    /* TODO */
    printk(KERN_INFO "----------> %X\n", *(pixel_ctrl_ptr + 2));
    resolution_x = (*(pixel_ctrl_ptr + 2)) & 0xFFFF;
    resolution_y = ((*(pixel_ctrl_ptr + 2)) & 0xFFFF0000) >> 16;
    printk(KERN_INFO " : %x -- : %x\n",*(pixel_ctrl_ptr), *(pixel_ctrl_ptr + 1));
}

void clear_screen(){
    /* TODO */
}

void plot_pixel(int x, int y, short int color){
    /* TODO */
    // *(pixel_buffer +  (((x) | ((y << 9))) << 1 )/4) = color;
    // printk(KERN_INFO "%X , %X , %X , %X, %X, %X\n", x, y, (x & 0x1FF) , (y & 0xFF), ((y & 0xFF) << 9), ((x & 0x1FF) | ((y & 0xFF) << 9)));
    // printk(KERN_INFO "%X , %X , %X\n", pixel_buffer ,  (((x & 0x1FF) | ((y & 0xFF) << 9)) << 1 )/4,(int *)((void *)pixel_buffer + (((x & 0x1FF) | ((y& 0xFF) << 9)) << 1 )));
    // *pixel_buffer = color;
    // *(int *)((void *)pixel_buffer + (((x & 0x1FF) | ((y& 0xFF) << 9)) << 1 )) = color;
}

static void __exit stop_video(void){
    /* unmap the physical-to-virtual mappings */
    iounmap (LW_virtual);
    iounmap ((void *) pixel_buffer);

    /* Remove the device from the kernel */
    /* TODO */
    misc_deregister(&chardev_video);
    printk(KERN_INFO "/dev/%s driver de-registered\n", DEV_NAME_VIDEO);
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
    sprintf(chardev_read, "%d %d\n",resolution_x, resolution_y);
    size_t bytes;
    bytes = strlen(chardev_read) - (*offset); // how many bytes not yet sent?
    bytes = bytes > length ? length : bytes;     // too much to send all at once?

    if (bytes)
        if (copy_to_user(buffer, &chardev_read[*offset], bytes) != 0)
            printk(KERN_ERR "Error: copy_to_user unsuccessful");
    *offset = bytes; // keep track of number of bytes sent to the user

    return bytes;
}

static ssize_t device_write(struct file *filp, const char 
    *buffer, size_t length, loff_t *offset) {
    // static char str_value_input[7];
    size_t bytes;
    bytes = length;
    int x,y;
    short int color;
    if (bytes > MAX_SIZE - 1)    // can copy all at once, or not?
        bytes = MAX_SIZE - 1;
    if (copy_from_user (chardev_video_msg, buffer, bytes) != 0)
        printk (KERN_ERR "Error: copy_from_user unsuccessful");
    chardev_video_msg[bytes] = '\0';    // NULL terminate
    // Note: we do NOT update *offset; we just copy the data into chardev_LEDR_msg

    /*
    *   Parse input
    */
    if (!strncmp(chardev_video_msg, "clear", 5)){
        printk(KERN_INFO "Clear Screen !!!!\n");
        /* TODO */
    } else if (sscanf(chardev_video_msg, "pixel %d,%d %X", &x, &y, &color ) == 3) {
        // printk(KERN_INFO "Color Pixel : %d,%d ---> %X\n", x, y , color);
        *(int *)((void *)pixel_buffer + (((x & 0x1FF) | ((y& 0xFF) << 9)) << 1 )) = color;
        /* TODO */
    } else {
        printk(KERN_ERR "Wrong Command\n");
    }                 
    // int string_length = strlen(str_value_input);


    //DOUBLE CHECK FOR VALID INPUT: EXPECTS 3 DIGIT HEX INPUT
    // if( strlen(chardev_LEDR_msg) == 4 && !not_hex && ledr_value >= 0x000 && ledr_value <= 0x3FF ){
    //     *LEDR_ptr = ledr_value;                       //Display value on LEDR
    // }
    // else{
    //     printk (KERN_ERR "Bad argument for /dev/LEDR %s", chardev_LEDR_msg);
    // }

    return bytes;
}

MODULE_LICENSE("GPL");
module_init (start_video);
module_exit (stop_video);