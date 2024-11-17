#ifdef KERNEL
#include <linux/kernel.h>
#include <linux/fs.h>               // struct file, struct file_operations
#include <linux/init.h>             // for __init, see code
#include <linux/module.h>           // for module init and exit macros
#include <linux/miscdevice.h>       // for misc_device_register and struct miscdev
#include <linux/uaccess.h>          // for copy_to_user, see code
#include <asm/io.h>                 // for mmap
#endif

#include "include/ADXL345.h"
#include "include/address_map_arm.h"


/** The following is provided so that you can use this file in both user and kernel space **/

volatile unsigned int * SYSMGR_virtual;
volatile unsigned int * I2C0_virtual;

#ifdef KERNEL

void pass_addrs(unsigned int* SYSMGR_ptr_ext, unsigned int* I2C0_ptr_ext) {

    SYSMGR_virtual = SYSMGR_ptr_ext;
    I2C0_virtual = I2C0_ptr_ext;

}

#else
#include "physical.h"

int fd = -1;
int map_mem() {

    // open /dev/mem
    if ((fd = open_physical (fd)) == -1)
        return (-1);
    else if ((SYSMGR_virtual = map_physical (fd, SYSMGR_BASE, SYSMGR_SPAN)) == 0)
        return (-1);
    else if ((I2C0_virtual = map_physical (fd, I2C0_BASE, I2C0_SPAN)) == 0)
        return (-1);

    return 0;

}
void unmap_mem() {
    unmap_physical ((void*) SYSMGR_virtual, SYSMGR_SPAN);
    unmap_physical ((void*) I2C0_virtual, I2C0_SPAN);
    close_physical(fd);
}

#endif


/** This sample mg_per_lsb calculator is provided for your convenience **/
int16_t calc_mg_per_lsb(uint8_t resolution, uint8_t range) {

    if (resolution == XL345_FULL_RESOLUTION)
        return 4;

    switch(range) {
        case XL345_RANGE_2G:  return ROUNDED_DIVISION(2*1000, 512);
        case XL345_RANGE_4G:  return ROUNDED_DIVISION(4*1000, 512);
        case XL345_RANGE_8G:  return ROUNDED_DIVISION(8*1000, 512);
        case XL345_RANGE_16G: return ROUNDED_DIVISION(16*1000, 512);
        default: return 0;
   }
}


/** This sample offset calculator is provided for your convenience **/
void calc_offsets(int16_t mg_per_lsb, int avx, int avy, int avz,
                            int8_t* ofx, int8_t* ofy, int8_t* ofz) {

    int g_per_lsb, offset_scale;

    g_per_lsb = mg_per_lsb * 1000;

    if (g_per_lsb > 15600) {

        offset_scale = ROUNDED_DIVISION(g_per_lsb, 15600);
        *ofx = 0 - (avx * offset_scale);
        *ofy = 0 - (avy * offset_scale);
        *ofz = 0 - (avz - ROUNDED_DIVISION(1000,mg_per_lsb)) * offset_scale;

    } else {

        offset_scale = ROUNDED_DIVISION(15600, g_per_lsb);
        *ofx = 0 - ROUNDED_DIVISION(avx, offset_scale);
        *ofy = 0 - ROUNDED_DIVISION(avy, offset_scale);
        *ofz = 0 - ROUNDED_DIVISION(avz - ROUNDED_DIVISION(1000,mg_per_lsb), offset_scale);

    }
}



/**  Please. Implement the following ADXL345 functions; your implementations      **/
/**  will be similar to accel_tutorial/ADXL345.c, but will use virtual addresses  **/
/**  NOTE: not all functions will be used in every part of the lab exercise       **/

// Set up pin muxing (in sysmgr) to connect ADXL345 wires to I2C0
void Pinmux_Config() {

}

// Initialize the I2C0 controller for use with the ADXL345 chip
void I2C0_Init() {

}

// Write value to internal register at address
void ADXL345_REG_WRITE(uint8_t address, uint8_t value) {

}

// Read value from internal register at address
void ADXL345_REG_READ(uint8_t address, uint8_t *value) {

}

// Read multiple consecutive internal registers
void ADXL345_REG_MULTI_READ(uint8_t address, uint8_t values[], uint8_t len) {

}

// Initialize the ADXL345 chip
void ADXL345_Init() {

}

// Calibrate the ADXL345. The DE1-SoC should be placed on a flat
// surface, and must remain stationary for the duration of the calibration.
void ADXL345_Calibrate() {

}

// Read acceleration data of all three axes
void ADXL345_XYZ_Read(int16_t szData16[3]) {

}

// Read the ID register
void ADXL345_IdRead(uint8_t *pId) {

}

