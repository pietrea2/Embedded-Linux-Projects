#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <time.h>
#include "../include/address_map_arm.h"
#include "../include/physical.h"
#include <stdbool.h>


// 7-seg bit patterns for scrolling message: [Intel SoC FPGA     Intel]
char message[25] = {'I', 'n', 't', 'e', 'l', 
                    ' ',
                    'S', 'o', 'C', 
                    ' ',
                    'F', 'P', 'G', 'A', 
                    ' ', ' ', ' ', ' ', ' ',
                    'I', 'n', 't', 'e', 'l'};

char * message_pointer = &message[0];

// used to exit the program cleanly
volatile sig_atomic_t stop;
void catchSIGINT(int);

int main(void)
{
	volatile int *KEY_ptr;        // virtual address for the KEY port

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
    *(KEY_ptr + 3) = 0xF;  //clear edgecapture register for KEYs

    while (!stop){

		if(!pause){        
            /*
            \e[2J    = clear window
            \e[H     = move the cursor to the home position
            \e[2;38H = set cursor location to row yy, column xx
            \e[?25l  = hide the cursor
            \e[37m   = set color to white (foreground)  
            */                
		    printf ("\e[?25l\e[37m\e[2J\e[H\e[2;38H ------ \n");
            printf ("\e[3;38H|");
            printf("\e[31m%c%c%c%c%c%c", message[scroll_count], 
                                         message[scroll_count+1],
                                         message[scroll_count+2],
                                         message[scroll_count+3],
                                         message[scroll_count+4],
                                         message[scroll_count+5]);
            printf ("\e[37m|\n");
            printf ("\e[4;38H ------ \n");
            fflush(stdout);

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

    //show the cursor
    printf ("\e[?25h");
    fflush(stdout);

    unmap_physical (LW_virtual, LW_BRIDGE_SPAN);	// release the physical-memory mapping
	close_physical (fd);	// close /dev/mem

    return 0;
}

/* Function to allow clean exit of the program */
void catchSIGINT(int signum)
{
	stop = 1;
}
