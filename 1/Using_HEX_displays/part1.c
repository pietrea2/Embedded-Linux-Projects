#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include "address_map_arm.h"
#include "physical.h"
#include "physical.c"

// 7-seg bit patterns for scrolling message: [Intel SoC FPGA     Intel]
char message[25] = {0b00000100, 0b01010100, 0b01111000, 0b01111001, 0b00111000, 
                    0b00000000,
                    0b01101101, 0b01011100, 0b00111001, 
                    0b00000000,
                    0b01110001, 0b01110011, 0b01111101, 0b01110111, 
                    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
                    0b00000100, 0b01010100, 0b01111000, 0b01111001, 0b00111000};

char * message_pointer = &message[0];

// used to exit the program cleanly
volatile sig_atomic_t stop;
void catchSIGINT(int);

int main(void)
{
    volatile int *KEY_ptr;        // virtual address for the KEY port
    volatile int *HEX3_HEX0_ptr;  // virtual pointer to HEX displays
    volatile int *HEX5_HEX4_ptr;

	int fd = -1;				        // used to open /dev/mem for access to physical addresses
	void *LW_virtual;			        // used to map physical addresses for the light-weight bridge
	
    //vars for scrolling functionality and pausing
    struct timespec ts;
    ts.tv_sec = 0;					// used to delay
	ts.tv_nsec = 500000000;			// 5 * 10^8 ns = 0.5 sec
	int scroll_count = 0;
	bool pause = false;
    
    // catch SIGINT from ctrl+c, instead of having it abruptly close this program
    signal(SIGINT, catchSIGINT);

    // Create access to the FPGA light-weight bridge
	if ((fd = open_physical (fd)) == -1)
		return (-1);
	else if ((LW_virtual = map_physical (fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN)) == NULL)
		return (-1);

    // Set virtual address pointers to the I/O ports
	KEY_ptr = (unsigned int *) (LW_virtual + KEY_BASE);
    HEX3_HEX0_ptr = (unsigned int *) (LW_virtual + HEX3_HEX0_BASE);
    HEX5_HEX4_ptr = (unsigned int *) (LW_virtual + HEX5_HEX4_BASE);

    *(KEY_ptr + 3) = 0xF;  //clear edgecapture register for KEYs

    while (!stop){

		if(!pause){                          
		    *(HEX3_HEX0_ptr) = (*(message_pointer + scroll_count + 2) << 24) |  //must shift and or the bits
		                       (*(message_pointer + scroll_count + 3) << 16) |  //to create 1 byte array 
		                       (*(message_pointer + scroll_count + 4) <<  8) |  //to write at HEX pointer
		                        *(message_pointer + scroll_count + 5);
		    *(HEX5_HEX4_ptr) = 0x0 | (*(message_pointer + scroll_count) << 8) 
		                           | (*(message_pointer + scroll_count + 1));

			nanosleep (&ts, NULL);                      //added shifting delay

			++scroll_count;
			if(scroll_count >= 19) scroll_count = 0;	//reset message bit array pointer to loop message
		}

        if( *(KEY_ptr + 3) != 0 ){                      //check KEY edgebits to enable pause
			if(pause) pause = false;
			else pause = true;
            *(KEY_ptr + 3) = 0xF;                       //reset edgecapture reg
        }

    }

    *HEX3_HEX0_ptr = 0;
    *HEX5_HEX4_ptr = 0;
	unmap_physical (LW_virtual, LW_BRIDGE_SPAN);	// release the physical-memory mapping
	close_physical (fd);	// close /dev/mem
	printf ("\nExiting sample solution program\n");
    return 0;
}

/* Function to allow clean exit of the program */
void catchSIGINT(int signum)
{
	stop = 1;
}