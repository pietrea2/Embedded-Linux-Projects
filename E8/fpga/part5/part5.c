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
#include "physical.h"
#include "address_map_arm.h"
#include "defines.h"
#include "stopwatch.h"
#include "video.h"
#include "KEY.h"
#include "LEDR.h"


/**  your part 5 user code here  **/
