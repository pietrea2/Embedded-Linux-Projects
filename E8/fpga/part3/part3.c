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
#include <physical.h>
#include <linux/input.h>
#include <pthread.h>
#include "address_map_arm.h"
#include "defines.h"

#define KEY_RELEASED 0
#define KEY_PRESSED 1
#define PLAYING_TIME 2000

volatile sig_atomic_t stop;
void catchSIGINT(int signum)
{
    stop = 1;
}

pthread_mutex_t mutex_tone_volume;
double note[13];
volatile int fd = -1;
void *LW_virtual;
cpu_set_t  mask;
inline void assignToThisCore(int core_id)
{
    CPU_ZERO(&mask);
    CPU_SET(core_id, &mask);
    sched_setaffinity(0, sizeof(mask), &mask);
}

void audio_thread()
{

    // declare and set up address pointer to AUDIO controller
    volatile int *AUDIO_ptr;
    assignToThisCore(1);
    note[13];
    size_t i;
    double freq_sum = 0;

    AUDIO_ptr = (unsigned int *)(LW_virtual + AUDIO_BASE);
    *AUDIO_ptr |= 0x8;        // Set CW to 1
    *AUDIO_ptr &= 0xFFFFFFF7; // Set CW to 0

    double scale[13] = {MIDC, DFLAT, DNAT, EFLAT, ENAT, FNAT, GFLAT, GNAT, AFLAT, ANAT, BFLAT, BNAT, HIC};
    int nth_sample = 0;
    while (1)
    {
        for (nth_sample = 0; nth_sample < SAMPLING_RATE * PLAYING_TIME / 1000 & !stop;)
        {
            pthread_testcancel();
            {
                int write = 0;
                while (write == 0)
                    if ((*(AUDIO_ptr + FIFOSPACE) & 0x00FF0000) && (*(AUDIO_ptr + FIFOSPACE) & 0xFF000000))
                    {
                        freq_sum = 0;
                        pthread_mutex_lock(&mutex_tone_volume);
                        for (i = 0; i < 13; i++)
                        {
                            freq_sum += MAX_VOLUME * note[i] * sin(nth_sample * scale[i]);
                            note[i] /= 1.0001;
                        }
                        // printf("frrr %lf\n", freq_sum);
                        pthread_mutex_unlock(&mutex_tone_volume);
                        *(AUDIO_ptr + LDATA) = (int)(freq_sum);
                        *(AUDIO_ptr + RDATA) = (int)(freq_sum);

                        nth_sample++;
                        write = 1;
                    }
            }
        }
    }
}


int main(int argc, char *argv[])
{
    assignToThisCore(0);
    struct input_event ev;
    int ffd, event_size = sizeof(struct input_event);
    int err;
    pthread_t tid;
    // signal(SIGINT, catchSIGINT);
    
    if ((fd = open_physical(fd)) == -1)
        return (-1);
    else if ((LW_virtual = map_physical(fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN)) == NULL)
        return (-1);
    if ((err = pthread_create(&tid, NULL, &audio_thread, NULL)) != 0)
        printf("pthread_create failed:[%s]\n", strerror(err));

    // Get the keyboard device
    if (argv[1] == NULL)
    {
        printf("Specify the path to the keyboard device ex./dev/input/by-id/HP-KEYBOARD");
        return -1;
    }

    // Open keyboard device
    if ((ffd = open(argv[1], O_RDONLY | O_NONBLOCK)) == -1)
    {
        printf("Could not open %s\n", argv[1]);
        return -1;
    }

    while (!stop)
    {
        // Read keyboard
        if (read(ffd, &ev, event_size) < event_size)
        {
            // No event
            continue;
        }
        if (ev.type == EV_KEY && (ev.value == KEY_PRESSED || ev.value == KEY_RELEASED))
        {

            printf("Pressed key: 0x%04x\n", (int)ev.code);
            if ((int)ev.code == 0x002d)
                break;
            pthread_mutex_lock(&mutex_tone_volume);
            if ((int)ev.code >= 0x3 && (int)ev.code <= 0x4)
                note[((int)ev.code - 0x3) * 2 + 1] = ev.value;
            if ((int)ev.code >= 0x6 && (int)ev.code <= 0x8)
                note[((int)ev.code - 0x3) * 2 ] = ev.value;
            if ((int)ev.code >= 0x10 && (int)ev.code <= 0x12)
                note[((int)ev.code - 0x10) * 2] = ev.value;
            if ((int)ev.code >= 0x13 && (int)ev.code <= 0x16)
                note[((int)ev.code - 0x10) * 2 - 1] = ev.value;
            if ((int)ev.code == 0x17)
                note[12] = ev.value;
            pthread_mutex_unlock(&mutex_tone_volume);
        }
    }
    
    pthread_cancel(tid);
    pthread_join(tid, NULL);
    if (unmap_physical(LW_virtual, LW_BRIDGE_SPAN) != 0)
        printf("ERROR unmapping virtual address mapping");
    close_physical(fd);
    close(ffd);
    return 0;
}