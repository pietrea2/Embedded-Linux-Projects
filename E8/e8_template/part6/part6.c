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


/**  your part 6 user code here  **/
