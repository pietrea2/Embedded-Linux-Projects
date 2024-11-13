#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include "part4.h"

/**  your part 1 user code here  **/
#define video_BYTES 8
#define true 1
#define false 0
#define NUM_XS_MAX 5
#define NUM_XS 5
int screen_x, screen_y;
void catchSIGINT(int signum);
volatile sig_atomic_t stop;
// void set_steps(struct vertex* v);
// void move_vertex(struct vertex* v);

struct vertex
{
    int row;
    int col;
    int col_step;
    int row_step;
	int last_col0;
	int last_row0;
	int last_col1;
	int last_row1;
};


void catchSIGINT(int signum)
{
    stop = 1;
}

int main(int argc, char *argv[]){
    stop = 0;
    int video_FD;   // file descriptor
    char buffer[video_BYTES];   // buffer for data read from /dev/video 
    char command[64];   // buffer for commands written to /dev/video
    size_t i;
	int boofer;
	boofer = 0;
    srand(time(NULL));
    signal(SIGINT, catchSIGINT);
    if ((video_FD = open("/dev/video",O_RDWR) ) == -1){
        printf("Error opening /dev/video: %s\n", strerror(errno));
        return -1;
    }


    struct vertex vertexes[NUM_XS_MAX];
    int num_xs = NUM_XS;
    read(video_FD, buffer, sizeof(buffer));
    sscanf(buffer,"%d %d", &screen_x, &screen_y);
    for (i = 0; i < NUM_XS_MAX; i++)
    {
        vertexes[i].row = rand() % screen_y + 1;
        vertexes[i].col = rand() % screen_x + 1;
        vertexes[i].col_step = rand() % 2 ? 1 : -1;
        vertexes[i].row_step = rand() % 2 ? 1 : -1;
		vertexes[i].last_col0 = 0;
		vertexes[i].last_col1 = 0;
		vertexes[i].last_row0 = 0;
		vertexes[i].last_row1 = 0;
    }

    
    sprintf(command, "clear");
    write(video_FD, command, sizeof(command));
	
    while(!stop){

        for (i = 0; i < num_xs; i++)
        {
			if(boofer){
				sprintf (command, "pixel %d,%d %hX\n", vertexes[i].last_col1,
                vertexes[i].last_row1, 0x0); // yellow
            write(video_FD, command, sizeof(command));
			} else {
				sprintf (command, "pixel %d,%d %hX\n", vertexes[i].last_col0,
                vertexes[i].last_row0, 0x0); // yellow
            write(video_FD, command, sizeof(command));
			}
            
            sprintf (command, "pixel %d,%d %hX\n", vertexes[i].col,
                vertexes[i].row, 0xFFFF);
            write(video_FD, command, sizeof(command));
        }

        for (i = 0; i < num_xs; i++) 
        {
				int last_cola = boofer ? vertexes[i].last_col1 : vertexes[i].last_col0;
				int last_rowa = boofer ? vertexes[i].last_row1 : vertexes[i].last_row0;
                if (i == num_xs - 1)
                {
					
					int last_colb = boofer ? vertexes[0].last_col1 : vertexes[0].last_col0;
					int last_rowb = boofer ? vertexes[0].last_row1 : vertexes[0].last_row0;
                    sprintf (command, "line %d,%d %d,%d %hX\n", last_cola,
                        last_rowa, last_colb, last_rowb, 0);
                    write(video_FD, command, sizeof(command));
                    sprintf (command, "line %d,%d %d,%d %hX\n", vertexes[i].col,
                        vertexes[i].row, vertexes[0].col, vertexes[0].row, 0xCCCC);
                    write(video_FD, command, sizeof(command));
                }
                else
                {
                    int last_colb = boofer ? vertexes[i+1].last_col1 : vertexes[i+1].last_col0;
					int last_rowb = boofer ? vertexes[i+1].last_row1 : vertexes[i+1].last_row0;
                    sprintf (command, "line %d,%d %d,%d %hX\n", last_cola,
                        last_rowa, last_colb, last_rowb, 0);
                    write(video_FD, command, sizeof(command));
                    sprintf (command, "line %d,%d %d,%d %hX\n", vertexes[i].col,
                        vertexes[i].row, vertexes[i+1].col, vertexes[i+1].row, 0xCCCC);
                    write(video_FD, command, sizeof(command));
                }
        }
        
        sprintf(command, "sync");
        write(video_FD, command, sizeof(command));
        for ( i = 0; i < num_xs; i++)
        {
            if (vertexes[i].col >= screen_x - 10){
                
                vertexes[i].col_step = -1;
            
            }
            if (vertexes[i].col <= 2 ){
                
                vertexes[i].col_step = 1;
            }
            if (vertexes[i].row >= screen_y - 10){
                
                vertexes[i].row_step = -1;
            }
            if (vertexes[i].row <= 2) {
                
                vertexes[i].row_step = 1;
            }
			if (boofer){
				vertexes[i].last_col1 = vertexes[i].col;
				vertexes[i].last_row1 = vertexes[i].row;
			} else {
				vertexes[i].last_col0 = vertexes[i].col;
				vertexes[i].last_row0 = vertexes[i].row;
			}
			
            vertexes[i].col += vertexes[i].col_step;
            vertexes[i].row += vertexes[i].row_step;
        }
		
		boofer = !boofer;
		
    }
    printf("done\n");
    sprintf(command, "clear");
    write(video_FD, command, sizeof(command));
    sprintf(command, "clear");
    write(video_FD, command, sizeof(command));

    close(video_FD);
    return 0;
}
void edge_clear(char command[64], size_t i, size_t j, size_t k, size_t l, int video_FD)
{
    
}
