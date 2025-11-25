#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdlib.h>
#include <stdio.h>
#include "MAX30205.h"

#define SDA_PIN (16)
#define SCL_PIN (17)

void MAX30205_init_i2c()
{
    // using I2C1 pin26 27
    i2c_init(i2c0, 125000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);

    // gpio_pull_up(SDA_PIN);
    // gpio_pull_up(SCL_PIN);

    //test
    gpio_set_function(22, GPIO_FUNC_SIO);
    gpio_init(22);
    gpio_set_dir(22, 1);
    gpio_put(22, 1);
}

void MAX30205_check_address()
{
    printf("scanning I2C bus for MAX30205...\n"); fflush(stdout);

    printf("   0 1 2 3 4 5 6 7 8 9 A B C D E F\n");fflush(stdout);
    for (int addr = 0; addr < (1 << 7); ++addr) {
        //printf("Scanning %d", addr); fflush(stdout);
        if (addr % 16 == 0) {
            printf("%02x ", addr);
        }

        int ret;
        uint8_t rxdata;
        uint8_t txdata=0;
        i2c_write_blocking(i2c0, addr, &txdata, 1, true);
        ret = i2c_read_blocking(i2c0, addr, &rxdata, 1, false);

        if (ret<0)
        {
            printf(".");fflush(stdout);
        } else
        {
            printf("@");fflush(stdout);
        }
        printf(addr % 16 == 15 ? "\n" : " ");fflush(stdout);
    }
    printf("Done\n");fflush(stdout);
}

void read_temperature()
{
    hw_clear_bits(&timer0_hw->intr, 1u << 0); // acknowledge timer

    int addr = 0x48;
    uint8_t txdata = 0;
    uint8_t buf[2];
    i2c_write_blocking(i2c0, addr, &txdata, 1, true);
    i2c_read_blocking(i2c0, addr, buf, 2, false);

    uint16_t raw_data = (uint16_t) (buf[0] << 8 | buf[1]);
    int signed_raw = (int16_t) raw_data;
    temperature = signed_raw * 0.00390625f + 64.0f;

    uint64_t quat_s = timer0_hw->timerawl + 250000;
    timer0_hw->alarm[0] = (uint32_t) quat_s;
}

void MAX30205_init_timer()
{
    hw_set_bits(&timer0_hw->inte, 1u << 0); // interrupt enable for timer0 alarm0
    irq_set_exclusive_handler(timer_hardware_alarm_get_irq_num(timer0_hw, 0), read_temperature);
    irq_set_enabled(timer_hardware_alarm_get_irq_num(timer0_hw, 0), true);
    uint64_t s1 = timer0_hw->timerawl + 1000000;
    timer0_hw->alarm[0] = (uint32_t) s1;
}