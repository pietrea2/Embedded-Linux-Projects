#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include "../include/address_map_arm.h"
#include "../include/interrupt_ID.h"


//Virtual address pointers to physical memory
void *virtual_base;
volatile int *key_ptr;
volatile int *timer_ptr;

//timer0 Counter Start Value
//Clock frequency is 100 MHz
//timer0 counts down to 0 from INITIAL_TIMER
//reaching 0 represents 10 ms
static const int INITIAL_TIMER = 1000000;

volatile int miliseconds;
volatile int miliseconds_2;
volatile int seconds;
volatile int seconds_2;
volatile int minutes;
volatile int minutes_2;


irq_handler_t key_handler(int irq, void *dev_id, struct pt_regs *regs)
{
    //reset KEY Edge bits to 0 by writing value 1 to every bit-position
	*(key_ptr + 3) = 0xF;
	
	//Print current time to terminal
	//\e[37m   = set color to white (foreground)  
	//\e[31m   = set color to red (foreground)  
	printk(KERN_ERR "\e[31m");
	printk("%d%d:%d%d:%d%d\e[37m\n", minutes_2, minutes, seconds_2, seconds, miliseconds_2, miliseconds);

	return (irq_handler_t) IRQ_HANDLED;
}

irq_handler_t timer_handler(int irq, void *dev_id, struct pt_regs *regs)
{
    //Clear TO bit by writing 1 to it
	*(timer_ptr + 0) = 0x0;

	//increment milisecond var after every timer0 interrupt
	miliseconds += 1;

    //Setting variables that keep track of MM:SS:DD
    //Time wraps around when it reaches 59:59:99
    //Seperate var used for each digit
    //Ex: D,D = miliseconds_2, miliseconds

    //Reset after reaching MM:SS:D9
	if (miliseconds == 10)
	{
		miliseconds = 0;
		miliseconds_2 += 1;

        //Reset after reaching MM:SS:9D
		if (miliseconds_2 == 10)
		{
			miliseconds_2 = 0;
			seconds += 1;

            //Reset after reaching MM:S9:DD
			if (seconds == 10)
			{
				seconds = 0;
				seconds_2 += 1;

                //Reset after reaching MM:5S:DD
				if (seconds_2 == 6)
				{
					seconds_2 = 0;
					minutes += 1;

                    //Reset after reaching M9:SS:DD
					if (minutes == 10)
					{
						minutes = 0;
						minutes_2 += 1;

                        //Reset after reaching 5M:SS:DD
						if (minutes_2 == 6)
							minutes_2 = 0;
					}
				}
			}
		}
	}


	return (irq_handler_t) IRQ_HANDLED;
}

static int __init custom_init(void)
{

	int value;
    int timer_value;
    //gen virtual address for FPGA lightweight bridge
	virtual_base = ioremap(LW_BRIDGE_BASE, LW_BRIDGE_SPAN);
	
    //Set timer vars
    miliseconds = 0;
    miliseconds_2 = 0;
	seconds = 0;
	seconds_2 = 0;
	minutes = 0;
	minutes_2 = 0;

    //Set virtual addresses for KEYs, HEXs and timer0
    key_ptr = virtual_base + KEY_BASE;	
	timer_ptr = virtual_base + TIMER0_BASE;

    //Initialize timer0 and reset KEYs
	*(timer_ptr + 2) = INITIAL_TIMER & 0x0000FFFF;
	*(timer_ptr + 3) = (INITIAL_TIMER & 0xFFFF0000) >> 16;
    //write 1s to Control register (START, CONT, ITO = 1) TO START
	*(timer_ptr + 1) = 0x00000007;
	*(key_ptr + 3) = 0xF;
	*(key_ptr + 2) = 0xF;


	value = request_irq(KEY_IRQ, (irq_handler_t) key_handler,
						IRQF_SHARED,
						"button  handler",
						(void *)(key_handler));

	timer_value = request_irq(TIMER0_IRQ, (irq_handler_t) timer_handler,
							  IRQF_SHARED,
							  "timer  handler",
							  (void *)(timer_handler));

	return timer_value;
}

static int __exit custom_exit(void)
{
    //Reset HEX displays and timer0
	*(timer_ptr + 0) = 0;
	*(timer_ptr + 1) = 0;
	*(timer_ptr + 2) = 0;
	*(timer_ptr + 3) = 0;
    
	iounmap(virtual_base);
	free_irq(TIMER0_IRQ, (void *) timer_handler);
	free_irq(KEY_IRQ, (void *) key_handler);

}

module_init(custom_init);
module_exit(custom_exit);
