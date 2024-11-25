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

static const char *const press_type[2] = {"RELEASED", "PRESSED "};
char *Note[] = {"C#", "D#", " ", "F#", "G#", "A#", "C", "D", "E", "F", "G", "A", "B", "C"};
char ASCII[] = {'2',  '3',  '4', '5',  '6',  '7',  'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I'};

static volatile sig_atomic_t stop;
void catchSIGINT (int signum);

pthread_mutex_t mutex_tone_volume;
double note[13] = {0};

void *LW_virtual;

int set_processor_affinity(unsigned int core);

cpu_set_t  mask;
inline void assignToThisCore(int core_id);




void audio_thread(){

    //assignToThisCore(1);
    set_processor_affinity(1);

    // declare and set up address pointer to AUDIO controller
    volatile int *AUDIO_ptr;
    //note[13];
    size_t i;
    double freq_sum = 0;

    AUDIO_ptr = (unsigned int *)(LW_virtual + AUDIO_BASE);
    *AUDIO_ptr |= 0x8;        // Set CW to 1
    *AUDIO_ptr &= 0xFFFFFFF7; // Set CW to 0

    double scale[13] = {MIDC, DFLAT, DNAT, EFLAT, ENAT, FNAT, GFLAT, GNAT, AFLAT, ANAT, BFLAT, BNAT, HIC};
    int nth_sample = 0;

    while (!stop){

        //pthread_testcancel();

        for (nth_sample = 0; nth_sample < (SAMPLING_RATE * PLAYING_TIME / 1000) && !stop; nth_sample++){

            pthread_testcancel();
            int write = 0;

            while (write == 0 && !stop) {

                if ((*(AUDIO_ptr + FIFOSPACE) & 0x00FF0000) && (*(AUDIO_ptr + FIFOSPACE) & 0xFF000000)){
                    
                    freq_sum = 0;
                    pthread_mutex_lock(&mutex_tone_volume);
                    for (i = 0; i < 13; i++){
                        freq_sum += MAX_VOLUME/13 * note[i] * sin(nth_sample * scale[i]);
                        note[i] /= 1.0001;
                    }
                    // printf("frrr %lf\n", freq_sum);
                    pthread_mutex_unlock(&mutex_tone_volume);
                    *(AUDIO_ptr + LDATA) = (int)(freq_sum);
                    *(AUDIO_ptr + RDATA) = (int)(freq_sum);

                    write = 1;
                }
            }
        }
    }
}


int main(int argc, char *argv[]){

    //assignToThisCore(0);

    volatile int fd = -1;
    struct input_event ev;
    int ffd, event_size = sizeof(struct input_event), key;
    int err;
    pthread_t tid;

    stop = 0;
    signal(SIGINT, catchSIGINT);
    
    if ((fd = open_physical(fd)) == -1)
        return (-1);
    else if ((LW_virtual = map_physical(fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN)) == NULL)
        return (-1);
    if ((err = pthread_create(&tid, NULL, &audio_thread, NULL)) != 0)
        printf("pthread_create failed:[%s]\n", strerror(err));

    set_processor_affinity(0);

    // Get the keyboard device
    if (argv[1] == NULL){
        printf("Specify the path to the keyboard device ex./dev/input/by-id/HP-KEYBOARD");
        return -1;
    }
    // Open keyboard device
    // Testing using:
    // /dev/input/by-id/usb-CHICONY_HP_USB_Multimedia_Keyboard-event-kbd
    if ((ffd = open(argv[1], O_RDONLY | O_NONBLOCK)) == -1){
        printf("Could not open %s\n", argv[1]);
        return -1;
    }



    while (!stop){

        // Read keyboard
        if (read(ffd, &ev, event_size) < event_size){
            continue;
        }
        if (ev.type == EV_KEY && (ev.value == KEY_PRESSED || ev.value == KEY_RELEASED)){

            key = (int) ev.code;
            //printf("Pressed key: 0x%04x\n", key);
            if (key == 0x002d)
                break;
            pthread_mutex_lock(&mutex_tone_volume);
            if (key > 2 && key < 9){
                printf("You %s key %c (note %s)\n", press_type[ev.value], ASCII[key-3], Note[key-3]);
                if(key > 2 && key < 5) note[(key-2)*2-1] = ev.value;
                if(key > 5 && key < 9) note[(key-6)*2+6] = ev.value;
            }
			else if (key > 15 && key < 24){
				printf("You %s key %c (note %s)\n", press_type[ev.value], ASCII[key-10], Note[key-10]);
                if(key > 15 && key < 19) note[(key-16)*2] = ev.value;
                if(key > 18 && key < 22) note[(key-16)*2-1] = ev.value;
                if(key > 21 && key < 24) note[key-11] = ev.value;
            }
			else{
				printf("You %s key code 0x%04x\n", press_type[ev.value], key);
            }
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



void catchSIGINT (int signum) {
    stop = 1;
}

inline void assignToThisCore(int core_id){
    CPU_ZERO(&mask);
    CPU_SET(core_id, &mask);
    sched_setaffinity(0, sizeof(mask), &mask);
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
