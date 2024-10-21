#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/fs.h>         // struct file, struct file_operations
#include <linux/init.h>       // for __init, see code
#include <linux/module.h>     // for module init and exit macros
#include <linux/miscdevice.h> // for misc_device_register and struct miscdev
#include <linux/uaccess.h>    // for copy_to_user, see code
#include <asm/io.h>           // for mmap
#include <linux/string.h>
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
volatile int stop_stopwatch;
volatile int display_on_hex;


// Kernel functions for device driver /dev/stopwatch
static int device_open_STOPWATCH(struct inode *, struct file *);
static int device_release_STOPWATCH(struct inode *, struct file *);
static ssize_t device_read_STOPWATCH(struct file *, char *, size_t, loff_t *);
static ssize_t device_write_STOPWATCH(struct file *, const char *, size_t, loff_t *);

static struct file_operations chardev_STOPWATCH_fops = {
    .owner = THIS_MODULE,
    .read = device_read_STOPWATCH,
    .write = device_write_STOPWATCH,
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
static char command[MAX_SIZE];         // the character array that can be written to





irq_handler_t timer_handler(int irq, void *dev_id, struct pt_regs *regs)
{

    // clear timer0 TO bit
    *(timer_ptr + 0) = 0b0;

    if (!stop_stopwatch)
    {
        miliseconds -= 1;

        // calculate miliseconds, seconds and minute values
        if (miliseconds == -1)
        {

            miliseconds = 9;
            miliseconds_2 -= 1;

            if (miliseconds_2 == -1)
            {

                miliseconds_2 = 9;
                seconds -= 1;

                if (seconds == -1)
                {

                    seconds = 9;
                    seconds_2 -= 1;

                    if (seconds_2 == -1)
                    {

                        seconds_2 = 5;
                        minutes -= 1;

                        if (minutes == -1)
                        {

                            minutes = 9;
                            minutes_2 -= 1;

                            if (minutes_2 == -1)
                            {
                    
                                stop_stopwatch = 1;
                                minutes_2 = 0;
                                minutes = 0;
                                seconds_2 = 0;
                                seconds = 0;
                                miliseconds_2 = 0;
                                miliseconds = 0;
                            }
                        }
                    }
                }
            }
        }
    }

    // display digits onto HEX 7-segs (of current time)
    if(display_on_hex){
        *seven_seg_ptr = (SEG[miliseconds]) | (SEG[miliseconds_2] << 8) | (SEG[seconds] << 16) | (SEG[seconds_2]) << 24;
        *seven_seg_ptr_2 = (SEG[minutes]) | (SEG[minutes_2] << 8);
    }
    else{
        *seven_seg_ptr = 0;
        *seven_seg_ptr_2 = 0;
    }

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
    miliseconds = 9;
    miliseconds_2 = 9;
    seconds = 9;
    seconds_2 = 5;
    minutes = 9;
    minutes_2 = 5;
    stop_stopwatch = 0;
    display_on_hex = 0;

    // Set virtual addresses for HEXs and timer0
    seven_seg_ptr = virtual_base + HEX3_HEX0_BASE;
    seven_seg_ptr_2 = virtual_base + HEX5_HEX4_BASE;
    timer_ptr = virtual_base + TIMER0_BASE;

    // Initialize timer0
    *(timer_ptr + 2) = INITIAL_TIMER & 0x0000FFFF;
    *(timer_ptr + 3) = (INITIAL_TIMER & 0xFFFF0000) >> 16;
    // write 1s to Control register (START, CONT, ITO = 1) TO START
    *(timer_ptr + 1) = 0x00000007;


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
    sprintf(timer_stopwatch, "%d%d:%d%d:%d%d\n", minutes_2, minutes, seconds_2, seconds, miliseconds_2, miliseconds);
    
    size_t bytes;
    bytes = strlen(timer_stopwatch) - (*offset); // how many bytes not yet sent?
    bytes = bytes > length ? length : bytes;     // too much to send all at once?

    if (bytes)
        if (copy_to_user(buffer, &timer_stopwatch[*offset], bytes) != 0)
            printk(KERN_ERR "Error: copy_to_user unsuccessful");
    *offset = bytes; // keep track of number of bytes sent to the user

    return bytes;
}

static ssize_t device_write_STOPWATCH(struct file *filp, const char *buffer, size_t length, loff_t *offset)
{
    size_t bytes;
    bytes = length;

    int temp_minutes;
    int temp_seconds;
    int temp_mseconds;

    if (bytes > MAX_SIZE - 1) // can copy all at once, or not?
        bytes = MAX_SIZE - 1;
    if (copy_from_user(command, buffer, bytes) != 0)
        printk(KERN_ERR "Error: copy_from_user unsuccessful");
    command[bytes] = '\0'; // NULL terminate
    // Note: we do NOT update *offset; we just copy the data into command

    // check which command has been written to the driver by the user, and take appropriate action
    if (!strncmp(command, "stop", sizeof("stop") - 1))
    {
        printk(KERN_INFO "Got stop command\n");
        stop_stopwatch = 1;
    }
    else if (!strncmp(command, "run", sizeof("run") - 1))
    {
        printk(KERN_INFO "Got run command\n");
        stop_stopwatch = 0;
    }
    else if (!strncmp(command, "disp", sizeof("disp") - 1))
    {
        printk(KERN_INFO "Got disp command\n");
        display_on_hex = 1;
    }
    else if (!strncmp(command, "nodisp", sizeof("nodisp") - 1))
    {
        printk(KERN_INFO "Got nodisp command\n");
        display_on_hex = 0;
    }
    else
    {
        int sscan_output = sscanf(command, "%d:%d:%d", &temp_minutes, &temp_seconds, &temp_mseconds);
        printk(KERN_INFO "%d:%d:%d -----------------> \n", temp_minutes, temp_seconds, temp_mseconds);
        if ((sscan_output < 0) ||
            (temp_minutes > 59 || temp_minutes < 0 ||
             temp_seconds > 59 || temp_seconds < 0 ||
             temp_mseconds > 99 || temp_mseconds < 0))
        {
            printk(KERN_ERR "Bad input for /dev/stopwatch %s %d", command, sscan_output);
        }
        else
        {
            miliseconds_2 = temp_mseconds / 10;
            miliseconds = temp_mseconds % 10;
            seconds_2 = temp_seconds / 10;
            seconds = temp_seconds % 10;
            minutes_2 = temp_minutes / 10;
            minutes = temp_minutes % 10;
        }
    }


    return bytes;
}
/*
--------------------------------------------------------------------------------------------------
*/

module_init(custom_init);
module_exit(custom_exit);
