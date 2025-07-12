#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
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

    int current_minutes;
    int current_seconds;
    int current_miliseconds;
    int reset_minutes;
    int reset_seconds;
    int reset_miliseconds;

    int KEY_data;
    int SW_data;
    int LEDR_data;

    // math game variables
    int operand_1;
    int operand_2;
    int solution;
    int user_input;
    int increase_complexity_1 = 0;
    int increase_complexity_2 = 0;

    // stats
    int number_of_questions_asked;
    int correct_answers;
    float score;
    int ave_time_question;
    
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

    // pause stopwatch
    fputs("stop", stopwatch_fp);
    fflush(stopwatch_fp);

    // write default stopwatch time 
    current_minutes = 0;
    current_seconds = 20;
    current_miliseconds = 0;
    sprintf(stopwatch_command, "%d:%d:%d\n", current_minutes, current_seconds, current_miliseconds);
    fputs(stopwatch_command, stopwatch_fp);                               
    fflush(stopwatch_fp);

    // display stopwatch time on HEXs
    fputs("disp", stopwatch_fp);
    fflush(stopwatch_fp);

    printf("Set stopwatch if desired. Press KEY0 to start!\n");

    while (!stop) {

        KEY_read(&KEY_data);

        // KEY1 pressed = set DD part of stopwatch time
        if( KEY_data == 2 ){

            // read SW value
            SW_read(&SW_data);
            // read current stopwatch time
            while ( fgets(stopwatch_buffer, chardev_BYTES, stopwatch_fp) );  // read the stopwatch driver until EOF
            stopwatch_buffer[ strcspn(stopwatch_buffer, "\n") ] = 0;         // remove \n from string
            sscanf(stopwatch_buffer, "%d:%d:%d", &current_minutes,
                                                 &current_seconds, 
                                                 &current_miliseconds);     // store cur time in seperate vars

            if( SW_data <= 99 ){

                reset_miliseconds = SW_data;

                // write SW value to DD of stopwatch time
                sprintf(stopwatch_command, "%d:%d:%d\n", current_minutes,
                                                         current_seconds,
                                                         reset_miliseconds);
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

                reset_seconds = SW_data;

                // write SW value to DD of stopwatch time
                sprintf(stopwatch_command, "%d:%d:%d\n", current_minutes,
                                                         reset_seconds,
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

                reset_minutes = SW_data;

                // write SW value to DD of stopwatch time
                sprintf(stopwatch_command, "%d:%d:%d\n", reset_minutes,
                                                         current_seconds,
                                                         current_miliseconds);
                fputs(stopwatch_command, stopwatch_fp);                               
                fflush(stopwatch_fp);

                LEDR_set(SW_data);
            }
        }
        // if KEY0 is pressed, begin game!
        else if( KEY_data == 1 ){

            while ( fgets(stopwatch_buffer, chardev_BYTES, stopwatch_fp) );  // read the stopwatch driver until EOF
            stopwatch_buffer[ strcspn(stopwatch_buffer, "\n") ] = 0;         // remove \n from string
            sscanf(stopwatch_buffer, "%d:%d:%d", &reset_minutes, &reset_seconds, &reset_miliseconds);

            fputs("run", stopwatch_fp);
            fflush(stopwatch_fp);


            // generate 2 random operands
            operand_1 = rand() % 15;
            operand_2 = rand() % 15;

            // keep asking math questions until user runs out of time
            while( current_minutes != 0 || current_seconds != 0 || current_miliseconds != 0 ){

                // 1. calculate expected answer
                solution = operand_1 + operand_2;

                // 2. print questions to terminal
                printf("%d + %d = ", operand_1, operand_2);
                number_of_questions_asked += 1;
                //printf("\n%d: solution", solution);


                // 3. read from terminal
                scanf("%d", &user_input);

                //printf("%d: input", user_input);

                // 4. if correct answer, repeat and calculate data
                if( user_input == solution ){

                    correct_answers += 1;

                    // reset stopwatch
                    sprintf(stopwatch_command, "%d:%d:%d\n", reset_minutes, reset_seconds, reset_miliseconds);
                    fputs(stopwatch_command, stopwatch_fp);                               
                    fflush(stopwatch_fp);

                    // generate 2 random operands
                    increase_complexity_1 += 2;
                    increase_complexity_2 += 1;
                    operand_1 = ( rand() % (15+increase_complexity_1) ) + increase_complexity_2;
                    operand_2 = ( rand() % (15+increase_complexity_1) ) + increase_complexity_2;

                } else {
                    // if incorrect, generate new question but don't increment stopwatch
                    // generate 2 random operands

                    printf("Try again: %d \n", solution);
                    increase_complexity_1 += 2;
                    increase_complexity_2 += 1;
                    operand_1 = ( rand() % (15+increase_complexity_1) ) + increase_complexity_2;
                    operand_2 = ( rand() % (15+increase_complexity_1) ) + increase_complexity_2;
                }

                // read time from stopwatch
                while ( fgets(stopwatch_buffer, chardev_BYTES, stopwatch_fp) );  // read the stopwatch driver until EOF
                stopwatch_buffer[ strcspn(stopwatch_buffer, "\n") ] = 0;         // remove \n from string
                sscanf(stopwatch_buffer, "%d:%d:%d", &current_minutes, &current_seconds, &current_miliseconds);
            }

            score = ((float)correct_answers / (float)number_of_questions_asked) * 100.0;

            printf("Time Expired! ");
            printf("You answered %d questions correctly out of %d. Your score is %.2f %%.\n", correct_answers, number_of_questions_asked, score);
            
            stop = 1;

        }

        sleep(0.1);

    }

    // turn off LEDRs 
    LEDR_set(0);

    //close all char driver files
    LEDR_close();
    SW_close();
    KEY_close();
    fclose(stopwatch_fp);

    return 0;
}