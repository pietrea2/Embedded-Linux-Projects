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

/**  your part 1 kernel code here  **/

// Virtual address pointers to physical memory
void *virtual_base;
volatile int *key_ptr;
volatile int *switch_ptr;
volatile int *seven_seg_ptr;
volatile int *seven_seg_ptr_2;
volatile int *timer_ptr;

/*
****************************************************
GLOBAL VARS for virtual memory addresses to hardware
*/
void *LW_virtual;      // used to map physical addresses for the light-weight bridge
volatile int *KEY_ptr; // virtual address for the KEY port
volatile int *SW_ptr;  // virtual address for the SW port

volatile int KEY_state; // vars for KEY and SW values
volatile int SW_state;
volatile int stop_stopwatch;
/*
****************************************************
*/

// Kernel functions for device driver /dev/KEY
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

// character device driver for KEY
static struct miscdevice chardev_STOPWATCH = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEV_NAME_STOPWATCH,
    .fops = &chardev_STOPWATCH_fops,
    .mode = 0666};

static char timer_stopwatch[MAX_SIZE]; // the character array that can be read
static char command[MAX_SIZE];         // the character array that can be wrote

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

// vars for key handler
volatile int key_pressed;
volatile int switch_value;
volatile int ones_column;
volatile int tens_column;

irq_handler_t key_handler(int irq, void *dev_id, struct pt_regs *regs)
{

    // check which key was pressed and current value on SWs
    key_pressed = *(key_ptr + 3);
    switch_value = *switch_ptr;

    // Check which KEY was pressed, set appropriate timer value
    if (key_pressed == 0x0001)
    {

        if (!stop)
        {
            // stop timer
            *(timer_ptr + 1) = 0x00000008;
        }
        else
        {
            // resume timer
            *(timer_ptr + 1) = 0x00000007;
        }

        stop = !stop;
    }
    else if (key_pressed == 0x0002)
    {
        // set values of miliseconds, seconds, and minutes
        // based on switch_value
        if (switch_value <= 99)
        {

            // split decimal value represented on SWs into 2 digits (ones and tens digit)
            ones_column = switch_value % 10;
            tens_column = switch_value / 10;

            miliseconds = 9 - ones_column;
            miliseconds_2 = 9 - tens_column;

            *seven_seg_ptr = (SEG[9 - miliseconds]) | (SEG[9 - miliseconds_2] << 8) | (SEG[9 - seconds] << 16) | (SEG[5 - seconds_2]) << 24;
        }
    }
    else if (key_pressed == 0x0004)
    {

        if (switch_value <= 59)
        {
            ones_column = switch_value % 10;
            tens_column = switch_value / 10;

            seconds = 9 - ones_column;
            seconds_2 = 5 - tens_column;

            *seven_seg_ptr = (SEG[9 - miliseconds]) | (SEG[9 - miliseconds_2] << 8) | (SEG[9 - seconds] << 16) | (SEG[5 - seconds_2]) << 24;
        }
    }
    else if (key_pressed == 0x0008)
    {

        if (switch_value <= 59)
        {
            ones_column = switch_value % 10;
            tens_column = switch_value / 10;

            minutes = 9 - ones_column;
            minutes_2 = 5 - tens_column;

            *seven_seg_ptr_2 = (SEG[9 - minutes]) | (SEG[5 - minutes_2] << 8);
        }
    }

    // reset KEY Edge bits
    *(key_ptr + 3) = 0xF;

    return (irq_handler_t)IRQ_HANDLED;
}

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
                                minutes_2 = 0;
                                stop_stopwatch = 1;
                                minutes_2 = 0;
                                minutes = 0;
                                seconds_2 = 0;
                                seconds = 0;
                                miliseconds = 0;
                                miliseconds_2 = 0;
                                //  mil
                            }
                        }
                    }
                }
            }
        }
    }
    // display digits onto HEX 7-segs (of current time)
    *seven_seg_ptr = (SEG[miliseconds]) | (SEG[miliseconds_2] << 8) | (SEG[seconds] << 16) | (SEG[seconds_2]) << 24;
    *seven_seg_ptr_2 = (SEG[minutes]) | (SEG[minutes_2] << 8);

    return (irq_handler_t)IRQ_HANDLED;
}

