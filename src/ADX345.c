#include "ADX345.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <math.h>
#include <stdio.h>

#define ADXL345_ADDR 0x53

#define REG_DEVID       0x00
#define REG_BW_RATE     0x2C
#define REG_POWER_CTL   0x2D
#define REG_DATA_FORMAT 0x31
#define REG_DATAX0      0x32

int step_count = 0;
int initialized = 0;
float prev_accel = 0.0f;


void adxl_write(uint8_t reg, uint8_t value) {
    uint8_t buf[2];
    buf[0] = reg;   
    buf[1] = value;   
    i2c_write_blocking(i2c0, ADXL345_ADDR, buf, 2, false);
}

void adxl_read(uint8_t reg, uint8_t *buf, uint8_t len) {
    i2c_write_blocking(i2c0, ADXL345_ADDR, &reg, 1, true);
    i2c_read_blocking(i2c0, ADXL345_ADDR, buf, len, false);
}

void accel_init() {
    gpio_set_function(8, GPIO_FUNC_I2C);
    gpio_set_function(9, GPIO_FUNC_I2C);
    gpio_pull_up(8);
    gpio_pull_up(9);

    i2c_init(i2c0, 400000);

    sleep_ms(200);

    uint8_t id = 0;
    adxl_read(0x00, &id, 1);
    printf("Initialize = 0x%02X\n", id);

    printf("Configuring\n");

    adxl_write(REG_POWER_CTL, 0x00);  
    sleep_ms(10);

    adxl_write(REG_DATA_FORMAT, 0x0B); 
    adxl_write(REG_BW_RATE, 0x0A);   

    adxl_write(REG_POWER_CTL, 0x08); 

    printf("Measurement mode\n");
}

float accel_read_magnitude() {
    uint8_t data[6];
    adxl_read(REG_DATAX0, data, 6);

    int16_t x = (int16_t)((data[1] << 8) | data[0]);
    int16_t y = (int16_t)((data[3] << 8) | data[2]);
    float xf = (float)x * 0.0039f;
    float yf = (float)y * 0.0039f;
    float mag = sqrtf(xf * xf + yf * yf);

    printf("X =%d  Y =%d   MAG=%.3f g\n", x, y, mag);

    return mag;
}

void accel_update_steps() {
    float current = accel_read_magnitude();
    float diff = current - prev_accel;

    if (diff < 0) {
        diff = -diff;
    }

    if (diff > 0.25f && diff < 3.0f) {
        step_count++;
        printf("Step detected, Total Steps = %d\n", step_count );
        sleep_ms(200);
    }

    prev_accel = current;
}

int accel_get_steps() {
    return step_count;
}
