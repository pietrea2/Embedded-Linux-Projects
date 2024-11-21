#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "ADXL345.h"
#include "accel_wrappers.h"

// Terminal Graphics Colours
#define BLACK 				30
#define RED 				31
#define GREEN 				32
#define YELLOW 			    33
#define BLUE 				34
#define MAGENTA 			35
#define CYAN 				36
#define WHITE 				37
// Constants for animation
#define MAX_X               40
#define MIN_X              -39
#define MAX_Y               12
#define MIN_Y              -11

#define AVERAGE_COUNT 2
int R, x, y, z, scale_factor;

volatile sig_atomic_t stop;

void catchSIGINT(int signum);
void plot_pixel(int x, int y, char color, char c);
void clear_screen(void);

int main(void){

    int read_from_char;
    int bubble_x, bubble_y;
    bubble_x = 0;
    bubble_y = 0;

    size_t c;
    int x_avg, y_avg, z_avg;
    float run_x_ave, run_y_ave, run_z_ave;
    float a = 0.5;

    // catch SIGINT from ctrl+c, instead of having it abruptly close this program
    signal(SIGINT, catchSIGINT);

    // Open the character device driver
    if (accel_open() == 0){
        printf("Error opening /dev/accel: %s\n", strerror(errno));
        return -1;
    }

    accel_init();
    accel_calibrate();

    clear_screen();
    printf("\e[?25l");   // hide the cursor
    plot_pixel(bubble_x, bubble_y, CYAN, '*');

    run_x_ave = 0;
    run_y_ave = 0;
    run_z_ave = 0;

    while (!stop) {

        x_avg = 0;
        y_avg = 0;
        z_avg = 0;

        // Calc average
        for (c = 0; c < AVERAGE_COUNT; c++) {
            read_from_char = accel_read(&R, &x, &y, &z, &scale_factor);
            if (!read_from_char) {
                continue;
            }
            else{
                x_avg += x;
                y_avg += y;
                z_avg += z;
            }
        }

        x_avg /= AVERAGE_COUNT;
        y_avg /= AVERAGE_COUNT;
        z_avg /= AVERAGE_COUNT;

        // Display average x, y, z values on screen
        clear_screen();
        printf("\e[H");
        printf("\e[37mX = %d mg, Y = %d mg, Z = %d mg", x_avg*scale_factor, y_avg*scale_factor, z_avg*scale_factor);

        // Calc Running Average (used for moving bubble)
        run_x_ave = run_x_ave * a + x_avg * (1 - a);
        run_y_ave = run_y_ave * a + y_avg * (1 - a);
        run_z_ave = run_z_ave * a + z_avg * (1 - a);

        // Moving bubble on screen (at its screen limits)
        if(run_x_ave < MAX_X && run_x_ave > MIN_X){
            bubble_x = run_x_ave;
        }
        else if(run_x_ave < MIN_X) bubble_x = MIN_X;
        else if(run_x_ave > MAX_X) bubble_x = MAX_X;

        if(run_y_ave < MAX_Y && run_y_ave > MIN_Y){
            bubble_y = run_y_ave;
        }
        else if(run_y_ave < MIN_Y) bubble_y = MIN_Y;
        else if(run_y_ave > MAX_Y) bubble_y = MAX_Y;

        plot_pixel(bubble_x, bubble_y, CYAN, '*');
        usleep(100000);
    }

    // Reset terminal settings
    printf("\e[2J\e[37m\e[?25h\e[H");
    fflush(stdout);
    
    accel_close();
    return 0;
}

void clear_screen(void){
    printf ("\e[2J");
}

void catchSIGINT(int signum){
    stop = 1;
}

void plot_pixel(int x, int y, char color, char c){
    /*
    \e[ccm:    set colour of text chars (cc = attribute)
    \e[yy;xxH : specify row:col on the screen to move cursor to (yy = row, xx = col)
    */
    printf ("\e[%2dm\e[%d;%dH%c", color, y + 12, x + 40, c);
    fflush (stdout);
}
