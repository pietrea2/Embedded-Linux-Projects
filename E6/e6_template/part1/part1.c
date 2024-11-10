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
    int x, y;
    size_t i, j;
    srand(time(NULL));

    if ((video_FD = open("/dev/video",O_RDWR) ) == -1){
        printf("Error opening /dev/video: %s\n", strerror(errno));
        return -1;
    }
    read(video_FD, buffer, sizeof(buffer));
    sscanf(buffer,"%d %d", &x, &y);

    /*
    *   Randomly color all pixels of the screen 
    */
    sprintf(command, "clear", NULL);
    write(video_FD, command, sizeof(command));
    for ( i = 0; i <= x; i++)
    {
        for ( j = 0; j <= x; j++)
        {
            if (rand() % 2){
                sprintf(command, "pixel %d,%d %hX", i, j, (short int)rand());
                write(video_FD, command, sizeof(command));
                // sleep(1);
            }
        }
        
    }
    
    

    close(video_FD);
    return 0;
}