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
    MAX30205_init_timer(); // use timer0 alarm0 ; delayed start 1s, update temperature every 0.25s
    #endif
    
    for(;;)
    {
        printf("Current temperature: %f     \r", temperature);
    }    
    return 0;
}