#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#define chardev_BYTES 256 // max number of characters to read from /dev/SW or KEY

volatile sig_atomic_t stop;
void catchSIGINT(int signum)
{
    stop = 1;
}

int main(int argc, char *argv[])
{
    FILE *ledr_fp;                  // LEDR file pointer
    FILE *hex_fp;                   // HEX displays file pointer
    FILE *sw_fp;                    // Switch file pointer
    FILE *key_fp;                   // Keys file pointer

    char sw_buffer[chardev_BYTES];  // buffer for SW character data
    char key_buffer[chardev_BYTES]; // buffer for KEY character data
    char sum_ptr[chardev_BYTES];    // buffer for HEX character data

    int sum;
    int sw_value;
    int key_value;

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
    sprintf(sum_ptr, "%06d\n", sum);                            // store sum in sum_ptr as a 6 digit int padded with 0s and \n
    fputs(sum_ptr, hex_fp);                                     // write sum buffer to HEX driver (to display accumulator)
    fflush(hex_fp);

<<<<<<< HEAD
            printf("%s", sw_buffer);
            sscanf(sw_buffer, "%x", &sw_value);
	    if (sw_value == 0)
		sum = 0;
            sum += sw_value;
            fputs(sw_buffer, ledr_fp);
=======
    while (!stop) {

        while ( fgets(key_buffer, chardev_BYTES, key_fp) );         // read the KEY driver until EOF
        sscanf(key_buffer, "%x", &key_value);                       // format output buffer from KEY and store as hex value in key_value

        //if key is pressed
        if (key_value > 0) {

            while ( fgets(sw_buffer, chardev_BYTES, sw_fp) );  // read the SW driver until EOF
            sw_buffer[ strcspn(sw_buffer, "\n") ] = 0;         // remove \n from string
            sscanf(sw_buffer, "%3x", &sw_value);               //store sw_buffer in sw_value as 3 digit formatted hex

            printf("Switch value: %s (hex) --> %d (decimal)\n", sw_buffer, sw_value);

            //reset accumulator if switches = 0 and key pressed
            if( sw_value == 0 ){
                sum = 0;
            }
            else{
                if( (sum + sw_value) <= 999999 ) sum += sw_value;
            }

            fputs(sw_buffer, ledr_fp);                                  // write sw_buffer to LEDR driver to display SW value
>>>>>>> 41b612edd93e491d22b8a8819df934b6a07598a4
            fflush(ledr_fp);

            sprintf(sum_ptr, "%06d\n", sum);                            // store sum in sum_ptr as a 6 digit int padded with 0s and \n
            fputs(sum_ptr, hex_fp);                                     // write sum buffer to HEX driver (to display accumulator)
            fflush(hex_fp);
        }

        sleep(1);
    }

    //close all char driver files
    fclose(sw_fp);
    fclose(hex_fp);
    fclose(ledr_fp);
    fclose(key_fp);

    return 0;
}
