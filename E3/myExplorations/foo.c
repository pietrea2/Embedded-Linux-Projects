#include <linux/fs.h>         // struct file, struct file_operations
#include <linux/init.h>       // for __init, see code
#include <linux/module.h>     // for module init and exit macros
#include <linux/miscdevice.h> // for misc_device_register and struct miscdev
#include <linux/uaccess.h>    // for copy_to_user, see code
#include <asm/io.h>           // for mmap
#include "address_map_arm.h"

volatile int *virtual_base;
volatile int *key_ptr;


static ssize_t dev_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char __user *, size_t, loff_t *);
static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);

#define READ_BYTES 4
#define WRITE_BYTES 8
#define MAX_SIZE 256
#define SUCCESS 0
#define DEV_NAME "My device"

static char chardev_msg[MAX_SIZE];

static struct file_operations dev_ops = {
    .owner = THIS_MODULE,
    .read = dev_read,
    .write = dev_write,
    .open = dev_open,
    .release = dev_release};

static struct miscdevice dev_dev =
    {
        .minor = MISC_DYNAMIC_MINOR,
        .name = "My Dev",
        .fops = &dev_ops,
        .mode = 0666};

static int dev_open(struct inode *node_ptr, struct file *filp)
{
    printk(KERN_INFO "connected device %s succesfully\n", DEV_NAME);
    return SUCCESS;
}

static int dev_release(struct inode *node_ptr, struct file *filp)
{
    printk(KERN_INFO "connected released %s succesfully\n", DEV_NAME);
    return SUCCESS;
}

static ssize_t
dev_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
    char message_buffer[READ_BYTES + 1];
    size_t bytes;
    bytes = READ_BYTES - (*offset);
    printk(KERN_ERR "dev mesg : %s\n",chardev_msg);
    if (bytes)
    {
        if (*(key_ptr + 3))
            sprintf(message_buffer, chardev_msg);
        else
            sprintf(message_buffer, "nar\n");
        copy_to_user(buffer, message_buffer, bytes);
    }
    (*offset) = bytes;
    return bytes;
}

static ssize_t
dev_write(struct file *filp, const char *buffer, size_t length, loff_t *offset)
{
    size_t bytes;
    bytes = length;
    if (bytes > WRITE_BYTES)
        bytes = WRITE_BYTES - 1;
    printk(KERN_INFO "what is the byte : %d -- buffer ::: %s\n",bytes, buffer);
    if((copy_from_user(chardev_msg, buffer, bytes)) != 0)
        printk(KERN_INFO "Shietye");
    chardev_msg[bytes] = '\0';
    printk(KERN_INFO "char dev message things -------------- %s",chardev_msg);
    return bytes;
}

static int __init start_chardev(void)
{
    printk(KERN_INFO "Starting the thing \n");
    int error = misc_register(&dev_dev);
    if (error < 0)
        return 1;

    virtual_base = ioremap(LW_BRIDGE_BASE, LW_BRIDGE_SPAN);
    key_ptr = virtual_base + KEY_BASE;
    return 0;
}

static int __exit cleanup_device(void)
{
    misc_deregister(&dev_dev);
    iounmap(virtual_base);
}

module_init(start_chardev);
module_exit(cleanup_device);
