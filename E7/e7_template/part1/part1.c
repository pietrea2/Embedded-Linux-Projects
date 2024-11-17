#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include "ADXL345.h"
#include "physical.h"
#include "address_map_arm.h"
#include <signal.h>


int main(void){

    if (map_mem() < 0)
        return 1;

    /**  Your part 1 user code here  **/



    unmap_mem();

    return 0;
}
