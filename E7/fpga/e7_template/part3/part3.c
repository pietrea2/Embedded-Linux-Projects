#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "ADXL345.h"
#include "accel_wrappers.h"

#define accel_BYTES 20
#define MAX_X 10
#define MAX_Y 10
#define AVERAGE_COUNT 10
int R, x, y, z, scale_factor;
void print_board(int, int);
void clear_screen(void);
/**             your part 2 user code here                   **/
/**  hint: you can call functions from ../accel_wrappers.c   **/
int main(void)
{

    int accel_FD;             // file descriptor
    char buffer[accel_BYTES]; // buffer for data read from /dev/accel
    int read_from_char;
    int bubble_x, bubble_y;
    size_t c;
    int x_avg, y_avg;
    bubble_x = 0;
    bubble_y = 0;
    // Open the character device driver
    if (accel_open() == 0)
    {
        printf("Error opening /dev/accel: %s\n", strerror(errno));
        return -1;
    }
    accel_init();
    accel_calibrate();
    
    while (1)
    {
        x_avg = 0;
        y_avg = 0;
        for (c = 0; c < AVERAGE_COUNT; c++)
        {

            read_from_char = accel_read(&R, &x, &y, &z, &scale_factor);
            if (!read_from_char)
            {
                // printf("Error reading from /dev/accel: %s\n", strerror(errno));
                // return -1;
            }
            else
            {
                // printf("%d %d\n", x, y);
                x_avg += x;
                y_avg += y;
            }
        }
        x_avg /= AVERAGE_COUNT;
        y_avg /= AVERAGE_COUNT;
        if (x_avg > 5 && bubble_x < MAX_X)
        {
            bubble_x++;
        }
        else if (x_avg < -5 && bubble_x > -MAX_X)
        {
            bubble_x--;
        }
        if (y_avg > 5 && bubble_y < MAX_Y)
        {
            bubble_y++;
        }
        else if (y_avg < -5 && bubble_y > -MAX_Y)
        {
            bubble_y--;
        }
        clear_screen();
        print_board(bubble_y, bubble_x);
        usleep(1000000);
        /*
        read(accel_FD, buffer, sizeof(buffer));
        sscanf(buffer,"%d %d %d %d %d", &R, &x, &y, &z, &scale_factor);
        if(R == 1) printf("X = %d, Y = %d, Z = %d\n", x*scale_factor, y*scale_factor, z*scale_factor);
        */
    }

    close(accel_FD);
    return 0;
}

void clear_screen(void)
{
    printf("\033[2J"); // Clear screen
    printf("\033[H");  // Move cursor to home position
}

void print_board(int x, int y)
{
    int i, j;
    for (i = -MAX_X; i < MAX_X; i++)
    {
        for (j = -MAX_Y; j < MAX_Y; j++)
        {
            printf("%c", (i == x && j == y) ? '*' : ' ');
        }
        printf("\n");
    }
}
