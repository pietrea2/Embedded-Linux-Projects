#include <linux/fs.h>           // struct file, struct file_operations
#include <linux/init.h>         // for __init, see code
#include <linux/module.h>       // for module init and exit macros
#include <linux/miscdevice.h>   // for misc_device_register and struct miscdev
#include <linux/uaccess.h>      // for copy_to_user, see code
#include <linux/string.h>
#include <asm/io.h>             // for mmap
#include "../include/address_map_arm.h"

void *LW_virtual;               // used to access FPGA light-weight bridge
volatile int * pixel_ctrl_ptr;  // virtual address of pixel buffer controller
volatile int * char_ctrl_ptr;   // virtual address of char buffer controller
int pixel_buffer;               // front pixel buffer
int back_pixel_buffer;          // back pixel buffer
int back_char_buffer;
int SDRAM_virtual;              // virtual address of SDRAM
int FPGA_ONCHIP_virtual;        // virtual address of FPGA on chip memory
int CHAR_virtual;               // virtual address of FPGA CHAR memory
int resolution_x, resolution_y; // VGA screen size (pixels)
int char_res_x, char_res_y;     // VGA screen size (characters)

// Vars and prototypes for a char driver
#define SUCCESS 0
#define DEV_NAME_VIDEO "video"
#define ABS(x) (((x) > 0) ? (x) : -(x))

// Function prototypes
static int device_open_VIDEO (struct inode *, struct file *);
static int device_release_VIDEO (struct inode *, struct file *);
static ssize_t device_write_VIDEO (struct file *, const char *, size_t, loff_t *);
static ssize_t device_read_VIDEO (struct file *, char* , size_t , loff_t *);

void get_screen_specs(volatile int * );
void wait_for_vsync(volatile int *);
void clear_screen(void);                 
void plot_pixel(int , int , short int );
void draw_line(int , int , int , int , short int );
void draw_box(int , int , int , int , short int );
void swap_vars(int *, int *);
void get_char_screen_specs(volatile int * );            // new char get screen size
void clear_text(void);                                  // new character clear
void plot_char(int, int, char);                         // new character draw
void draw_text(int, int, char *);                       // new string draw


static struct file_operations chardev_video_fops = {
    .owner = THIS_MODULE,
    .open = device_open_VIDEO,
    .release = device_release_VIDEO,
    .write = device_write_VIDEO,
    .read = device_read_VIDEO
};

// Character device driver for /dev/video
static struct miscdevice chardev_video = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEV_NAME_VIDEO,
    .fops = &chardev_video_fops,
    .mode = 0666
};

#define MAX_SIZE 256                         // we assume that no message will be longer than this
static char chardev_video_msg[MAX_SIZE];     // the character array that can be read
static char chardev_read[MAX_SIZE];          // the character array that would be writen on
static int chardev_video_registered = 0;





/* Code to initialize the video driver */
static int __init start_video(void){
    
    // Initialize the miscdevice data structures
    int err_video = misc_register (&chardev_video);
    if (err_video < 0) {
        printk (KERN_ERR "/dev/%s: misc_register() failed\n", DEV_NAME_VIDEO);
    } else {
        printk (KERN_INFO "/dev/%s driver registered\n", DEV_NAME_VIDEO);
        chardev_video_registered = 1;
    }


    // Generate a virtual address for the FPGA light-weight bridge
    LW_virtual = ioremap_nocache(LW_BRIDGE_BASE, LW_BRIDGE_SPAN);
    if (LW_virtual == 0){
        printk(KERN_ERR "Error: ioremap_nocache returned NULL\n");
    }



    // Create virtual memory access to the pixel buffer controller
    pixel_ctrl_ptr = (unsigned int *) (LW_virtual + PIXEL_BUF_CTRL_BASE);
    get_screen_specs (pixel_ctrl_ptr); // determine X, Y screen size

    // Create virtual memory access to the char buffer controller
    char_ctrl_ptr = (unsigned int *) (LW_virtual + CHAR_BUF_CTRL_BASE);
    get_char_screen_specs (char_ctrl_ptr); // determine X, Y screen size



    // Create virtual memory access to the FPGA_ONCHIP_BASE
    FPGA_ONCHIP_virtual = (int) ioremap_nocache(FPGA_ONCHIP_BASE, FPGA_ONCHIP_SPAN);
    if (FPGA_ONCHIP_virtual == 0){
        printk(KERN_ERR "Error: ioremap_nocache returned NULL\n");
    }

    // Create virtual memory access to the SDRAM_BASE
	SDRAM_virtual = (int) ioremap_nocache(SDRAM_BASE, SDRAM_SPAN);
	if (SDRAM_virtual == 0){
		    printk(KERN_ERR "Error: ioremap_nocache returned NULL\n");
	}

    // Create virtual memory access to the FPGA_CHAR_BASE
	CHAR_virtual = (int) ioremap_nocache(FPGA_CHAR_BASE, FPGA_CHAR_SPAN);
	if (CHAR_virtual == 0){
		    printk(KERN_ERR "Error: ioremap_nocache returned NULL\n");
	}




    pixel_buffer = FPGA_ONCHIP_virtual;         // set front and back pixel buffers (VIRTUAL ADDR)
    back_pixel_buffer = SDRAM_virtual;

    back_char_buffer = CHAR_virtual;            // set back char buffer (VIRTUAL ADDR)


    *(pixel_ctrl_ptr + 0) = FPGA_ONCHIP_BASE;   // set PIXEL Buffer and Backbuffer Registers (PHYSICAL ADDR)
    *(pixel_ctrl_ptr + 1) = SDRAM_BASE;

    *(char_ctrl_ptr + 0) = FPGA_CHAR_BASE;      // set CHAR Buffer and Backbuffer Registers (PHYSICAL ADDR)
    *(char_ctrl_ptr + 1) = FPGA_CHAR_BASE;

    clear_screen();
	wait_for_vsync(pixel_ctrl_ptr);
    clear_screen();
	wait_for_vsync(pixel_ctrl_ptr);

    clear_text();
    
    return 0;
}

