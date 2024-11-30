#define _GNU_SOURCE
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
#include "address_map_arm.h"
#include "defines.h"

#define PLAYING_TIME 1000

volatile sig_atomic_t stop;

void catchSIGINT (int signum) {
    stop = 1;
}

int main(int argc, char **argv){

    // declare and set up address pointer to AUDIO controller
    volatile int *AUDIO_ptr;
    void *LW_virtual;
    int fd = -1;

    stop = 0;
    signal(SIGINT, catchSIGINT);

    // Vars for reading and creating the user 13-char string input argument to play certain notes
    int note[13] = {0};
    size_t i;
    double freq_sum = 0;

    if (argc < 2){
        printf("Insuficient argument amount\n");
        exit(1);
    }
    /*
    if ( strlen(argv[1]) != 13 ){
        printf("String argument is NOT 13 characters long\n");
        exit(1);
    }
    */
    for (i = 0; i < 13; i++){
        char k;
        sscanf(&argv[1][i], "%c", &k);
        /*
        if ( k != '1' || k != '0' ){
            if( k == '\n' || k == 10 ){}
            else{
                printf("String argument is not acceptable!\n");
                exit(1);
            }
        }
        */
        if ( k == '1' ){
            note[i] = 1;
        }
    }



    if ((fd = open_physical(fd)) == -1)
        return (-1);
    else if ((LW_virtual = map_physical(fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN)) == NULL)
        return (-1);

    AUDIO_ptr = (unsigned int *)(LW_virtual + AUDIO_BASE);
    *AUDIO_ptr |= 0x8;        // Set CW to 1
    *AUDIO_ptr &= 0xFFFFFFF7; // Set CW to 0



    double scale[13] = {MIDC, DFLAT, DNAT, EFLAT, ENAT, FNAT, GFLAT, GNAT, AFLAT, ANAT, BFLAT, BNAT, HIC};
    int nth_sample = 0;

    //printf("left:%#x right:%#x\n", *(AUDIO_ptr + 1) & 0xFF000000, *(AUDIO_ptr + 1) & 0x00FF0000);
    for (nth_sample = 0; nth_sample < (SAMPLING_RATE * PLAYING_TIME / 1000) && !stop; nth_sample++){
        
        int write = 0;

        while (write == 0 && !stop){

            if ( (*(AUDIO_ptr + FIFOSPACE) & 0x00FF0000) && (*(AUDIO_ptr + FIFOSPACE) & 0xFF000000) ){
                
                freq_sum = 0;
                for ( i = 0; i < 13; i++) {
                    freq_sum += (MAX_VOLUME/13) * sin(nth_sample * note[i] * scale[i]);
                }
                // printf("frrr %lf\n", freq_sum);

                *(AUDIO_ptr + LDATA) = (int)(freq_sum);
                *(AUDIO_ptr + RDATA) = (int)(freq_sum);

                write = 1;
            }
        }
    }
    

    if (unmap_physical(LW_virtual, LW_BRIDGE_SPAN) != 0){
        printf("ERROR unmapping virtual address mapping");
    }
    close_physical(fd);

    return 0;
}
