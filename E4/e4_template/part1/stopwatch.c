#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/fs.h>         // struct file, struct file_operations
#include <linux/init.h>       // for __init, see code
#include <linux/module.h>     // for module init and exit macros
#include <linux/miscdevice.h> // for misc_device_register and struct miscdev
#include <linux/uaccess.h>    // for copy_to_user, see code
#include <asm/io.h>           // for mmap
#include "../include/address_map_arm.h"
#include "../include/interrupt_ID.h"

/*
****************************************************
GLOBAL VARS for virtual memory addresses to hardware
*/
void *virtual_base;                 // used to map physical addresses for the light-weight bridge
volatile int *seven_seg_ptr;
volatile int *seven_seg_ptr_2;
volatile int *timer_ptr;
/*
****************************************************
*/

// Bit patterns to display digits 0 - 9 on HEXs
static const char SEG[] = {
    0b00111111,
    0b00000110,
    0b01011011,
    0b01001111,
    0b01100110,
    0b01101101,
    0b01111101,
    0b00000111,
    0b01111111,
    0b01101111};

// timer0 Counter Start Value
static const int INITIAL_TIMER = 1000000;

// Vars for timer
volatile int stop;
volatile int miliseconds;
volatile int miliseconds_2;
volatile int seconds;
volatile int seconds_2;
volatile int minutes;
volatile int minutes_2;



// Kernel functions for device driver /dev/stopwatch
static int device_open_STOPWATCH(struct inode *, struct file *);
static int device_release_STOPWATCH(struct inode *, struct file *);
static ssize_t device_read_STOPWATCH(struct file *, char *, size_t, loff_t *);

static struct file_operations chardev_STOPWATCH_fops = {
    .owner = THIS_MODULE,
    .read = device_read_STOPWATCH,
    .open = device_open_STOPWATCH,
    .release = device_release_STOPWATCH};

#define DEV_NAME_STOPWATCH "stopwatch"
#define MAX_SIZE 256
#define SUCCESS 0

// character device driver for stopwatch
static struct miscdevice chardev_STOPWATCH = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEV_NAME_STOPWATCH,
    .fops = &chardev_STOPWATCH_fops,
    .mode = 0666};

static int chardev_stopwatch_registered = 0;
static char timer_stopwatch[MAX_SIZE]; // the character array that can be read



// hardware timer module to keep track of time
irq_handler_t timer_handler(int irq, void *dev_id, struct pt_regs *regs)
{

    // clear timer0 TO bit
    *(timer_ptr + 0) = 0b0;

    miliseconds += 1;

    // calculate miliseconds, seconds and minute values
    if (miliseconds == 10)
    {

        miliseconds = 0;
        miliseconds_2 += 1;

        if (miliseconds_2 == 10)
        {

            miliseconds_2 = 0;
            seconds += 1;

            if (seconds == 10)
            {

                seconds = 0;
                seconds_2 += 1;

                if (seconds_2 == 6)
                {

                    seconds_2 = 0;
                    minutes += 1;

                    if (minutes == 10)
                    {

                        minutes = 0;
                        minutes_2 += 1;

                        if (minutes_2 == 6)
                        {
                            minutes_2 = 5;
                            minutes = 9;
                            seconds_2 = 5;
                            seconds = 9;
                            miliseconds_2 = 9;
                            miliseconds = 9;
                        }
                    }
                }
            }
        }
    }
    // display digits onto HEX 7-segs (of current time)
    *seven_seg_ptr = (SEG[9 - miliseconds]) | (SEG[9 - miliseconds_2] << 8) | (SEG[9 - seconds] << 16) | (SEG[5 - seconds_2]) << 24;
    *seven_seg_ptr_2 = (SEG[9 - minutes]) | (SEG[5 - minutes_2] << 8);

    return (irq_handler_t)IRQ_HANDLED;
}