// Read Resolution Register
void get_screen_specs(volatile int * pixel_ctrl_ptr){

    resolution_x =   ( *(pixel_ctrl_ptr + 2) ) & 0xFFFF;
    resolution_y = ( ( *(pixel_ctrl_ptr + 2) ) & 0xFFFF0000 ) >> 16;

}

void wait_for_vsync(volatile int * pixel_ctrl_ptr){

    *pixel_ctrl_ptr = 1;                // Write value 1 to Buffer Register

    volatile int status;
    status = *(pixel_ctrl_ptr + 3);

    while ((status & 0x01) != 0) {      // Wait until S of the Status register becomes equal to 0
        status = *(pixel_ctrl_ptr + 3);
    }
    
    // set back buffer virtual memory address mto match the address stored in the Backbuffer Register
	if( *(pixel_ctrl_ptr + 1) == SDRAM_BASE ) back_pixel_buffer = (int) SDRAM_virtual;
	else back_pixel_buffer = (int) FPGA_ONCHIP_virtual;
    
}

// Writes 0x0 (RGB Black) into all pixels of VGA
void clear_screen(){

    size_t x, y;
    for ( x = 0; x <= resolution_x; x++){
        for ( y = 0; y <= resolution_y; y++){
            plot_pixel(x, y, 0x0);
        }
    }
}

void plot_pixel(int x, int y, short int color){
    
    //Address of pixels:
    //base + 0x00 0x100  0 
    //base + ( y )( x ) 0

    *(short int *)( back_pixel_buffer + ( ( (x & 0x1FF) | ( (y & 0xFF) << 9 ) ) << 1 ) ) = color;
}

//Function to swap 2 variable values (in draw_line function)
void swap_vars(int *a, int *b) {

    int temp = *a;
    *a = *b;
    *b = temp;

}

//Bresenham’s line-drawing algorithm
void draw_line(int x0, int x1, int y0, int y1, short int color){

    int deltax, deltay;
    int error;
    int x, y;
    int is_steep;
    int y_step;

    is_steep = ABS(y1 - y0) > ABS(x1 - x0);

    if (is_steep){
        swap_vars(&x0, &y0);
        swap_vars(&x1, &y1);
    }
    if (x0 > x1){
        swap_vars(&x0, &x1);
        swap_vars(&y0, &y1);
    }

    //define vars used for Bresenham's algorithm
    deltax = x1 - x0;
    deltay = ABS(y1 - y0);
    error = -(deltax / 2);
    y = y0;

    //calc if line has positive or negative slope
    if (y0 < y1) y_step = 1;
    else y_step = -1;

    //main for loop for drawing algorithm
    for (x = x0; x <= x1; x++){
        if ((x == x0 && y == y0) || (x == x1 && y == y1)) continue;   // don't draw line char at the vertices
        if (is_steep) plot_pixel(y, x, color);
        else plot_pixel(x, y, color);

        //calc error again
        //if greater than 0, draw pixel with y coordinate updated
        //if not, keep drawing pixels at same y coordinate
        error = error + deltay;
        if (error >= 0){
            y = y + y_step;
            error = error - deltax;
        }
    }
}

void draw_box(int x0, int x1, int y0, int y1, short int color){

    int x_range = x1 - x0;
    int y_range = y1 - y0;

    int x, y;
    for ( x = 0; x <= x_range; x++){
        for ( y = 0; y <= y_range; y++){
            plot_pixel(x0 + x, y0 + y, color);
        }
    }
}

// Read Resolution Register of Character Buffer Controller
void get_char_screen_specs(volatile int * char_ctrl_ptr){

    char_res_x =   ( *(char_ctrl_ptr + 2) ) & 0xFFFF;
    char_res_y = ( ( *(char_ctrl_ptr + 2) ) & 0xFFFF0000 ) >> 16;
}

