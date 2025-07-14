#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "ADXL345.h"
#include "accel_wrappers.h"

// Constants for animation
#define MAX_X               40
#define MIN_X              -39
#define MAX_Y               12
#define MIN_Y              -11

#define AVERAGE_COUNT 2
int  R, tap, double_tap, x, y, z, scale_factor;

volatile sig_atomic_t stop;

void plot_pixel(int x, int y, char color, char c);
void print_board(int, int, char, int, int);
void clear_screen(void);
void catchSIGINT(int signum);

int main(void){

    int read_from_char;
    int bubble_x, bubble_y;
    bubble_x = 0;
    bubble_y = 0;

    size_t c;
    int x_avg, y_avg, z_avg;
    float run_x_ave, run_y_ave, run_z_ave;
    run_x_ave = 0;
    run_y_ave = 0;
    run_z_ave = 0;
    float a = 0.5;

    volatile uint8_t tap_flag, double_tap_flag;
    volatile uint8_t fg_color, bg_color;
    volatile char shown = '*';    
    fg_color = 37;
    bg_color = 1;


    // Open the character device driver (it is initialized and calibrated on open)
    if (accel_open() == 0){
        printf("Error opening /dev/accel: %s\n", strerror(errno));
        return -1;
    }

    signal(SIGINT, catchSIGINT);
    srand(time(NULL));

    clear_screen();
    printf("\e[?25l");   // hide the cursor
    plot_pixel(bubble_x, bubble_y, fg_color, shown);
    
    while (!stop) {

        x_avg = 0;
        y_avg = 0;
        z_avg = 0;
        tap_flag = 0;
        double_tap_flag = 0;

        for (c = 0; c < AVERAGE_COUNT; c++) {
            read_from_char = accel_read_with_taps(&R, &tap, &double_tap, &x, &y, &z, &scale_factor);
            //printf("%d, %d\n", tap, double_tap);
            if (!read_from_char) {
                continue;
            }
            else{
                x_avg += x;
                y_avg += y;
                z_avg += z;

                if (tap) tap_flag = XL345_SINGLETAP;
                if (double_tap) double_tap_flag = XL345_DOUBLETAP;
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

        if (double_tap_flag) {                          // change colour of bubble when double tab
            fg_color = rand()%7 + 31;
            bg_color = 7 - fg_color;
        }
        else if (tap_flag){                             // toggle bubble between * and O when tap
            //shown = (char)(32 + rand() % 95);
            if(shown == '*') shown = 'O';
            else if(shown == 'O') shown = '*';
        }
        
            
        plot_pixel(bubble_x, bubble_y, fg_color, shown);
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

void print_board(int x, int y, char shown, int fg_color, int bg_color){
    int i, j;
    for (i = -MAX_X; i < MAX_X; i++)
    {
        for (j = -MAX_Y; j < MAX_Y; j++)
        {
            if (i == x && j == y) {
                printf("\033[48;5;%dm\033[38;5;%dm%c\033[0m", bg_color, fg_color, shown);
            } else {
                printf("\033[48;5;%dm\033[38;5;%dm%c\033[0m", bg_color, bg_color, ' ');
            }
        }
        printf("\n");
    }
}