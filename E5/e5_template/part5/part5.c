#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "defines.h"
#include <sys/mman.h>

/*
**************************************************************
Memory-mapped I/O variables and functions
For reading KEY and SW
*/
#define LW_BRIDGE_BASE 0xFF200000
#define SW_BASE 0x00000040
#define KEY_BASE 0x00000050
#define LW_BRIDGE_SPAN 5000

int open_physical(int);
void *map_physical(int, unsigned int, unsigned int);
void close_physical(int);
int unmap_physical(void *, unsigned int);
//************************************************************

struct vertex
{
    int row;
    int col;
    int col_step;
    int row_step;
    int colour;
};

volatile sig_atomic_t stop;

void catchSIGINT(int signum);
void set_steps(struct vertex* v);
void move_vertex(struct vertex* v);
void plot_pixel(int, int, char, char);
void draw_line(int x0, int x1, int y0, int y1, char color, char c);



int main(void)
{
    // declare and set up address pointers to KEY and SW ------------------------------
    volatile int *KEY_ptr;
    volatile int *SW_ptr;
    int fd = -1;
    void *LW_virtual;

    if ((fd = open_physical(fd)) == -1)
        return (-1);
    else if ((LW_virtual = map_physical(fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN)) == NULL)
        return (-1);

    KEY_ptr = (unsigned int *)(LW_virtual + KEY_BASE);
    SW_ptr = (unsigned int *)(LW_virtual + SW_BASE);
    *(KEY_ptr + 3) = 0xF; // clear edgecapture register for KEYs
    //---------------------------------------------------------------------------------



    // animation delay variable and seed random function
    srand(time(NULL));
    struct timespec ts;
    ts.tv_sec = 0;          // used to delay
    ts.tv_nsec = 100000000; // 1 * 10^8 ns = 0.1 sec

    // init vertex variables
    struct vertex vertexes[NUM_XS_MAX];
    int num_xs = NUM_XS;
    size_t i;

    for (i = 0; i < NUM_XS_MAX; i++)
    {
        vertexes[i].row = rand() % SCREEN_Y + 1;
        vertexes[i].col = rand() % SCREEN_X + 1;
        vertexes[i].col_step = rand() % 2 ? 1 : -1;
        vertexes[i].row_step = rand() % 2 ? 1 : -1;
        vertexes[i].colour = rand() % 7 + 31;
    }

    // catch SIGINT from ctrl+c, instead of having it abruptly close this program
    signal(SIGINT, catchSIGINT);

    printf("\e[2J");   // clear the screen
    printf("\e[?25l"); // hide the cursor


    //begin animation
    while (!stop)
    {
        //read KEY and perform following animation features
        switch (*(KEY_ptr + 3))
        {
            case 1: //KEY0: Increase speed of animation
                if (ts.tv_nsec > 100000000 / 2)
                    ts.tv_nsec *= 0.9;
                break;
            case 2: //KEY1: Decrease speed of animation
                if (ts.tv_nsec < 100000000 * 2)
                    ts.tv_nsec *= 1.2;
                break;
            case 4: //KEY2: Increase number of objects
                if (num_xs < 26)
                    num_xs++;
                break;
            case 8: //KEY3: Decrease number of objects
                if (num_xs > 5)
                    num_xs--;
                break;
            default:
                break;
        }

        *(KEY_ptr + 3) = 0xF; // clear edgecapture register for KEYs
        printf("\e[2J");      // clear the screen



        //1. Draw objects
        for (i = 0; i < num_xs; i++)
        {
            plot_pixel(vertexes[i].col, vertexes[i].row, vertexes[i].colour, 'X');
        }

        //2. Draw lines (if SW is not on)
        if (!*(SW_ptr))
            for (i = 0; i < num_xs; i++)
            {
                if (i == num_xs - 1)
                {
                    draw_line(vertexes[i].col, vertexes[0].col, vertexes[i].row, vertexes[0].row, vertexes[i].colour, '*');
                }
                else
                {
                    draw_line(vertexes[i].col, vertexes[i + 1].col, vertexes[i].row, vertexes[i + 1].row, vertexes[i].colour, '*');
                }
            }

        //3. Move objects
        for (i = 0; i < num_xs; i++)
        {
            move_vertex(&vertexes[i]);
        }

        nanosleep(&ts, NULL); // added shifting delay
    }

    printf("\e[2J");           // clear the screen
    printf("\e[%2dm", WHITE);  // reset foreground color
    printf("\e[%d;%dH", 1, 1); // move cursor to upper left
    printf("\e[?25h");         // show the cursor
    fflush(stdout);
    return (0);
}

