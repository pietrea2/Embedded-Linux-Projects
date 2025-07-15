#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include "physical.h"
#include "address_map_arm.h"

volatile sig_atomic_t stop;

void catchSIGINT (int signum) {
	stop = 1;
}

int main(void){

    // declare and set up address pointer to ADC port
    volatile int *ADC_ptr;
    void *LW_virtual;
    int fd = -1;
    float sample;

    stop = 0;
    signal(SIGINT, catchSIGINT);

    if ((fd = open_physical(fd)) == -1)
        return (-1);
    else if ((LW_virtual = map_physical(fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN)) == NULL)
        return (-1);

    ADC_ptr = (unsigned int *)(LW_virtual + ADC_BASE);
    *(ADC_ptr + 1) = 1;                             // Activate AUTO-update



    while(!stop){

        while( (*ADC_ptr & 0x8000) == 0 ){}         // Wait for R = 1

        sample = *ADC_ptr & 0xFFF;                  // Take 12 bit sample
        sample = sample * 5.0/4095.0;               // Convert to volts
        printf("%.2lf v\n", sample);
        sleep(1);
    }

    if (unmap_physical(LW_virtual, LW_BRIDGE_SPAN) != 0){
        printf("ERROR unmapping virtual address mapping");
    }
    close_physical(fd);

    return 0;
}