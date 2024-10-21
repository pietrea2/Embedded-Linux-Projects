#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <intelfpgaup/KEY.h>
#include <intelfpgaup/SW.h>
#include <intelfpgaup/LEDR.h>

#define chardev_BYTES 256 // max number of characters to read from /dev/SW or KEY

volatile sig_atomic_t stop;
void catchSIGINT(int signum)
{
    stop = 1;
}

int main(int argc, char *argv[])
{
    FILE *stopwatch_fp;                   // Stopwatch file pointer

    int stopwatch_run = 1;
    char stopwatch_command[chardev_BYTES];
    char stopwatch_buffer[chardev_BYTES];
    volatile int miliseconds;
    volatile int miliseconds_2;
    volatile int seconds;
    volatile int seconds_2;
    volatile int minutes;
    volatile int minutes_2;
    volatile int ones_column;
    volatile int tens_column;
    int current_minutes;
    int current_seconds;
    int current_miliseconds;

    int KEY_data;
    int SW_data;
    int LEDR_data;
    
    // catch SIGINT from ctrl+c, instead of having it abruptly close this program
    signal(SIGINT, catchSIGINT);
    
    if ( !LEDR_open() )
    {
        printf("Error opening /dev/IntelFPGAUP/LEDR: %s\n", strerror(errno));
        return -1;
    }
    if ( !SW_open() )
    {
        printf("Error opening /dev/IntelFPGAUP/SW: %s\n", strerror(errno));
        return -1;
    }
    if ( !KEY_open() )
    {
        printf("Error opening /dev/IntelFPGAUP/KEY: %s\n", strerror(errno));
        return -1;
    }
    if ((stopwatch_fp = fopen("/dev/stopwatch", "r+")) == NULL)
    {
        printf("Error opening /dev/stopwatch: %s\n", strerror(errno));
        return -1;
    }

    fputs("disp", stopwatch_fp);
    fflush(stopwatch_fp);

    while (!stop) {

        KEY_read(&KEY_data);

        // if KEY0 is pressed, toggle stopwatch between run and pause state
        if( KEY_data == 1 ){

            // toggle
            if(stopwatch_run){
                fputs("stop", stopwatch_fp);               // write sw_buffer to LEDR driver to display SW value
                fflush(stopwatch_fp);
                stopwatch_run = 0;
            }
            else{
                fputs("run", stopwatch_fp);               // write sw_buffer to LEDR driver to display SW value
                fflush(stopwatch_fp);
                stopwatch_run = 1;
            }


        }
        // KEY1 pressed = set DD part of stopwatch time
        else if( KEY_data == 2 ){

            // read SW value
            SW_read(&SW_data);
            // read current stopwatch time
            while ( fgets(stopwatch_buffer, chardev_BYTES, stopwatch_fp) );  // read the stopwatch driver until EOF
            stopwatch_buffer[ strcspn(stopwatch_buffer, "\n") ] = 0;         // remove \n from string
            sscanf(stopwatch_buffer, "%d:%d:%d", &current_minutes,
                                                 &current_seconds, 
                                                 &current_miliseconds);     // store cur time in seperate vars


            if( SW_data <= 99 ){

                current_miliseconds = SW_data;

                // write SW value to DD of stopwatch time
                sprintf(stopwatch_command, "%d:%d:%d\n", current_minutes,
                                                       current_seconds,
                                                       current_miliseconds);
                fputs(stopwatch_command, stopwatch_fp);                               
                fflush(stopwatch_fp);

                LEDR_set(SW_data);

            }


        }
        // KEY2 pressed = set SS part of stopwatch time
        else if( KEY_data == 4 ){

            // read SW value
            SW_read(&SW_data);
            // read current stopwatch time
            while ( fgets(stopwatch_buffer, chardev_BYTES, stopwatch_fp) );  // read the stopwatch driver until EOF
            stopwatch_buffer[ strcspn(stopwatch_buffer, "\n") ] = 0;         // remove \n from string
            sscanf(stopwatch_buffer, "%d:%d:%d", &current_minutes,
                                                 &current_seconds, 
                                                 &current_miliseconds);     // store cur time in seperate vars


            if( SW_data <= 59 ){

                current_seconds = SW_data;

                // write SW value to DD of stopwatch time
                sprintf(stopwatch_command, "%d:%d:%d\n", current_minutes,
                                                       current_seconds,
                                                       current_miliseconds);
                fputs(stopwatch_command, stopwatch_fp);                               
                fflush(stopwatch_fp);

                LEDR_set(SW_data);

            }
        }
        // KEY3 pressed = set MM part of stopwatch time
        else if( KEY_data == 8 ){
            
            // read SW value
            SW_read(&SW_data);
            // read current stopwatch time
            while ( fgets(stopwatch_buffer, chardev_BYTES, stopwatch_fp) );  // read the stopwatch driver until EOF
            stopwatch_buffer[ strcspn(stopwatch_buffer, "\n") ] = 0;         // remove \n from string
            sscanf(stopwatch_buffer, "%d:%d:%d", &current_minutes,
                                                 &current_seconds, 
                                                 &current_miliseconds);     // store cur time in seperate vars


            if( SW_data <= 59 ){

                current_minutes = SW_data;

                // write SW value to DD of stopwatch time
                sprintf(stopwatch_command, "%d:%d:%d\n", current_minutes,
                                                       current_seconds,
                                                       current_miliseconds);
                fputs(stopwatch_command, stopwatch_fp);                               
                fflush(stopwatch_fp);

                LEDR_set(SW_data);

            }
        }

        sleep(0.1);

    }



    //close all char driver files
    LEDR_close();
    SW_close();
    KEY_close();
    fclose(stopwatch_fp);

    return 0;
}
