#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "ADXL345.h"
#include "accel_wrappers.h"

#define accel_BYTES 20

int R, x, y, z, scale_factor;

/**             your part 2 user code here                   **/
/**  hint: you can call functions from ../accel_wrappers.c   **/
int main(void){

    int  accel_FD;              // file descriptor
    char buffer[accel_BYTES];   // buffer for data read from /dev/accel 
    int read_from_char;

    // Open the character device driver
    if ( accel_open() == 0 ) {
        printf("Error opening /dev/accel: %s\n", strerror(errno));
        return -1;
    }


    while(1){

        read_from_char = accel_read(&R, &x, &y, &z, &scale_factor);
        if ( !read_from_char ) {
            printf("Error reading from /dev/accel: %s\n", strerror(errno));
            return -1;
        }
        else{
            printf("X = %d, Y = %d, Z = %d\n", x*scale_factor, y*scale_factor, z*scale_factor);
        }

        /*
        read(accel_FD, buffer, sizeof(buffer));
        sscanf(buffer,"%d %d %d %d %d", &R, &x, &y, &z, &scale_factor);
        if(R == 1) printf("X = %d, Y = %d, Z = %d\n", x*scale_factor, y*scale_factor, z*scale_factor);
        */

    }





    close(accel_FD);
    return 0;
}
