#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "address_map_arm.h"
#include "physical.h"
#include "defines.h"
#include "video.h"
#include "SW.h"

// #include "SW.h"
#define SAMPLES 320
#define video_BYTES 8
/** timer data structures **/
struct itimerspec interval_timer_start = {
    .it_interval = {.tv_sec = 0, .tv_nsec = SAMPLING_PERIOD_NS},
    .it_value = {.tv_sec = 0, .tv_nsec = SAMPLING_PERIOD_NS}};

struct itimerspec interval_timer_stop = {
    .it_interval = {.tv_sec = 0, .tv_nsec = 0},
    .it_value = {.tv_sec = 0, .tv_nsec = 0}};

timer_t interval_timer_id;

volatile sig_atomic_t stop;
volatile int samples;
volatile int x;
volatile float sample1;
volatile float sample2;
volatile int *ADC_ptr;
volatile int rising;
volatile int start_capture;
void *LW_virtual;
int video_FD;             // file descriptor
char buffer[video_BYTES]; // buffer for data read from /dev/video
char command[64];         // buffer for commands written to /dev/video

int fd = -1;

void catchSIGINT(int signum)
{
    stop = 1;
    
}

/** timeout handler **/
void timeout_handler(int signo)
{

    /** please complete this function **/
    if(stop){
        timer_settime(interval_timer_id, 0, &interval_timer_stop, NULL);
        video_clear();
        video_show();
        video_clear();
        video_show();
        return;
    }
    while ((*ADC_ptr & 0x8000) == 0)
    {
    }
    sample2 = sample1;
    sample1 = *ADC_ptr & 0xFFF;       // Take 12 bit sample
    sample1 = sample1 * 5.0 / 4095.0; // Convert to volts
    sample1 = 150 - (sample1 * 20);
    // sprintf (command, "pixel %d,%d %hX\n", x, 150 - (sample * 2.5), 0xCCCC);
    // write(video_FD, command, sizeof(command));
    // sprintf(command, "sync");                       // VGA sync
    // write(video_FD, command, sizeof(command));
    // video_clear();
    if (!start_capture)
    {
        if (abs(sample1 - sample2) < 1 * 20)
            return;
        if ((rising & 0x1) && (sample1 - sample2 > 0))
        {
            return;
        }
        else if (!(rising & 0x1) && (sample1 - sample2 < 0))
        {
            return;
        }
    }
    start_capture = 1;

    video_line(x, sample2, x + 1, sample1, 0xCC);
    video_show();
    // printf("%.2lf v\n", sample);

    x++;
    if (x >= SAMPLES)
    {
        x = 0;
        // stop = 1;
        video_clear();
        video_show();
        video_clear();
        video_show();
        // timer_settime(interval_timer_id, 0, &interval_timer_stop, NULL);
    }

    // turn off timer
}

int main(int argc, char *argv[])
{

    if ((fd = open_physical(fd)) == -1)
        return (-1);
    else if ((LW_virtual = map_physical(fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN)) == NULL)
        return (-1);

    srand(time(NULL));

    if (!video_open())
    {
        return -1;
    }
    if (!SW_open())
    {
        return -1;
    }
    video_clear();
    video_show();
    video_clear();
    video_show();
    ADC_ptr = (unsigned int *)(LW_virtual + ADC_BASE);
    *(ADC_ptr + 1) = 1; // Activate AUTO-update

    /**  please complete the main function **/
    stop = 0;
    x = 0;
    sample1 = sample2 = 150;
    start_capture = 0;
    SW_read(&rising);
    // Set up the signal handling (version provided in lab instructions)
    struct sigaction act;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    act.sa_flags = 0;
    act.sa_mask = set;
    act.sa_handler = &timeout_handler;
    sigaction(SIGALRM, &act, NULL);

    // set up signal handling (shorter version shown in lecture)
    signal(SIGALRM, timeout_handler);

    // Create a monotonically increasing timer

    timer_create(CLOCK_MONOTONIC, NULL, &interval_timer_id);

    timer_settime(interval_timer_id, 0, &interval_timer_start, NULL);

    while (!stop)
    {
    }
    timer_settime(interval_timer_id, 0, &interval_timer_stop, NULL);
    video_clear();
    video_show();
    video_clear();
    video_show();

    SW_close();
    video_close();
    return 0;
}
