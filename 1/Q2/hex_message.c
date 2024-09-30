#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/thread_info.h>
#include <linux/delay.h>
#include <asm/io.h>
#include "fpga_addresses.h"
#include "irq_addresses.h"

void *virtual_base;
volatile int *key_ptr, *led_ptr,
	*seven_seg_ptr, *timer_ptr;

volatile int state;
volatile int left;

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

static const int INITIAL_TIMER = 100000000;

volatile int stop;

irq_handler_t key_handler(int irq, void *dev_id)
{

	

	*(key_ptr + 3) = 0xF;
	printk(KERN_INFO "EXITED COOKIE\n");
	if (!stop)
		*(timer_ptr + 1) = 0x00000000;
	else 
		*(timer_ptr + 1) = 0x00000007;
	stop = !stop;
	return (irq_handler_t)IRQ_HANDLED;
}

irq_handler_t timer_handler(int irq, void *dev_id)
{

	*(timer_ptr + 0) = 0x0;
	printk(KERN_INFO "GOOMBOOLOO\n");
	return (irq_handler_t)IRQ_HANDLED;
}

static int __init custom_init(void)
{
	int value, timer_value;
	virtual_base = ioremap(BRIDGE_OFFSET, STANDARD_SIZE);
	stop = 0;
	led_ptr = virtual_base + LEDR_OFFSET;
	key_ptr = virtual_base + DATA_REGISTER_OFFSET;
	seven_seg_ptr = virtual_base + SEVEN_SEG1_OFFSET;
	timer_ptr = virtual_base + TIMER_REGISTER_OFFSET;

	*(timer_ptr + 2) = INITIAL_TIMER & 0x0000FFFF;
	*(timer_ptr + 3) = (INITIAL_TIMER & 0xFFFF0000) >> 16;
	*(timer_ptr + 1) = 0x00000007;
	*(key_ptr + 3) = 0xF;
	*(key_ptr + 2) = 0xF;
	state = 0;
	left = 1;
	int i = 0;
	printk(KERN_INFO "wooned\n\n");
	value = request_irq(KEYS_IRQ, (irq_handler_t)key_handler,
						IRQF_SHARED,
						"button  handler",
						(void *)(key_handler));

	timer_value = request_irq(TIMER_IRQ, (irq_handler_t)timer_handler,
							  IRQF_SHARED,
							  "timer  handler",
							  (void *)(timer_handler));

	return timer_value;
}

static int __exit custom_exit(void)
{
	*led_ptr = 0;
	*seven_seg_ptr = 0;
	*(timer_ptr + 2) = 0;
	*(timer_ptr + 0) = 0;
	*(timer_ptr + 3) = 0;
	*(timer_ptr + 1) = 0;
	iounmap(virtual_base);
	free_irq(TIMER_IRQ, (void *)timer_handler);
	free_irq(KEYS_IRQ, (void *)timer_handler);
	printk(KERN_INFO "Ending interrupt handler module");
}

module_init(custom_init);
module_exit(custom_exit);
