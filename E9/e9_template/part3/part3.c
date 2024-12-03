#include <stdio.h>
#include <signal.h>
#include <time.h>
#include "address_map_arm.h"
#include "physical.h"
#include "defines.h"
#include "video.h"
#include "SW.h"

#define video_ORANGE 0xFC00
#define video_PINK   0xFC18
volatile int *ADC_ptr;
volatile int samples_collected;
float samples[320];
float sample;

volatile sig_atomic_t stop;

void catchSIGINT (int signum) {
	stop = 1;
}

/** timer data structures **/
struct itimerspec interval_timer_start = {
    .it_interval = {.tv_sec=0,.tv_nsec=SAMPLING_PERIOD_NS},
    .it_value = {.tv_sec=0,.tv_nsec=SAMPLING_PERIOD_NS}};

struct itimerspec interval_timer_stop  = {
    .it_interval = {.tv_sec=0,.tv_nsec=0},
    .it_value = {.tv_sec=0,.tv_nsec=0}};

timer_t interval_timer_id;

/** timeout handler **/
void timeout_handler(int signo) {

    if( samples_collected < NUM_SAMPLES-1 ){
        while( (*ADC_ptr & 0x8000) == 0 ){}                                 // Wait for R = 1
        sample = *ADC_ptr & 0xFFF;                                          // Take 12 bit sample
        sample = sample * 5.0/4095.0;                                       // Convert to volts
        samples[samples_collected] = sample; 
        samples_collected++;
    }
    else if( samples_collected == NUM_SAMPLES-1 ){
        timer_settime(interval_timer_id, 0, &interval_timer_stop, NULL);    // Turn off timer
        samples_collected = 0;
    }
}


int main(int argc, char* argv[]) {

    /**  please complete the main function **/
    void *LW_virtual;
    int fd = -1;
    float trigger_samples[2];
    int trigger_condition = 0;
    int switch_read;
    samples_collected = 0;

    stop = 0;
    signal(SIGINT, catchSIGINT);

    if ((fd = open_physical(fd)) == -1)
        return (-1);
    else if ((LW_virtual = map_physical(fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN)) == NULL)
        return (-1);
    if ( !video_open() ){
        printf("Error opening /dev/IntelFPGAUP/video\n");
        return -1;
    }
    if ( !SW_open() ){
        printf("Error opening /dev/IntelFPGAUP/SW\n");
        return -1;
    }

    ADC_ptr = (unsigned int *)(LW_virtual + ADC_BASE);
    *(ADC_ptr + 1) = 1;                                 // Activate AUTO-update
    video_clear();                                      // Clear VGA display
    video_show();
    sleep(1);
    video_clear();
    video_show();

    // Set up the signal handling (version provided in lab instructions)
    /* struct sigaction act;
    sigset_t set;
    sigemptyset (&set);
    sigaddset (&set, SIGALRM);
    act.sa_flags = 0;
    act.sa_mask = set;
    act.sa_handler = &timeout_handler;
    sigaction (SIGALRM, &act, NULL); */

    // set up signal handling (shorter version shown in lecture)
    signal(SIGALRM, timeout_handler);

    // Create a monotonically increasing timer
    timer_create (CLOCK_MONOTONIC, NULL, &interval_timer_id);

    
    int a;
    for(a = 0; a < 2; a++){
        while( (*ADC_ptr & 0x8000) == 0 ){}             // Read 2 samples from ADC!
        sample = *ADC_ptr & 0xFFF;          
        sample = sample * 5.0/4095.0;
        trigger_samples[a] = sample;
    }


    while( !stop ){

        while( (*ADC_ptr & 0x8000) == 0 ){}             // Read sample from ADC
        sample = *ADC_ptr & 0xFFF;          
        sample = sample * 5.0/4095.0;
        trigger_samples[0] = trigger_samples[1];
        trigger_samples[1] = sample;

        SW_read(&switch_read);                          // Check SW and look for trigger: Rising or falling edge of wave
        trigger_condition = 0;
        if( switch_read & 0x1 == 1 ){
            if( trigger_samples[1] - trigger_samples[0] > 1.0 ) trigger_condition = 1;      // Check for Rising Edge
        }
        else {
            if( trigger_samples[0] - trigger_samples[1] > 1.0 ) trigger_condition = 1;      // Check for Falling Edge
        }

        if( trigger_condition ){

            timer_settime(interval_timer_id, 0, &interval_timer_start, NULL);               // 1: Start sample timer
            while( samples_collected < NUM_SAMPLES-1 ) {}                                   // 2: Wait until done collecting samples for sweep

            video_clear();                                                                  // 3: Draw sweep
            for(a = 0; a < RES_X-1; a++){
                video_line(a, (RES_Y-1) - ((int)samples[a]) * 40, a+1, (RES_Y-1) - ((int)samples[a+1]) * 40, video_PINK);
            }
            video_show();
        }
    }

    video_clear();
    video_show();
    video_clear();
    video_show();
    video_close();
    SW_close();

    if (unmap_physical(LW_virtual, LW_BRIDGE_SPAN) != 0){
        printf("ERROR unmapping virtual address mapping");
    }
    close_physical(fd);

    return 0;
}