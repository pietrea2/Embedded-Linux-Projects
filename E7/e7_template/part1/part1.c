#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include "ADXL345.h"
#include "physical.h"
#include "address_map_arm.h"
#include <signal.h>

volatile sig_atomic_t stop;

void catchSIGINT(int signum);

int main(void){

    if (map_mem() < 0)
        return 1;


    // catch SIGINT from ctrl+c, instead of having it abruptly close this program
    signal(SIGINT, catchSIGINT);

    uint8_t devid;
    int16_t mg_per_lsb = calc_mg_per_lsb(XL345_10BIT, XL345_RANGE_16G);
    int16_t XYZ[3];
    
    // Configure Pin Muxing
    Pinmux_Config();
    printf("Pinmux configured\n");
    
    // Initialize I2C0 Controller
    I2C0_Init();
    printf("I2C0 initialized");
    
    // 0xE5 is read from DEVID(0x00) if I2C is functioning correctly
    ADXL345_REG_READ(0x00, &devid);
    
    // Correct Device ID
    if (devid == 0xE5){
        // Initialize accelerometer chip
        ADXL345_Init();
        printf("ADXL345 initialized");
        ADXL345_Calibrate();
        
        while(!stop){
            if ( ADXL345_IsDataReady() ){
                ADXL345_XYZ_Read(XYZ);
                printf("X=%d mg, Y=%d mg, Z=%d mg\n", XYZ[0]*mg_per_lsb, XYZ[1]*mg_per_lsb, XYZ[2]*mg_per_lsb);
            }
            //else printf("Activity not updated\n");
        }
    } else {
        printf("Incorrect device ID\n");
    }

    unmap_mem();

    return 0;
}

void catchSIGINT(int signum)
{
    stop = 1;
}