void catchSIGINT(int signum)
{
    stop = 1;
}

//update row and col step variable to deal with screen boundaries
//in order to 'bounce' the vertex when it reaches a boundary
void set_steps(struct vertex *v)
{
    //row boundary: 1 <= Y <= 24
    if (v->row >= SCREEN_Y + 1){
        v->row_step = -1;
        v->row = SCREEN_Y - 1;
    }
    else if (v->row == 0){
        v->row_step = 1;
        v->row = 2;
    }

    //col boundary: 1<= X <= 80
    if (v->col >= SCREEN_X + 1){
        v->col_step = -1;
        v->col = SCREEN_X - 1;
    }
    else if (v->col == 0){
        v->col_step = 1;
        v->col = 2;
    }
}

void move_vertex(struct vertex *v)
{
    v->row += v->row_step;
    v->col += v->col_step;
    set_steps(v);
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

//Function to swap 2 variable values (in draw_line function)
void swap(int *a, int *b)
{

    int swap_var;

    swap_var = *a;
    *a = *b;
    *b = swap_var;
}

//Bresenham’s line-drawing algorithm
void draw_line(int x0, int x1, int y0, int y1, char color, char c)
{

    int is_steep = ABS(y1 - y0) > ABS(x1 - x0);

    if (is_steep)
    {
        swap(&x0, &y0);
        swap(&x1, &y1);
        c = '|';
    }
    if (x0 > x1)
    {
        swap(&x0, &x1);
        swap(&y0, &y1);
    }

    //define vars used for Bresenham's algorithm
    int deltax = x1 - x0;
    int deltay = ABS(y1 - y0);
    int error = -(deltax / 2);
    int y = y0;
    int x;

    if(deltay <= 1) c = '-';

    //calc if line has positive or negative slope
    int y_step;
    if (y0 < y1)
        y_step = 1;
    else
        y_step = -1;

    //main for loop for drawing algorithm
    for (x = x0; x <= x1; x++)
    {
        if ((x == x0 && y == y0) || (x == x1 && y == y1))
            continue;
        if (is_steep)
            plot_pixel(y, x, color, c);
        else
            plot_pixel(x, y, color, c);

        //calc error again
        //if greater than 0, draw pixel with y coordinate updated
        //if not, keep drawing pixels at same y coordinate
        error = error + deltay;

        if (error > 0)
        {
            y = y + y_step;
            error = error - deltax;
        }
    }
}

// Open /dev/mem, if not already done, to give access to physical addresses
int open_physical(int fd)
{
    if (fd == -1)
        if ((fd = open("/dev/mem", (O_RDWR | O_SYNC))) == -1)
        {
            printf("ERROR: could not open \"/dev/mem\"...\n");
            return (-1);
        }
    return fd;
}

// Close /dev/mem to give access to physical addresses
void close_physical(int fd)
{
    close(fd);
}

/*
 * Establish a virtual address mapping for the physical addresses starting at base, and
 * extending by span bytes.
 */
void *map_physical(int fd, unsigned int base, unsigned int span)
{
    void *virtual_base;

    // Get a mapping from physical addresses to virtual addresses
    virtual_base = mmap(NULL, span, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, base);
    if (virtual_base == MAP_FAILED)
    {
        printf("ERROR: mmap() failed...\n");
        close(fd);
        return (NULL);
    }
    return virtual_base;
}

/*
 * Close the previously-opened virtual address mapping
 */
int unmap_physical(void *virtual_base, unsigned int span)
{
    if (munmap(virtual_base, span) != 0)
    {
        printf("ERROR: munmap() failed...\n");
        return (-1);
    }
    return 0;
}