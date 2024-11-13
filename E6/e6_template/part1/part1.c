#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define video_BYTES 8

int screen_x, screen_y;

int main(int argc, char *argv[]){

    int  video_FD;              // file descriptor
    char buffer[video_BYTES];   // buffer for data read from /dev/video 
    char command[64];           // buffer for commands written to /dev/video
    
    srand( time(NULL) );

    // Open the character device driver
    if ( (video_FD = open("/dev/video", O_RDWR) ) == -1 ) {
        printf("Error opening /dev/video: %s\n", strerror(errno));
        return -1;
    }

    read(video_FD, buffer, sizeof(buffer));
    sscanf(buffer,"%d %d", &screen_x, &screen_y);       // read buffer and set screen_x, screen_y

    
    sprintf(command, "clear", NULL);
    write(video_FD, command, sizeof(command));          // clear VGA display

    /*
    *   Randomly color all pixels of the screen 
    */
    size_t i, j;
    for ( i = 0; i <= screen_x; i++ ) {
        for ( j = 0; j <= screen_y; j++ ) {

            sprintf(command, "pixel %d,%d %hX", i, j, (short int) rand() );
            write(video_FD, command, sizeof(command));
            
        } 
    }

    (void) getchar ();       // wait for user to press return

    /*
    *   Color all pixels of the screen to green
    */
    for ( i = 0; i <= screen_x; i++ ) {
        for ( j = 0; j <= screen_y; j++ ) {

            sprintf(command, "pixel %d,%d %hX", i, j, 0x0F00 );
            write(video_FD, command, sizeof(command));
            
        } 
    }

    (void) getchar ();                                  // wait for user to press return

    sprintf(command, "clear", NULL);
    write(video_FD, command, sizeof(command));          // clear VGA display
    
    close(video_FD);
    return 0;
}