#define _GNU_SOURCE 						// required for set_processor_affinity code
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>

pthread_mutex_t sample_mutex;			// an example of a mutex for a shared variable

static volatile sig_atomic_t stop;
static pthread_t tid1, tid2;			// thread IDs
static int i, j, k;						// examples of static global variables

// Set the current thread's affinity to the core specified
int set_processor_affinity(unsigned int core) {
    cpu_set_t cpuset;
    pthread_t current_thread = pthread_self(); 
    
    if (core >= sysconf(_SC_NPROCESSORS_ONLN)){
        printf("CPU Core %d does not exist!\n", core);
        return -1;
    }
    // Zero out the cpuset mask
    CPU_ZERO(&cpuset);
    // Set the mask bit for specified core
    CPU_SET(core, &cpuset);
    
    return pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset); 
}

void *thread_1( ) {
	set_processor_affinity(1);			// assign this thread to CPU 1
	while (1)
	{
		pthread_testcancel();			// exit if this thread has been cancelled
		++i;
		pthread_mutex_lock (&sample_mutex);
			++k;
		pthread_mutex_unlock (&sample_mutex);
	}
}

void *thread_2( ) {
	set_processor_affinity(0);			// assign this thread to CPU 0
	while (1)
	{
		pthread_testcancel();			// exit if this thread has been canceled
		++j;
		pthread_mutex_lock (&sample_mutex);
			++k;
		pthread_mutex_unlock (&sample_mutex);
	}
}

void catchSIGINT(int signum) {			// used for ^C exit of the program
	stop = 1;
}

int main (int argc, char *argv[]) {
	int err1, err2, loops = 0;

	// catch SIGINT from ctrl+c, instead of having it abruptly close this program
	signal(SIGINT, catchSIGINT);
	
	// Spawn threads
	err1 = pthread_create (&tid1, NULL, &thread_1, NULL);
	err2 = pthread_create (&tid2, NULL, &thread_2, NULL);
	if ((err1 || err2) != 0)
	{
		printf("pthread_create failed\n");
		exit (-1);
	}
	set_processor_affinity(0);			// assign the main thread to CPU 0
	while (!stop) {
		printf ("The program is running\n");
		++loops;
		sleep (1);
	}
	printf ("\nThe program is stopping!\n");
	pthread_cancel(tid1);
	pthread_cancel(tid2);
	// Wait until threads terminates
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);

	printf ("loops = %d\n", loops);
	printf ("i = %d\n", i);
	printf ("j = %d\n", j);
	printf ("k = %d\n", k);
	return 0;
} 