static int __init custom_init(void)
{

    // init stopwatch driver
    int err_stopwatch = misc_register(&chardev_STOPWATCH);
    if (err_stopwatch < 0)
    {
        printk(KERN_ERR "/dev/%s: misc_register() failed\n", DEV_NAME_STOPWATCH);
    }
    else
    {
        printk(KERN_INFO "/dev/%s driver registered\n", DEV_NAME_STOPWATCH);
        chardev_stopwatch_registered = 1;
    }


    int timer_value;
    virtual_base = ioremap(LW_BRIDGE_BASE, LW_BRIDGE_SPAN);

    // Initialize time variables to 0
    stop = 0;
    miliseconds = 0;
    miliseconds_2 = 0;
    seconds = 0;
    seconds_2 = 0;
    minutes = 0;
    minutes_2 = 0;

    // Set virtual addresses for HEXs and timer0
    seven_seg_ptr = virtual_base + HEX3_HEX0_BASE;
    seven_seg_ptr_2 = virtual_base + HEX5_HEX4_BASE;
    timer_ptr = virtual_base + TIMER0_BASE;

    // Initialize timer0
    *(timer_ptr + 2) = INITIAL_TIMER & 0x0000FFFF;
    *(timer_ptr + 3) = (INITIAL_TIMER & 0xFFFF0000) >> 16;
    // write 1s to Control register (START, CONT, ITO = 1) TO START
    *(timer_ptr + 1) = 0x00000007;

    // Set timer HEX display to start at 59:59:99
    *seven_seg_ptr = (SEG[9 - miliseconds]) | (SEG[9 - miliseconds_2] << 8) | (SEG[9 - seconds] << 16) | (SEG[5 - seconds_2]) << 24;
    *seven_seg_ptr_2 = (SEG[9 - minutes]) | (SEG[5 - minutes_2] << 8);

    
    timer_value = request_irq(TIMER0_IRQ, (irq_handler_t)timer_handler,
                              IRQF_SHARED,
                              "timer  handler",
                              (void *)(timer_handler));


    return err_stopwatch;
}

static int __exit custom_exit(void)
{

    if(chardev_stopwatch_registered){
        misc_deregister(&chardev_STOPWATCH);
        printk(KERN_INFO "/dev/%s driver de-registered\n", DEV_NAME_STOPWATCH);
    }

    // Reset HEX displays and timer0
    *seven_seg_ptr = 0;
    *seven_seg_ptr_2 = 0;
    *(timer_ptr + 2) = 0;
    *(timer_ptr + 0) = 0;
    *(timer_ptr + 3) = 0;
    *(timer_ptr + 1) = 0;

    iounmap(virtual_base);
    free_irq(TIMER0_IRQ, (void *)timer_handler);
    
}

/*
--------------------------------------------------------------------------------------------------
Functions for Stopwatch device driver
*/
/* Called when a process opens chardev_STOPWATCH */
static int device_open_STOPWATCH(struct inode *inode, struct file *file)
{
    return SUCCESS;
}

/* Called when a process closes chardev_STOPWATCH */
static int device_release_STOPWATCH(struct inode *inode, struct file *file)
{
    return 0;
}

/* Called when a process reads the time. Provides character data from timer_stopwatch.
 * Returns, and sets *offset to, the number of bytes read. */
static ssize_t device_read_STOPWATCH(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
    // print stopwatch time to terminal
    sprintf(timer_stopwatch, "%d%d:%d%d:%d%d\n", 5 - minutes_2, 9 - minutes, 5 - seconds_2, 9 - seconds, 9 - miliseconds_2, 9 - miliseconds);
    
    size_t bytes;
    bytes = strlen(timer_stopwatch) - (*offset); // how many bytes not yet sent?
    bytes = bytes > length ? length : bytes;     // too much to send all at once?

    if (bytes)
        if (copy_to_user(buffer, &timer_stopwatch[*offset], bytes) != 0)
            printk(KERN_ERR "Error: copy_to_user unsuccessful");
    *offset = bytes; // keep track of number of bytes sent to the user

    return bytes;
}
/*
--------------------------------------------------------------------------------------------------
*/

module_init(custom_init);
module_exit(custom_exit);
