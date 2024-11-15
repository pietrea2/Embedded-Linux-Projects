#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <intelfpgaup/KEY.h> // Include libraries for char device drivers
#include <intelfpgaup/SW.h>

#define video_BYTES 8
#define NUM_VERTEX_MAX 25
#define NUM_VERTEX 8

#define video_WHITE		0xFFFF
#define video_YELLOW 	0xFFE0
#define video_RED		0xF800
#define video_GREEN		0x07E0
#define video_BLUE		0x041F
#define video_CYAN		0x07FF
#define video_MAGENTA	0xF81F
#define video_GREY		0xC618
#define video_PINK		0xFC18
#define video_ORANGE	0xFC00
short int video_colours[10] = {video_WHITE, video_YELLOW, video_RED, video_GREEN, video_BLUE, video_CYAN, video_MAGENTA, video_GREY, video_PINK, video_ORANGE};

int box_size = 2;           // CAN CHANGE SIZE OF BOXES (VERTICES)
int screen_x, screen_y;
volatile sig_atomic_t stop;

struct vertex {
    int row;
    int col;
    int col_step;
    int row_step;
	int last_col_buf0;      // store previous coordinates to erase for buffer 0 (front/back)
	int last_row_buf0;
	int last_col_buf1;      // store previous coordinates to erase for buffer 1 (front/back)
	int last_row_buf1;
    short int color;
};

void set_steps(struct vertex* v, int step);
void move_vertex(struct vertex* v, int step);
void catchSIGINT(int signum);

