#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "ADXL345.h"
#include "accel_wrappers.h"

int R, x, y, z, scale_factor;

int main(void){

    // Open the character device driver
    if ( accel_open() == 0 ) {
        printf("Error opening /dev/accel: %s\n", strerror(errno));
        return -1;
    }

    int read_from_char;
    int x_avg = 0;
    int y_avg = 0;
    int z_avg = 0;
    int c;
    
    while(1){

        read_from_char = accel_read(&R, &x, &y, &z, &scale_factor);
        if ( !read_from_char ) {
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
