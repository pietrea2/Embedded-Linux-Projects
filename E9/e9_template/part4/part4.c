#include <stdio.h>
#include <signal.h>
#include <time.h>
#include "address_map_arm.h"
#include "physical.h"
#include "defines.h"
#include "video.h"
#include "SW.h"
#include "KEY.h"

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

    /** please complete this function **/

        // turn off timer
        timer_settime(interval_timer_id, 0, &interval_timer_stop, NULL);
}


int main(int argc, char* argv[])
{

    /**  please complete the main function **/

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


}