int main(int argc, char *argv[]) {

    int KEY_data;           // used to read from driver
    int SW_data;

    struct timespec ts;
    ts.tv_sec = 0;          // used to delay animation
    ts.tv_nsec = 0;
    int speed_count = 5;
    int step_add = 1;

    signal(SIGINT, catchSIGINT);
    stop = 0;
    int video_FD;               // file descriptor
    char buffer[video_BYTES];   // buffer for data read from /dev/video 
    char command[64];           // buffer for commands written to /dev/video

    srand( time(NULL) );

    // Load video character device driver
    if ( (video_FD = open("/dev/video",O_RDWR) ) == -1){
        printf("Error opening /dev/video: %s\n", strerror(errno));
        return -1;
    }
    if ( !SW_open() ){
        printf("Error opening /dev/IntelFPGAUP/SW: %s\n", strerror(errno));
        return -1;
    }
    if ( !KEY_open() ){
        printf("Error opening /dev/IntelFPGAUP/KEY: %s\n", strerror(errno));
        return -1;
    }

	sleep(1);
    read(video_FD, buffer, sizeof(buffer));             // Read video buffer and set screen_x, screen_y
    sscanf(buffer,"%d %d", &screen_x, &screen_y);

    int a;
    for(a = 0; a < 2; a++){                             // Clear both VGA pixel buffers
        sprintf(command, "clear");
        write(video_FD, command, sizeof(command));      
        sprintf(command, "sync");
        write(video_FD, command, sizeof(command));
    }

    sleep(1);

    struct vertex vertexes[NUM_VERTEX_MAX];                       // Init 8 vertex positions and colors (random)
    int num_xs = NUM_VERTEX;
    size_t i;
    for (i = 0; i < NUM_VERTEX_MAX; i++) {
        vertexes[i].row = rand() % (screen_y - box_size);
        vertexes[i].col = rand() % (screen_x - box_size);
        vertexes[i].col_step = rand() % 2 ? 1 : -1;
        vertexes[i].row_step = rand() % 2 ? 1 : -1;
		vertexes[i].last_col_buf1 = 0;
		vertexes[i].last_col_buf0 = 0;
		vertexes[i].last_row_buf1 = 0;
		vertexes[i].last_row_buf0 = 0;
        vertexes[i].color = (short int) ( rand() % 65536 ) + 80;  // rand color (other than black)
    }

    // Draw first vertices and lines
    for (i = 0; i < num_xs; i++) {
        sprintf (command, "box %d,%d %d,%d %hX\n", vertexes[i].col, vertexes[i].row, vertexes[i].col + box_size, vertexes[i].row + box_size, vertexes[i].color);
        write (video_FD, command, strlen(command));

        sprintf (command, "line %d,%d %d,%d %hX\n", vertexes[i].col, vertexes[i].row, vertexes[(i + 1)%num_xs].col, vertexes[(i + 1)%num_xs].row, vertexes[i].color);
        write (video_FD, command, strlen(command));
    }

    sprintf(command, "sync");                           // VGA sync
    write(video_FD, command, sizeof(command));

    // Save vertices' initial coordinates as "last" locations (used to clear previous drawing)
    for ( i = 0; i < NUM_VERTEX_MAX; i++){  
        vertexes[i].last_col_buf0 = vertexes[i].col;
        vertexes[i].last_row_buf0 = vertexes[i].row;
        vertexes[i].last_col_buf1 = vertexes[i].col;
        vertexes[i].last_row_buf1 = vertexes[i].row;
        move_vertex(&vertexes[i], step_add);
    }

    // Var to keep track of which pixel buffer we are writing to
    int buffer_num;
	buffer_num = 0;

    int last_col_1;
    int last_row_1;
    int last_col_2;
    int last_row_2;

    while(!stop){
        
        SW_read(&SW_data);

        // 1. Clear previous vertices and lines (draw black)
        for (i = 0; i < num_xs; i++) {

            last_col_1 = buffer_num ? vertexes[i].last_col_buf1 : vertexes[i].last_col_buf0;
            last_row_1 = buffer_num ? vertexes[i].last_row_buf1 : vertexes[i].last_row_buf0;
           
            last_col_2 = buffer_num ? vertexes[(i + 1)%num_xs].last_col_buf1 : vertexes[(i + 1)%num_xs].last_col_buf0;
            last_row_2 = buffer_num ? vertexes[(i + 1)%num_xs].last_row_buf1 : vertexes[(i + 1)%num_xs].last_row_buf0;

            sprintf (command, "box %d,%d %d,%d %hX\n", last_col_1, last_row_1, (last_col_1 + box_size), (last_row_1 + box_size), 0x0);
            write (video_FD, command, strlen(command));

            sprintf (command, "line %d,%d %d,%d %hX\n", last_col_1, last_row_1, last_col_2, last_row_2, 0x0);
            write (video_FD, command, strlen(command));
            
        }

        // 2. Draw new vertices and lines
        for (i = 0; i < num_xs; i++) {
            sprintf (command, "box %d,%d %d,%d %hX\n", vertexes[i].col, vertexes[i].row, vertexes[i].col + box_size, vertexes[i].row + box_size, vertexes[i].color);
            write (video_FD, command, strlen(command));

            if(!SW_data){
                sprintf (command, "line %d,%d %d,%d %hX\n", vertexes[i].col, vertexes[i].row, vertexes[(i + 1)%num_xs].col, vertexes[(i + 1)%num_xs].row, vertexes[i].color);
                write (video_FD, command, strlen(command));   
            }
        }

        nanosleep(&ts, NULL); // added shifting delay
        
        // 3. Sync VGA
        sprintf(command, "sync");
        write(video_FD, command, sizeof(command));

        // buffer switched after sync
		buffer_num = !buffer_num;

		// Read KEYs and perform following animation features
        KEY_read(&KEY_data);
        switch ( KEY_data ) {
            case 1: //KEY0: Increase speed of animation
                if ( speed_count < 8){
                    if( speed_count >= 5 ){
                        step_add = step_add + 1;
                    }
                    else{
                        if(speed_count == 4) ts.tv_nsec = 0;
                        else ts.tv_nsec  = ts.tv_nsec / 2;
                    }
                    speed_count++;
                    //printf("Speed count: %d NSec: %d\n", speed_count, ts.tv_nsec);
                }
                break;
            case 2: //KEY1: Decrease speed of animation
                if ( speed_count > -2 ){
                    if( speed_count >= 6 ){
                        step_add = step_add - 1;
                    }
                    else{
                        if(speed_count == 5) ts.tv_nsec = 16500000;
                        else{
                            ts.tv_nsec  = ts.tv_nsec * 2;
                            if (ts.tv_nsec > 999999999) ts.tv_nsec = 999999999;
                        }
                    }
                    speed_count--;
                    //printf("Speed count: %d NSec: %d\n", speed_count, ts.tv_nsec);
                }
                break;
            case 4: //KEY2: Increase number of objects
                if (num_xs < 16){
                    // Save vertices' initial coordinates as "last" locations (used to clear previous drawing)
                    for ( i = 0; i < num_xs; i++){
                        if (!buffer_num){
                            vertexes[i].last_col_buf1 = vertexes[i].col;
                            vertexes[i].last_row_buf1 = vertexes[i].row;
                        }
                        else {
                            vertexes[i].last_col_buf0 = vertexes[i].col;
                            vertexes[i].last_row_buf0 = vertexes[i].row;
                        }
                    }

                    for (i = 0; i < (num_xs); i++) {

                        // Clear screen of what is stored in both buffers
                        sprintf (command, "box %d,%d %d,%d %hX\n", vertexes[i].last_col_buf0, vertexes[i].last_row_buf0, (vertexes[i].last_col_buf0 + box_size), (vertexes[i].last_row_buf0 + box_size), 0x0);
                        write (video_FD, command, strlen(command));
                        sprintf (command, "line %d,%d %d,%d %hX\n", vertexes[i].last_col_buf0, vertexes[i].last_row_buf0, vertexes[(i + 1)%num_xs].last_col_buf0, vertexes[(i + 1)%num_xs].last_row_buf0, 0x0);
                        write (video_FD, command, strlen(command));

                        sprintf (command, "box %d,%d %d,%d %hX\n", vertexes[i].last_col_buf1, vertexes[i].last_row_buf1, (vertexes[i].last_col_buf1 + box_size), (vertexes[i].last_row_buf1 + box_size), 0x0);
                        write (video_FD, command, strlen(command));
                        sprintf (command, "line %d,%d %d,%d %hX\n", vertexes[i].last_col_buf1, vertexes[i].last_row_buf1, vertexes[(i + 1)%num_xs].last_col_buf1, vertexes[(i + 1)%num_xs].last_row_buf1, 0x0);
                        write (video_FD, command, strlen(command));
                    }
                    num_xs++;
                }
                break;
            case 8: //KEY3: Decrease number of objects
                if (num_xs > 1){
                    // Save vertices' initial coordinates as "last" locations (used to clear previous drawing)
                    for ( i = 0; i < num_xs; i++){
                        if (!buffer_num){
                            vertexes[i].last_col_buf1 = vertexes[i].col;
                            vertexes[i].last_row_buf1 = vertexes[i].row;
                        }
                        else {
                            vertexes[i].last_col_buf0 = vertexes[i].col;
                            vertexes[i].last_row_buf0 = vertexes[i].row;
                        }
                    }
                    for (i = 0; i < (num_xs); i++) {

                        // Clear screen of what is stored in both buffers
                        sprintf (command, "box %d,%d %d,%d %hX\n", vertexes[i].last_col_buf0, vertexes[i].last_row_buf0, (vertexes[i].last_col_buf0 + box_size), (vertexes[i].last_row_buf0 + box_size), 0x0);
                        write (video_FD, command, strlen(command));
                        sprintf (command, "line %d,%d %d,%d %hX\n", vertexes[i].last_col_buf0, vertexes[i].last_row_buf0, vertexes[(i + 1)%num_xs].last_col_buf0, vertexes[(i + 1)%num_xs].last_row_buf0, 0x0);
                        write (video_FD, command, strlen(command));

                        sprintf (command, "box %d,%d %d,%d %hX\n", vertexes[i].last_col_buf1, vertexes[i].last_row_buf1, (vertexes[i].last_col_buf1 + box_size), (vertexes[i].last_row_buf1 + box_size), 0x0);
                        write (video_FD, command, strlen(command));
                        sprintf (command, "line %d,%d %d,%d %hX\n", vertexes[i].last_col_buf1, vertexes[i].last_row_buf1, vertexes[(i + 1)%num_xs].last_col_buf1, vertexes[(i + 1)%num_xs].last_row_buf1, 0x0);
                        write (video_FD, command, strlen(command));
                    }
                    num_xs--;
                }
                break;
            default:
                break;
        }

        // 4. Save current positions for other buffer + update positons
        for ( i = 0; i < num_xs; i++){
            if (buffer_num){
                vertexes[i].last_col_buf1 = vertexes[i].col;
                vertexes[i].last_row_buf1 = vertexes[i].row;
            }
            else {
                vertexes[i].last_col_buf0 = vertexes[i].col;
                vertexes[i].last_row_buf0 = vertexes[i].row;
            }
            move_vertex(&vertexes[i], step_add);
        }
        
    }


    // Clear both VGA pixel buffers
    for(a = 0; a < 2; a++){
        sprintf(command, "clear");
        write(video_FD, command, sizeof(command));
        sprintf(command, "sync");
        write(video_FD, command, sizeof(command));
    }

    //close all char driver files
    SW_close();
    KEY_close();
    close(video_FD);
    return 0;
}

void catchSIGINT(int signum) {
    stop = 1;
}

// Update row and col step variable to deal with screen boundaries
// in order to 'bounce' the vertex when it reaches a boundary
void set_steps(struct vertex* v, int step){

    //row boundary: 0 <= Y <= 239
    if (v -> row >= (screen_y - box_size) - 1 ) {
        v -> row_step = -1 * step;
    }
    else if (v ->row <= 0) {
        v -> row_step = 1 * step;
    }

    //col boundary: 0 <= X <= 319
    if (v->col >= (screen_x - box_size) - 1 ) {
        v->col_step = -1 * step;
    } 
    else if (v->col <= 0) {
        v->col_step = 1 * step;
    }
}

void move_vertex(struct vertex* v, int step){
    set_steps(v, step);
    v -> row += v -> row_step * step;
    v -> col += v -> col_step * step;
}
