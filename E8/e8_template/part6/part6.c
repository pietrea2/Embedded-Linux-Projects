#define _GNU_SOURCE
#include <sched.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <linux/input.h>
#include <pthread.h>
#include "address_map_arm.h"
#include "defines.h"
#include "stopwatch.h"
#include "video.h"
#include "KEY.h"
#include "HEX.h"
#include "LEDR.h"
#include "audio.h"


#define KEY_RELEASED 0
#define KEY_PRESSED 1
#define PLAYING_TIME 2000
#define video_ORANGE 0xFC00

struct recorded_note {
    int note;
    int time_sec;
    int time_mili_sec;
    int press_type;
};

void calc_time_passed(int sec0, int mili0, int sec1, int mili1, int * sec_passed, int * mili_passed);
struct recorded_note recorded_note_array[100];
int rec_num_count;
static const char *const press_type[2] = {"RELEASED", "PRESSED "};
char *Note[] = {"C#", "D#", " ", "F#", "G#", "A#", "C", "D", "E", "F", "G", "A", "B", "C"};
char ASCII[] = {'2',  '3',  '4', '5',  '6',  '7',  'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I'};

static volatile sig_atomic_t stop;
void catchSIGINT (int signum);

static pthread_t tid1, tid2;			// thread IDs
int set_processor_affinity(unsigned int core);
pthread_mutex_t mutex_tone_volume;
double scale[13] = {MIDC, DFLAT, DNAT, EFLAT, ENAT, FNAT, GFLAT, GNAT, AFLAT, ANAT, BFLAT, BNAT, HIC};
double note_vol[13] = {0};
double note_vol_rec[13] = {0};
int draw_waves;



void video_thread(){

    set_processor_affinity(0);

    int columns, rows, text_cols, text_rows, x, b;
    double wave_sum1, wave_sum2;

    if( !video_read(&columns, &rows, &text_cols, &text_rows) ) return printf("Error reading from VGA");
    //printf("VGA columns: %d, Rows: %d", columns, rows);

    while(!stop){

        pthread_testcancel();

        if(draw_waves){

            draw_waves = 0;     // Reset

            // Draw
            video_clear();
            for(x = 0; x < columns-1; x++){

                wave_sum1 = 0;
                wave_sum2 = 0;

                pthread_mutex_lock(&mutex_tone_volume);

                for (b = 0; b < 13; b++){
                    wave_sum1 += note_vol[b] * sin(x * scale[b]);
                    wave_sum2 += note_vol[b] * sin((x+1) * scale[b]);
                }

                pthread_mutex_unlock(&mutex_tone_volume);

                video_line(x, (int)(wave_sum1*20) + (rows/2), x+1, (int)(wave_sum2*20) + (rows/2), video_ORANGE);
            }
            video_show();
            
        }
    }
}




void audio_thread(){

    set_processor_affinity(1);

    size_t i;
    double freq_sum = 0;
    int write;
    int nth_sample = 0;

    while (!stop){

        for (nth_sample = 0; nth_sample < (SAMPLING_RATE * PLAYING_TIME / 1000) && !stop; nth_sample++){

            pthread_testcancel();
            write = 0;

            while (write == 0 && !stop) {

                audio_wait_write();
                    
                freq_sum = 0;

                pthread_mutex_lock(&mutex_tone_volume);
                for (i = 0; i < 13; i++){
                    freq_sum += MAX_VOLUME/13 * note_vol[i] * sin(nth_sample * scale[i]);
                    note_vol[i] /= 1.0001;
                }
                pthread_mutex_unlock(&mutex_tone_volume);

                audio_write_left( (int)(freq_sum) );
                audio_write_right( (int)(freq_sum) );
                write = 1;
                
            }
        }
    }
}




