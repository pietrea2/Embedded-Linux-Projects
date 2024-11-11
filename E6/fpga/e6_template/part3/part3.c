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
#define true 1
#define false 0
int screen_x, screen_y;
void catchSIGINT(int signum);
volatile sig_atomic_t stop;

void catchSIGINT(int signum)
{
    stop = 1;
}

int main(int argc, char *argv[]){
    stop = 0;
    int video_FD;   // file descriptor
    char buffer[video_BYTES];   // buffer for data read from /dev/video 
    char command[64];   // buffer for commands written to /dev/video
    size_t i, j;
    srand(time(NULL));
    signal(SIGINT, catchSIGINT);
    if ((video_FD = open("/dev/video",O_RDWR) ) == -1){
        printf("Error opening /dev/video: %s\n", strerror(errno));
        return -1;
    }

    read(video_FD, buffer, sizeof(buffer));
    sscanf(buffer,"%d %d", &screen_x, &screen_y);
    i = 2;
    j = 1;
    sprintf(command, "clear");
    write(video_FD, command, sizeof(command));
    while(!stop){
        
        /*
        *   Randomly color all pixels of the screen 
        */
        /* Draw a few lines */
        sprintf (command, "line %d,%d %d,%d %hX\n", i - j - j, 0, i - j - j,
        screen_y - 1, 0x0); // yellow
        write(video_FD, command, sizeof(command));
        sprintf (command, "line %d,%d %d,%d %hX\n", i, 0, i,
        screen_y - 1, 0xFFFF); // yellow
        write(video_FD, command, sizeof(command));
        sprintf(command, "sync");
        write(video_FD, command, sizeof(command));
        if (i >= screen_x - 10){
            j = -1;
        }
        if (i <= 2 ){
            j = 1;
        }
        i += j;
        sprintf (command, "line %d,%d %d,%d %hX\n", i - j - j, 0, i - j - j,
        screen_y - 1, 0x0); // yellow
        write(video_FD, command, sizeof(command));
        sprintf (command, "line %d,%d %d,%d %hX\n", i, 0, i,
        screen_y - 1, 0xCCCC); // yellow
        write(video_FD, command, sizeof(command));
        sprintf(command, "sync");
        write(video_FD, command, sizeof(command));
        if (i >= screen_x - 10){
            j = -1;
        }
        if (i <= 2 ){
            j = 1;
        }
        i += j;
        
    }
    printf("done\n");
    sprintf(command, "clear");
    write(video_FD, command, sizeof(command));
    sprintf(command, "clear");
    write(video_FD, command, sizeof(command));

    close(video_FD);
    return 0;

    
}