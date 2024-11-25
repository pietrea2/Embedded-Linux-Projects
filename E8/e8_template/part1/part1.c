#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <physical.h>
#include <linux/input.h>
#include <pthread.h>
#include "address_map_arm.h"
#include "defines.h"

#define PLAYING_TIME 300

volatile sig_atomic_t stop;

void catchSIGINT (int signum) {
	stop = 1;
}

int main(void){

    // declare and set up address pointer to AUDIO controller
    volatile int *AUDIO_ptr;
    void *LW_virtual;
    int fd = -1;

    stop = 0;
    signal(SIGINT, catchSIGINT);

    if ((fd = open_physical(fd)) == -1)
        return (-1);
    else if ((LW_virtual = map_physical(fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN)) == NULL)
        return (-1);

    AUDIO_ptr = (unsigned int *)(LW_virtual + AUDIO_BASE);
    *AUDIO_ptr |= 0x8;        // Set CW to 1
    *AUDIO_ptr &= 0xFFFFFFF7; // Set CW to 0

    double scale[13] = {MIDC, DFLAT, DNAT, EFLAT, ENAT, FNAT, GFLAT, GNAT, AFLAT, ANAT, BFLAT, BNAT, HIC};
    int note;
    int nth_sample = 0;

    //printf("left:%#x right:%#x\n", *(AUDIO_ptr + 1) & 0xFF000000, *(AUDIO_ptr + 1) & 0x00FF0000);
    for (note = 0; note < 13 && !stop; note++){

        for (nth_sample = 0; nth_sample < (SAMPLING_RATE * PLAYING_TIME / 1000) && !stop; nth_sample++){
            
            int write = 0;
            while (write == 0 && !stop){

                if ( (*(AUDIO_ptr + FIFOSPACE) & 0x00FF0000) && (*(AUDIO_ptr + FIFOSPACE) & 0xFF000000) ){
                    *(AUDIO_ptr + LDATA) = (int)( MAX_VOLUME * sin(nth_sample * scale[note]) );
                    *(AUDIO_ptr + RDATA) = (int)( MAX_VOLUME * sin(nth_sample * scale[note]) );

                    write = 1;
                }
            }
        }
    }


    if (unmap_physical(LW_virtual, LW_BRIDGE_SPAN) != 0){
        printf("ERROR unmapping virtual address mapping");
    }
    close_physical(fd);

    return 0;
}