int main(int argc, char *argv[]){

    struct input_event ev;
    int ffd, event_size = sizeof(struct input_event), key;
    int err;
    draw_waves = 0;
    int key_pressed;
    int key_0_press = 0;
    int recording = 0;
    int minutes, seconds, miliseconds;

    stop = 0;
    signal(SIGINT, catchSIGINT);
    
    if ( !video_open() ){
        printf("Error opening /dev/IntelFPGAUP/video\n");
        return -1;
    }
    else if ( !audio_open() ){
        printf("Error opening /dev/IntelFPGAUP/audio\n");
        return -1;
    }
    else if ( !stopwatch_open() ){
        printf("Error opening stopwatch\n");
        return -1;
    }
    else if ( !LEDR_open() ){
        printf("Error opening stopwatch\n");
        return -1;
    }
    else if ( !KEY_open() ){
        printf("Error opening stopwatch\n");
        return -1;
    }

    // Initialize drivers
    audio_rate(SAMPLING_RATE);
    audio_init();


    stopwatch_stop();
    stopwatch_nodisplay();
    LEDR_set(0);
    
    video_clear();
    video_show();
    sleep(0.5);
    video_clear();
    video_show();

    if ((err = pthread_create(&tid1, NULL, &audio_thread, NULL)) != 0)
        printf("pthread_create failed:[%s]\n", strerror(err));
    if ((err = pthread_create(&tid2, NULL, &video_thread, NULL)) != 0)
        printf("pthread_create failed:[%s]\n", strerror(err));

    set_processor_affinity(0);

    // Get the keyboard device
    if (argv[1] == NULL){
        printf("Specify the path to the keyboard device ex./dev/input/by-id/HP-KEYBOARD");
        return -1;
    }
    if ((ffd = open(argv[1], O_RDONLY | O_NONBLOCK)) == -1){    // Open keyboard device
        printf("Could not open %s\n", argv[1]);                 // Testing using:
        return -1;                                              // /dev/input/by-id/usb-CHICONY_HP_USB_Multimedia_Keyboard-event-kbd
    }



    while (!stop){

        // Read KEYs
        KEY_read(&key_pressed);
        // KEY0: START/STOP RECORDING
        if(key_pressed == 1){
            
            if(!recording){
                stopwatch_set(0, 59, 99);
                stopwatch_display();
                stopwatch_run();
                recording = 1;
                rec_num_count = 0;
                LEDR_set(1);
            }
            else{
                stopwatch_stop();
                stopwatch_nodisplay();
                recording = 0;
                LEDR_set(0);
            } 
        }
        // KEY1: PLAY REC
        else if(key_pressed == 2 && !recording){

            LEDR_set(2);
            int a;
            int sec_to_wait, mili_sec_to_wait;
            int note_num;
            
            pthread_mutex_lock(&mutex_tone_volume);
            for(a = 0; a < 13; a++){
                note_vol[a] = 0;
            }
            pthread_mutex_unlock(&mutex_tone_volume);


            for(a = 0; a < rec_num_count; a++){

                // Calc time to wait
                if (a == 0) calc_time_passed(59, 99, recorded_note_array[a].time_sec, recorded_note_array[a].time_mili_sec, &sec_to_wait, &mili_sec_to_wait);
                else calc_time_passed(recorded_note_array[a-1].time_sec, recorded_note_array[a-1].time_mili_sec, recorded_note_array[a].time_sec, recorded_note_array[a].time_mili_sec, &sec_to_wait, &mili_sec_to_wait);
                
                stopwatch_set(0, sec_to_wait, mili_sec_to_wait);
                stopwatch_display();
                stopwatch_run();
                // Wait till next note to play
                stopwatch_read(&minutes, &seconds, &miliseconds);
                while(minutes != 0 || seconds != 0 || miliseconds != 0){
                    stopwatch_read(&minutes, &seconds, &miliseconds);
                }
                stopwatch_stop();

                // Play note
                pthread_mutex_lock(&mutex_tone_volume);
                note_num = recorded_note_array[a].note;
                if( recorded_note_array[a].press_type ) note_vol[ note_num ] = 2;
                draw_waves = 1;
                pthread_mutex_unlock(&mutex_tone_volume);
                
            }

            LEDR_set(0);
            stopwatch_nodisplay();
        }



        // Read keyboard
        if (read(ffd, &ev, event_size) < event_size){
            //continue;
        }
        if (ev.type == EV_KEY && (ev.value == KEY_PRESSED || ev.value == KEY_RELEASED)){

            key = (int) ev.code;

            if (key == 0x002d)
                break;
            pthread_mutex_lock(&mutex_tone_volume);
            if (key > 2 && key < 9){
                if(ev.value){
                    if(key > 2 && key < 5) note_vol[(key-2)*2-1] = 2;
                    if(key > 5 && key < 9) note_vol[(key-6)*2+6] = 2;
                }
                draw_waves = 1;
                if(recording){
                    if(key > 2 && key < 5) recorded_note_array[rec_num_count].note = (key-2)*2-1;
                    if(key > 5 && key < 9) recorded_note_array[rec_num_count].note = (key-6)*2+6;

                    stopwatch_read(&minutes, &seconds, &miliseconds);
                    recorded_note_array[rec_num_count].press_type = ev.value;
                    recorded_note_array[rec_num_count].time_mili_sec = miliseconds;
                    recorded_note_array[rec_num_count].time_sec = seconds;
                }
                rec_num_count++;
            }
			else if (key > 15 && key < 24){
                if(ev.value){    
                    if(key > 15 && key < 19) note_vol[(key-16)*2] = 2;
                    if(key > 18 && key < 22) note_vol[(key-16)*2-1] = 2;
                    if(key > 21 && key < 24) note_vol[key-11] = 2;
                }
                draw_waves = 1;
                if(recording){
                    if(key > 15 && key < 19) recorded_note_array[rec_num_count].note = (key-16)*2;
                    if(key > 18 && key < 22) recorded_note_array[rec_num_count].note = (key-16)*2-1;
                    if(key > 21 && key < 24) recorded_note_array[rec_num_count].note = key-11;

                    stopwatch_read(&minutes, &seconds, &miliseconds);
                    recorded_note_array[rec_num_count].press_type = ev.value;
                    recorded_note_array[rec_num_count].time_mili_sec = miliseconds;
                    recorded_note_array[rec_num_count].time_sec = seconds;
                }
                rec_num_count++;
            }
			else{
                draw_waves = 1;
            }
            pthread_mutex_unlock(&mutex_tone_volume);
        }
    }

    audio_close();
    LEDR_close();
    KEY_close();
    stopwatch_close();

    video_clear();
    video_show();
    sleep(0.5);
    video_clear();
    video_show();
    video_close();

    pthread_cancel(tid1);
    pthread_cancel(tid2);
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    
    close(ffd);
    return 0;
}





void catchSIGINT (int signum) {
    stop = 1;
}

// Set the current thread’s affinity to the core specified
int set_processor_affinity(unsigned int core){

    cpu_set_t cpuset;
    pthread_t current_thread = pthread_self();

    if (core >= sysconf(_SC_NPROCESSORS_ONLN)){
        printf("CPU Core %d does not exist!\n", core);
        return-1;
    }
    // Zero out the cpuset mask
    CPU_ZERO(&cpuset);
    // Set the mask bit for specified core
    CPU_SET(core, &cpuset);

    return pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
}

void calc_time_passed(int sec0, int mili0, int sec1, int mili1, int * sec_passed, int * mili_passed){

    int time0, time1, time_passed;
    time0 = mili0 + sec0 * 100;
    time1 = mili1 + sec1 * 100;

    time_passed = time0 - time1;

    *sec_passed = time_passed / 100;
    *mili_passed = time_passed - (*sec_passed) * 100;
}
