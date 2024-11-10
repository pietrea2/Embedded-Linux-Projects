#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

/**  your part 1 user code here  **/
#define video_BYTES 8

int screen_x, screen_y;

int main(int argc, char *argv[]){
    int video_FD;   // file descriptor
    char buffer[video_BYTES];   // buffer for data read from /dev/video 
    char command[64];   // buffer for commands written to /dev/video
    size_t i, j;
    srand(time(NULL));

    if ((video_FD = open("/dev/video",O_RDWR) ) == -1){
        printf("Error opening /dev/video: %s\n", strerror(errno));
        return -1;
    }
    read(video_FD, buffer, sizeof(buffer));
    sscanf(buffer,"%d %d", &screen_x, &screen_y);

    /*
    *   Randomly color all pixels of the screen 
    */
    sprintf(command, "clear", NULL);
    write(video_FD, command, sizeof(command));
    /* Draw a few lines */
    sprintf (command, "line %d,%d %d,%d %hX\n", 0, screen_y - 1, screen_x - 1,
    0, 0xFFE0); // yellow
    write (video_FD, command, strlen(command));
    sprintf (command, "line %d,%d %d,%d %hX\n", 0, screen_y - 1,
    (screen_x >> 1) - 1, 0, 0x07FF); // cyan
    write (video_FD, command, strlen(command));

    

    close(video_FD);
    return 0;
}