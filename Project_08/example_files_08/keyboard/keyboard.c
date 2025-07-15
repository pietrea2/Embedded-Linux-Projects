/* This code provides an example of reading from a keyboard device */
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <linux/input.h>

#define RELEASED 0
#define PRESSED 1
static const char *const press_type[2] = {"RELEASED", "PRESSED "};

char *Note[] = {"C#", "D#", " ", "F#", "G#", "A#", "C", "D", "E", "F", "G", "A", "B", "C"};
char ASCII[] = {'2',  '3',  '4', '5',  '6',  '7',  'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I'};

static volatile sig_atomic_t stop;
void catchSIGINT(int signum) {
	stop = 1;
}

int main (int argc, char *argv[]) {
	struct input_event ev;
	int fd_kb, event_size = sizeof (struct input_event), key;
	// set a default keyboard
	char *keyboard = "/dev/input/by-id/usb-Logitech_USB_Receiver-event-kbd";
	
	// Open keyboard device
	if ((fd_kb = open (keyboard, O_RDONLY | O_NONBLOCK)) == -1) {
		printf ("Could not open %s\n", keyboard);
		return -1;
	}
	// catch SIGINT from ctrl+c, instead of having it abruptly close this program
	signal(SIGINT, catchSIGINT);
	
	while (!stop) {
		// Read keyboard
		if (read (fd_kb, &ev, event_size) < event_size)
			continue;
		if (ev.type == EV_KEY && ((ev.value == RELEASED) || (ev.value == PRESSED))) {
			key = (int) ev.code;
			if (key > 2 && key < 9)
				printf("You %s key %c (note %s)\n", press_type[ev.value], ASCII[key-3], 
					Note[key-3]);
			else if (key > 15 && key < 24)
				printf("You %s key %c (note %s)\n", press_type[ev.value], ASCII[key-10], 
					Note[key-10]);
			else
				printf("You %s key code 0x%04x\n", press_type[ev.value], key);
		}
	}
	printf ("\nExiting program\n");
	return 0;
} 
