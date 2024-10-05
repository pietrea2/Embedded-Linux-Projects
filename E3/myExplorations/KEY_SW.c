#include <linux/fs.h>         // struct file, struct file_operations
#include <linux/init.h>       // for __init, see code
#include <linux/module.h>     // for module init and exit macros
#include <linux/miscdevice.h> // for misc_device_register and struct miscdev
#include <linux/uaccess.h>    // for copy_to_user, see code
#include <asm/io.h>           // for mmap
#include "address_map_arm.h"

void *virtual_base;
volatile int *key_ptr;
volatile int *sw_ptr;

volatile int switch_value;
volatile int key_value;

static ssize_t sw_dev_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t sw_dev_write(struct file *, const char __user *, size_t, loff_t *);
static int sw_dev_open(struct inode *, struct file *);
static int sw_dev_release(struct inode *, struct file *);

static ssize_t key_dev_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t key_dev_write(struct file *, const char __user *, size_t, loff_t *);
static int key_dev_open(struct inode *, struct file *);
static int key_dev_release(struct inode *, struct file *);


#define READ_BYTES 4
#define WRITE_BYTES 8
#define MAX_SIZE 256
#define SUCCESS 0
#define SW_DEV_NAME "SW"
#define KEY_DEV_NAME "KEY"

static char charsw_dev_msg[MAX_SIZE];

static struct file_operations sw_dev_ops = {
    .owner = THIS_MODULE,
    .read = sw_dev_read,
    .write = sw_dev_write,
    .open = sw_dev_open,
    .release = sw_dev_release};

static struct miscdevice sw_dev_dev =
    {
        .minor = MISC_DYNAMIC_MINOR,
        .name = SW_DEV_NAME,
        .fops = &sw_dev_ops,
        .mode = 0666};

static struct file_operations key_dev_ops = {
    .owner = THIS_MODULE,
    .read = key_dev_read,
    .write = key_dev_write,
    .open = key_dev_open,
    .release = key_dev_release};

static struct miscdevice key_dev_dev =
    {
        .minor = MISC_DYNAMIC_MINOR,
        .name = KEY_DEV_NAME,
        .fops = &key_dev_ops,
        .mode = 0666};


static int sw_dev_open(struct inode *node_ptr, struct file *filp)
{
    printk(KERN_INFO "connected device %s succesfully\n", SW_DEV_NAME);
    return SUCCESS;
}

static int sw_dev_release(struct inode *node_ptr, struct file *filp)
{
    printk(KERN_INFO "connected released %s succesfully\n", SW_DEV_NAME);
    return SUCCESS;
}

static ssize_t
sw_dev_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
    size_t bytes;
    char hex_string[MAX_SIZE];
    switch_value = *sw_ptr;
    sprintf(hex_string, "%X\n", switch_value);
    printk(KERN_ERR "hex_string : %s\n",hex_string);
    bytes = strlen(hex_string) - (*offset);
    printk(KERN_ERR "bytes : %d\n",bytes);
    bytes = bytes > length ? length : bytes;
    if (bytes)
    {
        copy_to_user(buffer, &hex_string[*offset], bytes);
    }
    *offset = bytes;
    return bytes;
}


static ssize_t
sw_dev_write(struct file *filp, const char *buffer, size_t length, loff_t *offset)
{

    // size_t byte;
    // size_t max_write;
    // byte = 0;
    // max_write = MAX_SIZE - 1 > length ? length : MAX_SIZE - 1;
    // while (buffer[*offset] && max_write)
    // {
    //     charsw_dev_msg[byte] = buffer[*offset];
    //     byte ++;
    //     (*offset) ++;
    //     max_write --;
    // }
    // charsw_dev_msg[byte] = '\0';
    // (*offset)= byte;
    // return byte;
}


static int key_dev_open(struct inode *node_ptr, struct file *filp)
{
    printk(KERN_INFO "connected device %s succesfully\n", KEY_DEV_NAME);
    return SUCCESS;
}

static int key_dev_release(struct inode *node_ptr, struct file *filp)
{
    printk(KERN_INFO "connected released %s succesfully\n", KEY_DEV_NAME);
    return SUCCESS;
}

static ssize_t
key_dev_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
    size_t bytes;
    char hex_string[MAX_SIZE];
    key_value = *key_ptr;
    sprintf(hex_string, "%X\n", switch_value);
    printk(KERN_ERR "hex_string : %s\n",hex_string);
    bytes = strlen(hex_string) - (*offset);
    printk(KERN_ERR "bytes : %d\n",bytes);
    bytes = bytes > length ? length : bytes;
    if (bytes)
    {
        copy_to_user(buffer, &hex_string[*offset], bytes);
    }
    *offset = bytes;
    return bytes;
}


static ssize_t
key_dev_write(struct file *filp, const char *buffer, size_t length, loff_t *offset)
{

    // size_t byte;
    // size_t max_write;
    // byte = 0;
    // max_write = MAX_SIZE - 1 > length ? length : MAX_SIZE - 1;
    // while (buffer[*offset] && max_write)
    // {
    //     charsw_dev_msg[byte] = buffer[*offset];
    //     byte ++;
    //     (*offset) ++;
    //     max_write --;
    // }
    // charsw_dev_msg[byte] = '\0';
    // (*offset)= byte;
    // return byte;
}

static int __init start_chardev(void)
{
    printk(KERN_INFO "Starting the thing \n");
    int error = misc_register(&sw_dev_dev);
    int error2 = misc_register(&key_dev_dev);
    if (error < 0 || error2 < 0)
        return 1;

    virtual_base = ioremap(LW_BRIDGE_BASE, LW_BRIDGE_SPAN);
    key_ptr = virtual_base + KEY_BASE;
    sw_ptr = virtual_base + SW_BASE;
    return 0;
}

static int __exit cleanup_device(void)
{
    misc_deregister(&sw_dev_dev);
    misc_deregister(&key_dev_dev);
    iounmap(virtual_base);
}

module_init(start_chardev);
module_exit(cleanup_device);
