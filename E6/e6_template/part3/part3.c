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
volatile sig_atomic_t stop;

void catchSIGINT(int signum);

int main(int argc, char *argv[]){

    signal(SIGINT, catchSIGINT);
    stop = 0;
    int video_FD;               // file descriptor
    char buffer[video_BYTES];   // buffer for data read from /dev/video 
    char command[64];           // buffer for commands written to /dev/video

    srand(time(NULL));
    
    if ( (video_FD = open("/dev/video",O_RDWR) ) == -1){
        printf("Error opening /dev/video: %s\n", strerror(errno));
        return -1;
    }

    read(video_FD, buffer, sizeof(buffer));             // read buffer and set screen_x, screen_y
    sscanf(buffer,"%d %d", &screen_x, &screen_y);

    sprintf(command, "clear");
    write(video_FD, command, sizeof(command));          // clear VGA display


    int line_x1 = 59;
    int line_x2 = 259;
    int line_y = 0;
    int y_step = 1;

    sprintf (command, "line %d,%d %d,%d %hX\n", line_x1, line_y, line_x2, line_y, 0xFFE0); // draw yellow horizontal line
    write (video_FD, command, strlen(command));

    while(!stop){
        
        sprintf(command, "sync");                       // VGA sync
        write(video_FD, command, sizeof(command));

        line_y += y_step;                               // move line y coordinate

        sprintf (command, "line %d,%d %d,%d %hX\n", line_x1, (line_y - y_step), line_x2, (line_y - y_step), 0x0);   // erase previous line (draw black)
        write (video_FD, command, strlen(command));

        sprintf (command, "line %d,%d %d,%d %hX\n", line_x1, line_y, line_x2, line_y, 0xFFE0);                      // draw yellow horizontal line
        write (video_FD, command, strlen(command));

        if( line_y == 239 ){
            y_step = -1;
        }
        if( line_y == 0 ){
            y_step = 1;
        }
        
    }

    sprintf(command, "clear");                          // clear VGA display
    write(video_FD, command, sizeof(command));

    close(video_FD);
    return 0;
}

void catchSIGINT(int signum) {
    stop = 1;
}