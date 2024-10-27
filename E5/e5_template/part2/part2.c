#include <stdio.h>
#include "defines.h"

void plot_pixel(int, int, char, char);
void draw_line(int x0, int x1, int y0, int y1, char color, char c);

int main(void)
{
    printf ("\e[2J");                     // clear the screen
    printf ("\e[?25l");                   // hide the cursor

    plot_pixel (1, 1, CYAN, 'X');
    plot_pixel (80, 24, CYAN, 'X');
    /*
    for (i = 8; i < 18; ++i)
        plot_pixel (40, i, YELLOW, '*');
    */

   draw_line(1, 80, 1, 1, WHITE, '*');
   draw_line(1, 80, 24, 24, WHITE, '*');
   draw_line(1, 80, 24, 1, GREEN, '*');
   draw_line(1, 20, 24, 1, MAGENTA, '|');
   draw_line(1, 1, 24, 1, RED, '|');
   draw_line(80, 1, 24, 5, BLUE, '*');
   draw_line(1, 80, 24, 5, BLUE, '*');

    (void) getchar ();                    // wait for user to press return
    printf ("\e[2J");                     // clear the screen
    printf ("\e[%2dm", WHITE);            // reset foreground color
    printf ("\e[%d;%dH", 1, 1);           // move cursor to upper left
    printf("\e[37m");
    printf ("\e[?25h");                   // show the cursor

    /*
    int a = 1;
    int b = 2;
    printf("A = %d B = %d", a, b);
    swap(&a, &b);
    printf("A = %d B = %d", a, b);
    */

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
