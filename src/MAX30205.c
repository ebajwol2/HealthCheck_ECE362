#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdlib.h>
#include <stdio.h>

#define SDA_PIN (26)
#define SCL_PIN (27)

void MAX30205_init_i2c()
{
    // using I2C1 pin26 27
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);

    i2c_init(i2c1, 125000);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
}

void MAX30205_check_address()
{
    printf("scanning I2C bus for MAX30205...\n"); fflush(stdout);
    
    for (uint8_t addr = 0x08; addr < 0x7F; addr++)
    {   
        uint8_t reg=0x00;
        i2c_write_blocking(i2c1, addr, &reg, 1, true);

        uint8_t result;
        int read = i2c_read_blocking(i2c1, addr, &result, 1, false);
        if (read >= 0)
        {
            printf("I2C found at 0x%02X\n", addr); fflush(stdout);
            sleep_ms(100);
        } 
        else
        {
            printf("Failed at 0x%X\r\n", addr); fflush(stdout);
            sleep_ms(100);
        } 
    }
    printf("Scanning End...\n"); fflush(stdout);
}