#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "defines.h"

void plot_pixel(int, int, char, char);
void draw_line(int x0, int x1, int y0, int y1, char color, char c);

struct vertex {
    int row;
    int col;
    int col_step;
    int row_step;
};

volatile sig_atomic_t stop;
void catchSIGINT(int signum)
{
    stop = 1;
}

void set_steps(struct vertex* v){
        if (v -> row == 25)
        {
            v -> row_step = -1;
            v ->row = 23;
        }
        else if (v ->row == 0)
        {
            v -> row_step = 1;
            v ->row = 2;
        }
        if (v->col == 60) {
            v->col_step = -1;
            v->col = 59;
        } else if (v->col == 0) {
            v->col_step = 1;
            v->col = 1;
        }
}

void move_vertex(struct vertex* v){
        v -> row += v -> row_step;
        v -> col += v -> col_step;
        set_steps(v);
}

int main(void)
{
    struct timespec ts;
    ts.tv_sec = 0;          // used to delay
    ts.tv_nsec = 100000000; // 1 * 10^8 ns = 0.1 sec

    // int row = 1;
    // int row_step = 1;
    // int col = 1;
    // int col_step = 1;
    struct vertex v1 = {
        .row = 1,
        .col = 1,
        .col_step = 1,
        .row_step = 1
    };
    struct vertex v2 = {
        .row = 13,
        .col = 17,
        .col_step = -1,
        .row_step = -1
    };
    struct vertex v3 = {
        .row = 12,
        .col = 13,
        .col_step = 1,
        .row_step = -1
    };
    struct vertex v4 = {
        .row = 9,
        .col = 5,
        .col_step = -1,
        .row_step = 1
    };

    // catch SIGINT from ctrl+c, instead of having it abruptly close this program
    signal(SIGINT, catchSIGINT);

    printf("\e[2J");   // clear the screen
    printf("\e[?25l"); // hide the cursor
    while (!stop)
    {

        printf("\e[2J");   // clear the screen
        plot_pixel(v1.col, v1.row, RED, 'X');
        plot_pixel(v2.col, v2.row, BLUE, 'X');
        plot_pixel(v3.col, v3.row, GREEN, 'X');
        plot_pixel(v4.col, v4.row, WHITE, 'X');
        draw_line(v1.col, v2.col, v1.row, v2.row, YELLOW, '*');    //draw line from v1 to v2
        draw_line(v1.col, v3.col, v1.row, v3.row, YELLOW, '*');    //draw line from v1 to v3 
        draw_line(v2.col, v4.col, v2.row, v4.row, YELLOW, '*');    //draw line from v2 to v4
        draw_line(v3.col, v4.col, v3.row, v4.row, YELLOW, '*');    //draw line from v3 to v4
        move_vertex(&v1);
        move_vertex(&v2);
        move_vertex(&v3);
        move_vertex(&v4);

        nanosleep(&ts, NULL); // added shifting delay
    }

    printf("\e[2J");           // clear the screen
    printf("\e[%2dm", WHITE);  // reset foreground color
    printf("\e[%d;%dH", 1, 1); // move cursor to upper left
    printf("\e[?25h");         // show the cursor
    fflush(stdout);
    return (0);
}

void plot_pixel(int x, int y, char color, char c)
{
    /*
    \e[ccm:    set colour of text chars (cc = attribute)
    \e[yy;xxH : specify row:col on the screen to move cursor to (yy = row, xx = col)
    */
    printf("\e[%2dm\e[%d;%dH%c", color, y, x, c);
    fflush(stdout);
}

void swap(int *a, int *b)
{

    int swap_var;

    swap_var = *a;
    *a = *b;
    *b = swap_var;
}

// Bresenham’s algorithm
void draw_line(int x0, int x1, int y0, int y1, char color, char c)
{

    int is_steep = ABS(y1 - y0) > ABS(x1 - x0);

    if (is_steep)
    {
        // swap x0, y0
        swap(&x0, &y0);
        // swap x1, y1
        swap(&x1, &y1);
    }
    if (x0 > x1)
    {
        swap(&x0, &x1);
        swap(&y0, &y1);
    }

    int deltax = x1 - x0;
    int deltay = ABS(y1 - y0);
    int error = -(deltax / 2);
    int y = y0;
    int x;

    int y_step;
    if (y0 < y1)
        y_step = 1;
    else
        y_step = -1;

    for (x = x0; x <= x1; x++)
    {
        if ((x == x0 && y == y0) || (x == x1 && y == y1))
            continue;
        if (is_steep )
            plot_pixel(y, x, color, c);
        else
            plot_pixel(x, y, color, c);

        error = error + deltay;

        if (error > 0)
        {
            y = y + y_step;
            error = error - deltax;
        }
    }
}
