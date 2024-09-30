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
volatile int *switch_ptr;
volatile int *timer_ptr;

//timer0 Counter Start Value
static const int INITIAL_TIMER = 1000000;

//Vars for timer
volatile int stop;
volatile int miliseconds;
volatile int miliseconds_2;
volatile int seconds;
volatile int seconds_2;
volatile int minutes;
volatile int minutes_2;

//vars for key handler
volatile int key_pressed;
volatile int switch_value;
volatile int ones_column;
volatile int tens_column;
//used to keep track of which digit we are setting
volatile int digit_set;

irq_handler_t key_handler(int irq, void *dev_id, struct pt_regs *regs)
{
    
	//check which key was pressed and current value on SWs
    key_pressed = *(key_ptr + 3);
    switch_value = *switch_ptr;

    //Check which KEY was pressed, set appropriate timer value
	//KEY0
    if( key_pressed == 0x0001 ){

        if (!stop) {
			//stop timer
		    *(timer_ptr + 1) = 0x00000008;
	    }
	    else{
			//resume timer
		    *(timer_ptr + 1) = 0x00000007;
        }
	
        stop = !stop;
		digit_set = 0;

    }
	//KEY1:
    else if( key_pressed == 0x0002 ){
        
		//Pressing KEY1 causes the stopwatch time to be displayed on the Terminal window
		if(!stop){
			printk(KERN_ERR "\e[32m");
			printk("%d%d:%d%d:%d%d\e[37m\n", (5-minutes_2), (9-minutes), (5-seconds_2), (9-seconds), (9-miliseconds_2), (9-miliseconds));
		}
		//when paused, each digit get set by value of SW by pressing KEY1
		else{
			if(digit_set == 0){
				if(switch_value <= 9){
					miliseconds = 9 - switch_value;
					digit_set +=1;
				}
			}
			else if(digit_set == 1){
				if(switch_value <= 9){
					miliseconds_2 = 9 - switch_value;
					digit_set +=1;
				}
			}
			else if(digit_set == 2){
				if(switch_value <= 9){
					seconds = 9 - switch_value;
					digit_set +=1;
				}
			}
			else if(digit_set == 3){
				if(switch_value <= 5){
					seconds_2 = 5 - switch_value;
					digit_set +=1;
				}
			}
			else if(digit_set == 4){
				if(switch_value <= 9){
					minutes = 9 - switch_value;
					digit_set +=1;
				}
			}
			else if(digit_set == 5){
				if(switch_value <= 5){
					minutes_2 = 5 - switch_value;
					digit_set = 0;
				}
			}
		
			printk(KERN_ERR "\e[31m");
			printk("%d%d:%d%d:%d%d\e[37m\n", (5-minutes_2), (9-minutes), (5-seconds_2), (9-seconds), (9-miliseconds_2), (9-miliseconds));
		}
			
	}
	
	//reset KEY Edge bits
    *(key_ptr + 3) = 0xF;

	return (irq_handler_t) IRQ_HANDLED;
}

irq_handler_t timer_handler(int irq, void *dev_id, struct pt_regs *regs)
{

    //clear timer0 TO bit
	*(timer_ptr + 0) = 0b0;

	miliseconds += 1;

	//calculate miliseconds, seconds and minute values
	if (miliseconds == 10) {

		miliseconds = 0;
		miliseconds_2 += 1;

		if (miliseconds_2 == 10) {

			miliseconds_2 = 0;
			seconds += 1;

			if (seconds == 10) {

				seconds = 0;
				seconds_2 += 1;

				if (seconds_2 == 6) {

					seconds_2 = 0;
					minutes += 1;

					if (minutes == 10) {

						minutes = 0;
						minutes_2 += 1;

						if (minutes_2 == 6) minutes_2 = 0;

					}
				}
			}
		}
	}

	return (irq_handler_t) IRQ_HANDLED;
}

static int __init custom_init(void)
{

	int value, timer_value;
	virtual_base = ioremap(LW_BRIDGE_BASE, LW_BRIDGE_SPAN);

	//Initialize time variables to 0
	stop = 0;
    miliseconds = 0;
    miliseconds_2 = 0;
	seconds = 0;
	seconds_2 = 0;
	minutes = 0;
	minutes_2 = 0;
	digit_set = 0;

	//Set virtual addresses for KEYs, HEXs, SWs and timer0
	key_ptr = virtual_base + KEY_BASE;
    switch_ptr = virtual_base + SW_BASE;
	timer_ptr = virtual_base + TIMER0_BASE;

	//Initialize timer0 and reset KEYs
	*(timer_ptr + 2) = INITIAL_TIMER & 0x0000FFFF;
	*(timer_ptr + 3) = (INITIAL_TIMER & 0xFFFF0000) >> 16;
	//write 1s to Control register (START, CONT, ITO = 1) TO START
	*(timer_ptr + 1) = 0x00000007;
	//reset KEY Edge bits
	//set KEY interrupt bits
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
	*(timer_ptr + 2) = 0;
	*(timer_ptr + 0) = 0;
	*(timer_ptr + 3) = 0;
	*(timer_ptr + 1) = 0;

	iounmap(virtual_base);
	free_irq(TIMER0_IRQ, (void *) timer_handler);
	free_irq(KEY_IRQ, (void *) key_handler);

	
}

module_init(custom_init);
module_exit(custom_exit);

