#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include "../include/address_map_arm.h"
#include "../include/interrupt_ID.h"

//Virtual address pointers to physical memory
void *virtual_base;
volatile int *ledr_ptr;
volatile int *switch_ptr;
volatile int *seven_seg_ptr;
volatile int *gpio_ptr;
volatile int *timer_ptr;

//Bit patterns to display digits 0 - 9 on HEXs
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

// Timer0 Counter Start Value
// Freq of Timer0 = 100 MHz
// Count to 10 000 000 to gen 10 Hz frequency
static const int INITIAL_TIMER = 10000000;

volatile int switch_value;
volatile int SW_9_6_value;
volatile int cur_SW_9_6_value;
volatile int timer_count_val;
volatile int ones;
volatile int tens;
volatile int first;

void update_frequency(void) {
    
    switch_value = *switch_ptr;                                     // Read SWs
    cur_SW_9_6_value = switch_value >> 6;
    *ledr_ptr = switch_value;                                       // Display SW value on LEDRs

    if( (cur_SW_9_6_value != SW_9_6_value) || first ){

        timer_count_val = (INITIAL_TIMER / (cur_SW_9_6_value + 1)) / 2;   // Calc new count value for timer

        *(timer_ptr + 1) = 0x00000008;                              // Stop timer
        *(timer_ptr + 2) = timer_count_val & 0x0000FFFF;            // Load new count value
	    *(timer_ptr + 3) = (timer_count_val & 0xFFFF0000) >> 16;
        *(timer_ptr + 1) = 0x00000007;                              // Resume timer

        SW_9_6_value = cur_SW_9_6_value;
        tens = (SW_9_6_value + 1) / 10;
        ones = (SW_9_6_value + 1) - tens * 10;
        *seven_seg_ptr = (SEG[0]) | (SEG[ones] << 8) | (SEG[tens] << 16) | (SEG[0]) << 24;

        first = 0;
    }

}

irq_handler_t timer_handler(int irq, void *dev_id, struct pt_regs *regs) {

    *gpio_ptr = (*gpio_ptr) ^ 1;    // Toggle bit
	*(timer_ptr + 0) = 0b0;         // Clear timer0 TO bit

    update_frequency();

	return (irq_handler_t) IRQ_HANDLED;
}

static int __init custom_init(void) {

	int timer_value;
    first = 1;

	virtual_base = ioremap(LW_BRIDGE_BASE, LW_BRIDGE_SPAN);

	//Set virtual addresses for LEDRs, HEXs, SWs and timer0
	ledr_ptr = virtual_base + LEDR_BASE;
    switch_ptr = virtual_base + SW_BASE;
	seven_seg_ptr = virtual_base + HEX3_HEX0_BASE;
	gpio_ptr = virtual_base + GPIO0_BASE;
	timer_ptr = virtual_base + TIMER0_BASE;

    *gpio_ptr = 1;                                              // Set data bit 
    *(gpio_ptr + 1) = 0xFFFFFFFF;                               // Set all data bits to be outputs

    update_frequency();

	*(timer_ptr + 2) = timer_count_val & 0x0000FFFF;            // Initialize timer0
	*(timer_ptr + 3) = (timer_count_val & 0xFFFF0000) >> 16;
	*(timer_ptr + 1) = 0x00000007;                              // Write 1s to Control register (START, CONT, ITO = 1) TO START

	timer_value = request_irq(TIMER0_IRQ, (irq_handler_t) timer_handler, IRQF_SHARED, "timer  handler", (void *)(timer_handler));
	return timer_value;
}

static int __exit custom_exit(void) {

	//Reset HEX, LEDR and timer0
    *ledr_ptr = 0;
	*seven_seg_ptr = 0;
	*(timer_ptr + 2) = 0;
	*(timer_ptr + 0) = 0;
	*(timer_ptr + 3) = 0;
	*(timer_ptr + 1) = 0;

	iounmap(virtual_base);
	free_irq(TIMER0_IRQ, (void *) timer_handler);
}

module_init(custom_init);
module_exit(custom_exit);