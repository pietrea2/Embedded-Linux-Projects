#ifndef DEFINES_H

#define DEFINES_H

#define TWOPI 6.28318531
#define SAMPLING_RATE 8000

// Audio Core Registers
#define FIFOSPACE 1
#define LDATA 2
#define RDATA 3

#define MAX_VOLUME 0x7FFFFFFF
#define RPS(n)   TWOPI*n/SAMPLING_RATE

#define MIDC RPS(261.626)
#define DFLAT RPS(277.183)
#define DNAT RPS(293.665)
#define EFLAT RPS(311.127)
#define ENAT RPS(329.628)
#define FNAT RPS(349.228)
#define GFLAT RPS(369.994)
#define GNAT RPS(391.995)
#define AFLAT RPS(415.305)
#define ANAT RPS(440.000)
#define BFLAT RPS(466.164)
#define BNAT RPS(493.883)
#define HIC RPS(523.251)

#endif
