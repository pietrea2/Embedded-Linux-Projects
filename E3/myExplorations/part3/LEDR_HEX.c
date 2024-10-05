#include <linux/fs.h>         // struct file, struct file_operations
#include <linux/init.h>       // for __init, see code
#include <linux/module.h>     // for module init and exit macros
#include <linux/miscdevice.h> // for misc_device_register and struct miscdev
#include <linux/uaccess.h>    // for copy_to_user, see code
#include <asm/io.h>           // for mmap
#include "address_map_arm.h"

void *virtual_base;
volatile int *ledr_ptr;
volatile int *hex_ptr;

volatile int switch_value;
volatile int key_value;

static ssize_t ledr_dev_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t ledr_dev_write(struct file *, const char __user *, size_t, loff_t *);
static int ledr_dev_open(struct inode *, struct file *);
static int ledr_dev_release(struct inode *, struct file *);

static ssize_t hex_dev_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t hex_dev_write(struct file *, const char __user *, size_t, loff_t *);
static int hex_dev_open(struct inode *, struct file *);
static int hex_dev_release(struct inode *, struct file *);


#define READ_BYTES 4
#define WRITE_BYTES 8
#define MAX_SIZE 256
#define SUCCESS 0
#define LEDR_DEV_NAME "LEDR"
#define HEX_DEV_NAME "HEX"



static struct file_operations ledr_dev_ops = {
    .owner = THIS_MODULE,
    .read = ledr_dev_read,
    .write = ledr_dev_write,
    .open = ledr_dev_open,
    .release = ledr_dev_release};

static struct miscdevice ledr_dev_dev =
    {
        .minor = MISC_DYNAMIC_MINOR,
        .name = LEDR_DEV_NAME,
        .fops = &ledr_dev_ops,
        .mode = 0666};

static struct file_operations hex_dev_ops = {
    .owner = THIS_MODULE,
    .read = hex_dev_read,
    .write = hex_dev_write,
    .open = hex_dev_open,
    .release = hex_dev_release};

static struct miscdevice hex_dev_dev =
    {
        .minor = MISC_DYNAMIC_MINOR,
        .name = HEX_DEV_NAME,
        .fops = &hex_dev_ops,
        .mode = 0666};


static int ledr_dev_open(struct inode *node_ptr, struct file *filp)
{
    printk(KERN_INFO "connected device %s succesfully\n", LEDR_DEV_NAME);
    return SUCCESS;
}

static int ledr_dev_release(struct inode *node_ptr, struct file *filp)
{
    printk(KERN_INFO "connected released %s succesfully\n", LEDR_DEV_NAME);
    return SUCCESS;
}

static ssize_t
ledr_dev_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
    
}


static ssize_t
ledr_dev_write(struct file *filp, const char *buffer, size_t length, loff_t *offset)
{
    // char input[lenght];
    size_t bytes;
    bytes = strlen(buffer) - (*offset);
    char input[bytes];
    if (copy_from_user(input, &buffer[*offset], bytes) < 0){
        printk(KERN_ERR "Failed to write onto driver\n");
    }
    (*offset) = bytes;
    printk(KERN_INFO "coppied %s\n", input);
    return bytes;
}


static int hex_dev_open(struct inode *node_ptr, struct file *filp)
{
    printk(KERN_INFO "connected device %s succesfully\n", HEX_DEV_NAME);
    return SUCCESS;
}

static int hex_dev_release(struct inode *node_ptr, struct file *filp)
{
    printk(KERN_INFO "connected released %s succesfully\n", HEX_DEV_NAME);
    return SUCCESS;
}

static ssize_t
hex_dev_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{

}


static ssize_t
hex_dev_write(struct file *filp, const char *buffer, size_t length, loff_t *offset)
{


}

static int __init start_chardev(void)
{
    printk(KERN_INFO "Starting the thing \n");
    int error = misc_register(&ledr_dev_dev);
    // int error2 = misc_register(&hex_dev_dev);
    if (error < 0)
        return 1;

    virtual_base = ioremap(LW_BRIDGE_BASE, LW_BRIDGE_SPAN);
    ledr_ptr = virtual_base + LEDR_BASE;
    hex_ptr = virtual_base + HEX3_HEX0_BASE;
    return 0;
}

static int __exit cleanup_device(void)
{
    misc_deregister(&ledr_dev_dev);
    // misc_deregister(&hex_dev_dev);
    iounmap(virtual_base);
}

module_init(start_chardev);
module_exit(cleanup_device);
