#include <stdio.h>
#include "defines.h"

void plot_pixel(int, int, char, char);

int main(void)
{
    int i;
    printf ("\e[2J");                     // clear the screen
    printf ("\e[?25l");                   // hide the cursor

    plot_pixel (1, 1, CYAN, 'X');
    plot_pixel (80, 24, CYAN, 'X');
    for (i = 8; i < 18; ++i)
        plot_pixel (40, i, YELLOW, '*');

    (void) getchar ();                    // wait for user to press return
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
