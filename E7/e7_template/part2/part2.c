#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "ADXL345.h"
#include "accel_wrappers.h"

int R, x, y, z, scale_factor;

volatile sig_atomic_t stop;

void catchSIGINT(int signum);

int main(void){

    // Open the character device driver
    if ( accel_open() == 0 ) {
        printf("Error opening /dev/accel: %s\n", strerror(errno));
        return -1;
    }

    // catch SIGINT from ctrl+c, instead of having it abruptly close this program
    signal(SIGINT, catchSIGINT);

    int read_from_char;
    
    while(!stop){

        //read_from_char = accel_read(&R, &x, &y, &z, &scale_factor);
        if ( !accel_read(&R, &x, &y, &z, &scale_factor) ) {
            continue;
        }
        else{
            printf("X = %d, Y = %d, Z = %d\n", x*scale_factor, y*scale_factor, z*scale_factor);
        }
    
        usleep(100000);
    }

    accel_close();
    return 0;
}

void catchSIGINT(int signum)
{
    stop = 1;
}