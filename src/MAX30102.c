#include "MAX30102.h"
const int I2C_SDA = 8;
const int I2C_SCL = 9; 
const uint8_t SENSOR_ADDRESS = 0x57;
#define SAMPLE_RATE   100
#define SAMPLE_PERIOD (1000 / SAMPLE_RATE)

void init_i2c() {
    i2c_init(i2c0, 400 * 1000); 
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    // gpio_pull_up(I2C_SDA);
    // gpio_pull_up(I2C_SCL);
}

void max30102_init() {
    uint8_t fifo_cfg[] = {0x08, 0x04};
    i2c_write_blocking(i2c0, SENSOR_ADDRESS, fifo_cfg, 2, false);

    uint8_t mode_cfg[] = {0x09, 0x02};
    i2c_write_blocking(i2c0, SENSOR_ADDRESS, mode_cfg, 2, false);

    uint8_t spo2_cfg[] = {0x0A, 0x27};
    i2c_write_blocking(i2c0, SENSOR_ADDRESS, spo2_cfg, 2, false);

    uint8_t led_cfg[] = {0x0C, 0x24};
    i2c_write_blocking(i2c0, SENSOR_ADDRESS, led_cfg, 2, false);
}

uint32_t max30102_read_red() {
    double prev1 = 0;
    double prev2 = 0;
    uint64_t last_peak = 0;

    double intervals[8] = {0};
    int int_count = 0;
    int int_i = 0;
    uint32_t true_bpm = 0;
    uint8_t reg = 0x07;    
    uint8_t raw[3];

    i2c_write_blocking(i2c0, SENSOR_ADDRESS, &reg, 1, true);
    i2c_read_blocking(i2c0, SENSOR_ADDRESS, raw, 3, false);

    uint32_t sample = ((raw[0] << 16) | (raw[1] << 8) | raw[2]) & 0x3FFFF;
    double val = sample - 50000;  

    double cur = val;
    uint64_t now = time_us_64();

    bool is_peak =
        (prev1 > prev2) &&
        (prev1 > cur) &&
        (prev1 > 20000); // threshold
    uint32_t bpm_filter = sample;

    if (is_peak) {
        if (last_peak != 0) {
            double dt = (now - last_peak) / 1e6; // seconds
            intervals[int_i] = dt;

            if (int_count < 8) int_count++;
            int_i = (int_i + 1) % 8;

            double sum = 0;
            for (int i = 0; i < int_count; i++) sum += intervals[i];
            double bpm = 60.0 / (sum / int_count);
            sum = bpm; //r
            //printf("BPM: %.1f\n", bpm);
        }
        last_peak = now;
    }
    if(bpm_filter < 20000){
        true_bpm = 0;
    }else{
        true_bpm = bpm_filter/2000;
    }
    prev2 = prev1;
    prev1 = cur;
    
    return true_bpm;
}
