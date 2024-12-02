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

int main(void)
{

    // declare and set up address pointer to AUDIO controller
    volatile int *AUDIO_ptr;
    void *LW_virtual;
    int fd = -1;

    if ((fd = open_physical(fd)) == -1)
        return (-1);
    else if ((LW_virtual = map_physical(fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN)) == NULL)
        return (-1);
    printf("cock\n");
    fflush(stdout);
    AUDIO_ptr = (unsigned int *)(LW_virtual + AUDIO_BASE);
    *AUDIO_ptr = 0x8; // Set CW to 1
    sleep(1);
    *AUDIO_ptr = 0x0; // Set CW to 0
    sleep(1);
    printf("cock2\n");
    fflush(stdout);
    double scale[13] = {MIDC, DFLAT, DNAT, EFLAT, ENAT, FNAT, GFLAT, GNAT, AFLAT, ANAT, BFLAT, BNAT, HIC};
    int note;
    int nth_sample = 0;
    printf("cock3\n");
    fflush(stdout);
    for (note = 0; note < 1; note++)
    {

        // sleep(1);
        *AUDIO_ptr = 0x8; // Set CW to 1
        *AUDIO_ptr = 0x0; // Set CW to 0
        printf("cock\n");
        fflush(stdout);
        // Loop thru notes
        // for(nth_sample = 0; nth_sample < SAMPLING_RATE; nth_sample++){
        while (1)
        {
            printf("%d \n", *AUDIO_ptr);
            fflush(stdout);
            // Wait until space is avail in outgoing fifo (check if WSLC and WSRC is not 0 = empty)
            while (*(int *)(AUDIO_ptr + FIFOSPACE) & 0x00FF0000 == 0 || *(int *)(AUDIO_ptr + FIFOSPACE) & 0xFF000000 == 0)
            {
            }

            *(AUDIO_ptr + LDATA) = (int)(0);
            *(AUDIO_ptr + RDATA) = (int)(0);

            nth_sample++;
            sleep(1);
        }
    }

    if (unmap_physical(LW_virtual, LW_BRIDGE_SPAN) != 0)
        printf("ERROR unmapping virtual address mapping");
    close_physical(fd);

    return 0;
}