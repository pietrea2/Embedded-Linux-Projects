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

#define accel_BYTES 20
#define MAX_X 10
#define MAX_Y 10
#define AVERAGE_COUNT 10
int  x, y, z, scale_factor;
void print_board(int, int, char, int, int);
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
    volatile uint8_t tap, double_tap;
    volatile uint8_t fg_color,bg_color;
    volatile char shown;
    char dummy[10];
    uint8_t intrupt;
    shown = '*';
    bubble_x = 0;
    bubble_y = 0;
    fg_color = 0;
    bg_color = 1;
    // Open the character device driver
    if (accel_open() == 0)
    {
        printf("Error opening /dev/accel: %s\n", strerror(errno));
        return -1;
    }
    srand(time(NULL));
    
    while (1)
    {
        x_avg = 0;
        y_avg = 0;
        tap = 0;
        double_tap = 0;
        for (c = 0; c < AVERAGE_COUNT; c++)
        {

            read_from_char = accel_read(&intrupt, &x, &y, &z, &scale_factor);
            sprintf(dummy, "%d", intrupt); // Stores hex string in
            sscanf(dummy, "%hhX", &intrupt);
            if (!read_from_char)
            {
                // printf("Error reading from /dev/accel: %s\n", strerror(errno));
                // return -1;
            }
            else
            {
                
                if (intrupt & XL345_SINGLETAP)
                    tap = XL345_SINGLETAP;
                if (intrupt & XL345_DOUBLETAP)
                    double_tap = XL345_DOUBLETAP;
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
        if (tap){
            // printf("tap : %hhX\n",tap);
            shown = (char)(32 + rand() % 95);
        }
        if (double_tap) {
            fg_color = rand()%7;
            bg_color = 7 - fg_color;
        }
            
        // printf("shown char : %c\n",shown);
        clear_screen();
        print_board(bubble_y, bubble_x, shown, fg_color, bg_color);
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

void print_board(int x, int y, char shown, int fg_color, int bg_color)
{
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
