#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "ADXL345.h"
#include "accel_wrappers.h"

#define accel_BYTES 40

int16_t x, y, z, scale_factor;
int R;

/**             your part 2 user code here                   **/
/**  hint: you can call functions from ../accel_wrappers.c   **/
int main(void){

    int  accel_FD;              // file descriptor
    char buffer[accel_BYTES];   // buffer for data read from /dev/accel 

    // Open the character device driver
    if ( (accel_FD = open("/dev/accel", O_RDWR) ) == -1 ) {
        printf("Error opening /dev/accel: %s\n", strerror(errno));
        return -1;
    }


    while(1){
        
        read(accel_FD, buffer, sizeof(buffer));
        sscanf(buffer,"%d %d %d %d %d", &R, &x, &y, &z, &scale_factor);
        printf("%d %d %d %d %d\n", R, x, y, z, scale_factor);

    }





    close(accel_FD);
    return 0;
}
