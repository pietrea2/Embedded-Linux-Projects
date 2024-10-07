

/**  your part 4 user code here  **/
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#define chardev_BYTES 256 // max number of characters to read from /dev/chardev

volatile sig_atomic_t stop;
void catchSIGINT(int signum)
{
    stop = 1;
}

/* This code uses the character device driver /dev/chardev. The code reads the default
 * message from the driver and then prints it. After this the code changes the message in
 * a loop by writing to the driver, and prints each new message. The program exits if it
 * receives a kill signal (for example, ^C typed on stdin). */
int main(int argc, char *argv[])
{
    FILE *ledr_fp;                  // LEDR file pointer
    FILE *hex_fp;                   // Hex displays file pointer
    FILE *sw_fp;                    // Switch file pointer
    FILE *key_fp;                   // Keys file pointer
    char sw_buffer[chardev_BYTES];  // buffer for chardev character data
    char key_buffer[chardev_BYTES]; // buffer for chardev character data
    char sum_ptr[chardev_BYTES]; // buffer for chardev character data
    char new_msg[128];              // space for the new message that we generate
    int sum;
    int sw_value;

    // catch SIGINT from ctrl+c, instead of having it abruptly close this program
    signal(SIGINT, catchSIGINT);

    if ((ledr_fp = fopen("/dev/LEDR", "w")) == NULL)
    {
        printf("Error opening /dev/LEDR: %s\n", strerror(errno));
        return -1;
    }
    if ((hex_fp = fopen("/dev/HEX", "w")) == NULL)
    {
        printf("Error opening /dev/chardev: %s\n", strerror(errno));
        return -1;
    }
    if ((sw_fp = fopen("/dev/SW", "r")) == NULL)
    {
        printf("Error opening /dev/chardev: %s\n", strerror(errno));
        return -1;
    }
    if ((key_fp = fopen("/dev/KEY", "r")) == NULL)
    {
        printf("Error opening /dev/chardev: %s\n", strerror(errno));
        return -1;
    }

    sum = 0;
    while (!stop)
    {
        int key_value;
        while (fgets(key_buffer, chardev_BYTES, key_fp))
            ; // read the driver until EOF
        sscanf(key_buffer, "%d", &key_value);
        // printf("keys === %d\n", key_value);
        // sscanf(sum,"%s",sum_ptr);
        sprintf(sum_ptr, "%d\n", sum);
        fputs(sum_ptr, hex_fp);
        fflush(hex_fp);
        if (key_value > 0)
        {
            while (fgets(sw_buffer, chardev_BYTES, sw_fp))
                ; // read the driver until EOF

            printf("%s", sw_buffer);
            sscanf(sw_buffer, "%x", &sw_value);
	    if (sw_value == 0)
		sum = 0;
            sum += sw_value;
            fputs(sw_buffer, ledr_fp);
            fflush(ledr_fp);
        }
        sleep(1);
    }
    fclose(sw_fp);
    fclose(hex_fp);
    fclose(ledr_fp);
    fclose(key_fp);
    return 0;
}
