#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "hardware/i2c.h"
#include "MAX30205.h"

#define MAX30205_TEST

int main()
{
    stdio_init_all();
    sleep_ms(3000);
    printf("START...\n");
    fflush(stdout);

    #ifdef MAX30205_TEST
    MAX30205_init_i2c();
    //MAX30205_check_address();

    #endif
    for(;;)
    {
        float temp = read_temperature();
        printf("Current temperature: %f     \r", temp);
        sleep_ms(10);
    }    
    return 0;
}