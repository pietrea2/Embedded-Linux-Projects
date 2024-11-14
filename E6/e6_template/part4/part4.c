#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define video_BYTES 8
#define NUM_VERTEX 8

int box_size = 2;           // CHANGE SIZE OF BOXES
int screen_x, screen_y;
volatile sig_atomic_t stop;

struct vertex {
    int row;
    int col;
    int col_step;
    int row_step;
	int last_col_buf0;      // store last coordinates to erase for buffer 0 (front/back)
	int last_row_buf0;
	int last_col_buf1;      // store last coordinates to erase for buffer 1 (front/back)
	int last_row_buf1;
    short int color;
};

void set_steps(struct vertex* v);
void move_vertex(struct vertex* v);
void catchSIGINT(int signum);

int main(int argc, char *argv[]) {

    signal(SIGINT, catchSIGINT);
    stop = 0;
    int video_FD;               // file descriptor
    char buffer[video_BYTES];   // buffer for data read from /dev/video 
    char command[64];           // buffer for commands written to /dev/video

    srand( time(NULL) );

    if ( (video_FD = open("/dev/video",O_RDWR) ) == -1){
        printf("Error opening /dev/video: %s\n", strerror(errno));
        return -1;
    }

	sleep(1);
    read(video_FD, buffer, sizeof(buffer));             // read buffer and set screen_x, screen_y
    sscanf(buffer,"%d %d", &screen_x, &screen_y);

    int a;
    for(a = 0; a < 2; a++){                             // do twice! for each pixel buffer
        sprintf(command, "clear");
        write(video_FD, command, sizeof(command));      // clear VGA display
        sprintf(command, "sync");                       // VGA sync
        write(video_FD, command, sizeof(command));
		sleep(0.1);
    }

	sleep(1);

    struct vertex vertexes[NUM_VERTEX];                 // init 8 vertex positions and colors (random)
    int num_xs = NUM_VERTEX;
    size_t i;
    for (i = 0; i < num_xs; i++) {
        vertexes[i].row = rand() % (screen_y - box_size);
        vertexes[i].col = rand() % (screen_x - box_size);
        vertexes[i].col_step = rand() % 2 ? 1 : -1;
        vertexes[i].row_step = rand() % 2 ? 1 : -1;
		vertexes[i].last_col_buf1 = 0;
		vertexes[i].last_col_buf0 = 0;
		vertexes[i].last_row_buf1 = 0;
		vertexes[i].last_row_buf0 = 0;
        vertexes[i].color = (short int) ( rand() % 65536 ) + 60;  // rand color (other than black)
    }

	sleep(1);

    // draw first vertices and lines
    for (i = 0; i < num_xs; i++) {
        sprintf (command, "box %d,%d %d,%d %hX\n", vertexes[i].col, vertexes[i].row, vertexes[i].col + box_size, vertexes[i].row + box_size, vertexes[i].color); // draw box (vertex)
        write (video_FD, command, strlen(command));

        sprintf (command, "line %d,%d %d,%d %hX\n", vertexes[i].col, vertexes[i].row, vertexes[(i + 1)%num_xs].col, vertexes[(i + 1)%num_xs].row, vertexes[i].color); // draw line
        write (video_FD, command, strlen(command));
    }

    sprintf(command, "sync");                       // VGA sync
    write(video_FD, command, sizeof(command));

    // save this last col for both buffers
    for ( i = 0; i < num_xs; i++){  
        vertexes[i].last_col_buf0 = vertexes[i].col;
        vertexes[i].last_row_buf0 = vertexes[i].row;
        vertexes[i].last_col_buf1 = vertexes[i].col;
        vertexes[i].last_row_buf1 = vertexes[i].row;
        move_vertex(&vertexes[i]);
    }

    int buffer_num;
	buffer_num = 0;

    while(!stop){

        // Clear previous vertices and lines (draw black)
        for (i = 0; i < num_xs; i++) {

            int last_col_1 = buffer_num ? vertexes[i].last_col_buf1 : vertexes[i].last_col_buf0;
            int last_row_1 = buffer_num ? vertexes[i].last_row_buf1 : vertexes[i].last_row_buf0;
           
            int last_col_2 = buffer_num ? vertexes[(i + 1)%num_xs].last_col_buf1 : vertexes[(i + 1)%num_xs].last_col_buf0;
            int last_row_2 = buffer_num ? vertexes[(i + 1)%num_xs].last_row_buf1 : vertexes[(i + 1)%num_xs].last_row_buf0;
			
            sprintf (command, "box %d,%d %d,%d %hX\n", last_col_1, last_row_1, (last_col_1 + box_size), (last_row_1 + box_size), 0x0);
            write (video_FD, command, strlen(command));

            sprintf (command, "line %d,%d %d,%d %hX\n", last_col_1, last_row_1, last_col_2, last_row_2, 0x0);
            write (video_FD, command, strlen(command));
        }

        // Draw new vertices and lines
        for (i = 0; i < num_xs; i++) {
            sprintf (command, "box %d,%d %d,%d %hX\n", vertexes[i].col, vertexes[i].row, vertexes[i].col + box_size, vertexes[i].row + box_size, vertexes[i].color);
            write (video_FD, command, strlen(command));

            sprintf (command, "line %d,%d %d,%d %hX\n", vertexes[i].col, vertexes[i].row, vertexes[(i + 1)%num_xs].col, vertexes[(i + 1)%num_xs].row, vertexes[i].color);
            write (video_FD, command, strlen(command));
        }
        
        sprintf(command, "sync");                   // sync VGA
        write(video_FD, command, sizeof(command));

		buffer_num = !buffer_num;                   // buffer switched after sync

        // Save current positions for other buffer + update positons
        for ( i = 0; i < num_xs; i++){
			if (buffer_num){
				vertexes[i].last_col_buf1 = vertexes[i].col;
				vertexes[i].last_row_buf1 = vertexes[i].row;
			}
            else {
				vertexes[i].last_col_buf0 = vertexes[i].col;
				vertexes[i].last_row_buf0 = vertexes[i].row;
			}
            move_vertex(&vertexes[i]);
        }
    }



    for(a = 0; a < 2; a++){                             // do twice for each pixel buffer
        sprintf(command, "clear");
        write(video_FD, command, sizeof(command));      // clear VGA display
        sprintf(command, "sync");                       // VGA sync
        write(video_FD, command, sizeof(command));
    }

    close(video_FD);
    return 0;
}

void catchSIGINT(int signum) {
    stop = 1;
}

//update row and col step variable to deal with screen boundaries
//in order to 'bounce' the vertex when it reaches a boundary
void set_steps(struct vertex* v){

    //row boundary: 0 <= Y <= 239
    if (v -> row == (screen_y - box_size) - 1 ) {
        v -> row_step = -1;
        //v ->row = screen_y - box_size - 2;
    }
    else if (v ->row == 0) {
        v -> row_step = 1;
        //v ->row = 1;
    }

    //col boundary: 0 <= X <= 319
    if (v->col == (screen_x - box_size) - 1 ) {
        v->col_step = -1;
        //v->col = screen_x - box_size - 2;
    } 
    else if (v->col == 0) {
        v->col_step = 1;
        //v->col = 1;
    }
}

void move_vertex(struct vertex* v){
    v -> row += v -> row_step;
    v -> col += v -> col_step;
    set_steps(v);
}