void clear_text(void){

    size_t x, y;
    for ( x = 0; x <= char_res_x; x++){
        for ( y = 0; y <= char_res_y; y++){
            plot_char(x, y, ' ');
        }
    }
}
void plot_char(int x, int y, char c){
    
    //Address of pixels:
    //base + 0b000000 0b0000000
    //base + ( y )( x )

    *(short int *)( back_char_buffer + ( ( (y & 0x3F) << 7 ) | (x & 0x7F) ) ) = c;
}

void draw_text(int x, int y, char *string){

    //char c;
    int i;
    int length = strlen(string);

    for (i = 0; i < length; i++) {
        plot_char( (x + i), y, string[i] );
    }
}


static void __exit stop_video(void){
    
    clear_screen();
	wait_for_vsync(pixel_ctrl_ptr);
    clear_screen();
	wait_for_vsync(pixel_ctrl_ptr);
    clear_text();

    /* unmap the physical-to-virtual mappings */
    iounmap (LW_virtual);
    iounmap ((void *) SDRAM_virtual);
    iounmap ((void *) FPGA_ONCHIP_virtual);
    iounmap ((void *) CHAR_virtual);

    /* Remove the device from the kernel */
    misc_deregister(&chardev_video);
    if(chardev_video_registered){
        printk(KERN_INFO "/dev/%s driver de-registered\n", DEV_NAME_VIDEO);
    }
}

static int device_open_VIDEO(struct inode *inode, struct file *file){
    return SUCCESS;
}

static int device_release_VIDEO(struct inode *inode, struct file *file){
    return 0;
}

// cat /dev/video command --> return "ccc rrr ccc rrr" number of cols and rows in VGA screen and character buffer controller
static ssize_t device_read_VIDEO(struct file *filp, char* buffer, size_t length, loff_t *offset) {

    // insert res_x and res_y into chardev_red char array (string)
    sprintf(chardev_read, "%d %d %d %d\n", resolution_x, resolution_y, char_res_x, char_res_y);

    size_t bytes;
    bytes = strlen(chardev_read) - (*offset);    // how many bytes not yet sent?
    bytes = bytes > length ? length : bytes;     // too much to send all at once?

    if (bytes)
        if (copy_to_user(buffer, &chardev_read[*offset], bytes) != 0)
            printk(KERN_ERR "Error: copy_to_user unsuccessful");
    *offset = bytes; // keep track of number of bytes sent to the user

    return bytes;
}

// echo clear > /dev/video
// echo pixel 319,239 0xFFFF > /dev/video
// echo line x1,y1 x2,y2 color > /dev/video
// echo sync > /dev/video
// echo box x1,y1 x2,y2 color > /dev/video
// echo erase > /dev/video
// echo text x,y string > /dev/video
static ssize_t device_write_VIDEO(struct file *filp, const char *buffer, size_t length, loff_t *offset) {
    
    size_t bytes;
    bytes = length;

    if (bytes > MAX_SIZE - 1)    // can copy all at once, or not?
        bytes = MAX_SIZE - 1;
    if (copy_from_user (chardev_video_msg, buffer, bytes) != 0)
        printk (KERN_ERR "Error: copy_from_user unsuccessful");
    chardev_video_msg[bytes] = '\0';    // NULL terminate
    // Note: we do NOT update *offset; we just copy the data into chardev_LEDR_msg

    /*
    *   Parse input
    */
    int x,y;
    int x1,y1,x2,y2;
    short int color;
    char text_string[30];
    
    if ( !strncmp(chardev_video_msg, "clear", 5) ) {
        printk(KERN_INFO "Clear Pixel Screen !!!!\n");
        clear_screen();
    }
    else if ( !strncmp(chardev_video_msg, "sync", 4) ) {
        wait_for_vsync(pixel_ctrl_ptr);
    }
    else if ( sscanf(chardev_video_msg, "pixel %d,%d %hX", &x, &y, &color) == 3 ) {
        plot_pixel(x, y, color);
    }
    else if ( sscanf(chardev_video_msg, "line %d,%d %d,%d %hX", &x1, &y1, &x2, &y2, &color) == 5 ){
        printk(KERN_INFO "Draw line\n");
        draw_line(x1,x2,y1,y2,color);
    }
    else if ( sscanf(chardev_video_msg, "box %d,%d %d,%d %hX", &x1, &y1, &x2, &y2, &color) == 5 ){
        printk(KERN_INFO "Draw box\n");
        draw_box(x1,x2,y1,y2,color);
    }
    else if ( !strncmp(chardev_video_msg, "erase", 5) ) {
        printk(KERN_INFO "Clear Character Screen !!!!\n");
        clear_text();
    }
    else if ( sscanf(chardev_video_msg, "text %d,%d %s", &x1, &y1, text_string) == 3 ){
        printk(KERN_INFO "Draw char\n");
        draw_text(x1, y1, text_string);
    }
    else {
        printk(KERN_ERR "Wrong Command\n");
    }

    return bytes;
}

MODULE_LICENSE("GPL");
module_init (start_video);
module_exit (stop_video);