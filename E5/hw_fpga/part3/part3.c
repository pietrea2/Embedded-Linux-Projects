#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "defines.h"

void plot_pixel(int, int, char, char);
void draw_line(int x0, int x1, int y0, int y1, char color, char c);

volatile sig_atomic_t stop;
void catchSIGINT(int signum)
{
    stop = 1;
}

int main(void)
{
    struct timespec ts;
    ts.tv_sec = 0;					// used to delay
    ts.tv_nsec = 100000000;			// 1 * 10^8 ns = 0.1 sec

    int row = 1;
    int row_step = 1;

    // catch SIGINT from ctrl+c, instead of having it abruptly close this program
    signal(SIGINT, catchSIGINT);

    printf ("\e[2J");                     // clear the screen
    printf ("\e[?25l");                   // hide the cursor

    while (!stop) {
        
        printf ("\e[2J");      // clear the screen
        draw_line(20, 60, row, row, GREEN, '*');    //draw line

        row += row_step;       

        if (row == 25){ row_step = -1; row = 23; }
        else if (row == 0){ row_step = 1; row = 2; }

        nanosleep (&ts, NULL);    //added shifting delay

    }

    printf ("\e[2J");                     // clear the screen
    printf ("\e[%2dm", WHITE);            // reset foreground color
    printf ("\e[%d;%dH", 1, 1);           // move cursor to upper left
    printf ("\e[?25h");                   // show the cursor
    fflush (stdout);
    return (0);
}

void plot_pixel(int x, int y, char color, char c)
{
    /*
    \e[ccm:    set colour of text chars (cc = attribute)
    \e[yy;xxH : specify row:col on the screen to move cursor to (yy = row, xx = col)
    */
    printf ("\e[%2dm\e[%d;%dH%c", color, y, x, c);
    fflush (stdout);
}

void swap(int * a, int * b){

    int swap_var;

    swap_var = *a;
    *a = *b;
    *b = swap_var;
}

//Bresenham’s algorithm
void draw_line(int x0, int x1, int y0, int y1, char color, char c){

    int is_steep = ABS(y1 - y0) > ABS(x1 - x0);

    if(is_steep){
        //swap x0, y0
        swap(&x0, &y0);
        //swap x1, y1
        swap(&x1, &y1);
    }
    if(x0 > x1){
        swap(&x0, &x1);
        swap(&y0, &y1);
    }

    int deltax = x1 - x0;
    int deltay = ABS(y1 - y0);
    int error = - (deltax / 2);
    int y = y0;
    int x;

    int y_step;
    if(y0 < y1) y_step = 1;
    else y_step = -1;

    for(x = x0; x <= x1; x++){
        if(is_steep) plot_pixel(y, x, color, c);
        else plot_pixel(x, y, color, c);

        error = error + deltay;

        if(error > 0){
            y = y + y_step;
            error = error - deltax;
        }
    }
}