static int __init custom_init(void)
{

    // init KEY driver
    int err_KEY = misc_register(&chardev_STOPWATCH);
    if (err_KEY < 0)
    {
        printk(KERN_ERR "/dev/%s: misc_register() failed\n", DEV_NAME_STOPWATCH);
    }
    else
    {
        printk(KERN_INFO "/dev/%s driver registered\n", DEV_NAME_STOPWATCH);
    }

    /* initialize the message */

    int value, timer_value;
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

    // Set virtual addresses for KEYs, HEXs, SWs and timer0
    key_ptr = virtual_base + KEY_BASE;
    switch_ptr = virtual_base + SW_BASE;
    seven_seg_ptr = virtual_base + HEX3_HEX0_BASE;
    seven_seg_ptr_2 = virtual_base + HEX5_HEX4_BASE;
    timer_ptr = virtual_base + TIMER0_BASE;

    // Initialize timer0 and reset KEYs
    *(timer_ptr + 2) = INITIAL_TIMER & 0x0000FFFF;
    *(timer_ptr + 3) = (INITIAL_TIMER & 0xFFFF0000) >> 16;
    // write 1s to Control register (START, CONT, ITO = 1) TO START
    *(timer_ptr + 1) = 0x00000007;
    // reset KEY Edge bits
    // set KEY interrupt bits
    *(key_ptr + 3) = 0xF;
    *(key_ptr + 2) = 0xF;

    // Set timer HEX display to start at 59:59:99
    *seven_seg_ptr = (SEG[miliseconds]) | (SEG[miliseconds_2] << 8) | (SEG[seconds] << 16) | (SEG[seconds_2]) << 24;
    *seven_seg_ptr_2 = (SEG[minutes]) | (SEG[minutes_2] << 8);

    value = request_irq(KEY_IRQ, (irq_handler_t)key_handler,
                        IRQF_SHARED,
                        "button  handler",
                        (void *)(key_handler));

    timer_value = request_irq(TIMER0_IRQ, (irq_handler_t)timer_handler,
                              IRQF_SHARED,
                              "timer  handler",
                              (void *)(timer_handler));

    return timer_value;
}

static int __exit custom_exit(void)
{

    // Reset HEX displays and timer0
    *seven_seg_ptr = 0;
    *seven_seg_ptr_2 = 0;
    *(timer_ptr + 2) = 0;
    *(timer_ptr + 0) = 0;
    *(timer_ptr + 3) = 0;
    *(timer_ptr + 1) = 0;
    misc_deregister(&chardev_STOPWATCH);
    printk(KERN_INFO "/dev/%s driver de-registered\n", DEV_NAME_STOPWATCH);
    iounmap(virtual_base);
    free_irq(TIMER0_IRQ, (void *)timer_handler);
    free_irq(KEY_IRQ, (void *)key_handler);
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
    //  *seven_seg_ptr = (SEG[9 - miliseconds]) | (SEG[9 - miliseconds_2] << 8) | (SEG[9 - seconds] << 16) | (SEG[5 - seconds_2]) << 24;
    // *seven_seg_ptr_2 = (SEG[9 - minutes]) | (SEG[5 - minutes_2] << 8);
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
/*
--------------------------------------------------------------------------------------------------
*/
static ssize_t device_write_STOPWATCH(struct file *filp, const char *buffer, size_t length, loff_t *offset)
{
    int ledr_value;
    static char str_value_input[9];
    int not_hex = 0;
    int i;
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
    // printk(KERN_INFO "SHOOOPI %s %d\n", command, strncmp(command, "stop", 4));
    // Note: we do NOT update *offset; we just copy the data into command
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

    // /*
    // Takes input as HEXADECIMAL VALUE
    // */
    // sscanf(command, "%X", &ledr_value); // Parse command to get LEDR value to write

    // sscanf(command, "%3s", str_value_input); // Scan command as string to double check
    // int string_length = strlen(str_value_input);

    // // DOUBLE CHECK FOR VALID INPUT: EXPECTS 3 DIGIT HEX INPUT
    // if (strlen(command) == 4 && !not_hex && ledr_value >= 0x000 && ledr_value <= 0x3FF)
    // {
    //     *LEDR_ptr = ledr_value; // Display value on LEDR
    // }
    // else
    // {
    //     printk(KERN_ERR "Bad argument for /dev/stopwatch %s", command);
    // }

    return bytes;
}

module_init(custom_init);
module_exit(custom_exit);
