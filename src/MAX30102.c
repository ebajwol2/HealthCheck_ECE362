#include "MAX30102.h"
#include <stdbool.h>
#include <stdint.h>

const int I2C_SDA = 32;
const int I2C_SCL = 33; 
const uint8_t SENSOR_ADDRESS = 0x57;
uint32_t heart_rate = 0;
uint32_t sample = 0;

// PBA algorithm variables
int16_t IR_AC_Max = 20;
int16_t IR_AC_Min = -20;

int16_t IR_AC_Signal_Current = 0;
int16_t IR_AC_Signal_Previous = 0;
int16_t IR_AC_Signal_min = 0;
int16_t IR_AC_Signal_max = 0;
int16_t IR_Average_Estimated = 0;

int16_t positiveEdge = 0;
int16_t negativeEdge = 0;
int32_t ir_avg_reg = 0;

int16_t cbuf[32] = {0};
uint8_t offset = 0;

static const uint16_t FIRCoeffs[12] = {172, 321, 579, 927, 1360, 1858, 2390, 2916, 3391, 3768, 4012, 4096};

//Determines the DC so it can subtracted from the sample
int16_t averageDCEstimator(int32_t *p, int32_t x) {
    *p += (((x << 15) - *p) >> 4);
    return (*p >> 15);
}

int32_t mul16(int16_t x, int16_t y) {
    return ((int32_t)x * (int32_t)y);
}

int16_t lowPassFIRFilter(int16_t din) {
    cbuf[offset] = din;

    int32_t z = mul16(FIRCoeffs[11], cbuf[(offset - 11) & 0x1F]);
    //convolution
    for (uint8_t i = 0; i < 11; i++) {
        z += mul16(FIRCoeffs[i], cbuf[(offset - i) & 0x1F] + cbuf[(offset - 22 + i) & 0x1F]);
    }

    offset = (offset + 1) & 0x1F;
    return (int16_t)(z >> 15);
}

// Returns true if a heartbeat is detected
bool checkForBeat(int32_t sample) {
    bool beatDetected = false;

    IR_AC_Signal_Previous = IR_AC_Signal_Current;

    IR_Average_Estimated = averageDCEstimator(&ir_avg_reg, sample);
    IR_AC_Signal_Current = lowPassFIRFilter(sample - IR_Average_Estimated);

    // Positive zero crossing
    if ((IR_AC_Signal_Previous < 0) && (IR_AC_Signal_Current >= 0)) {
        IR_AC_Max = IR_AC_Signal_max;
        IR_AC_Min = IR_AC_Signal_min;

        positiveEdge = 1;
        negativeEdge = 0;
        IR_AC_Signal_max = 0;

        if ((IR_AC_Max - IR_AC_Min) > 20 && (IR_AC_Max - IR_AC_Min) < 1000) {
            beatDetected = true;
        }
    }

    // Negative zero crossing
    if ((IR_AC_Signal_Previous > 0) && (IR_AC_Signal_Current <= 0)) {
        positiveEdge = 0;
        negativeEdge = 1;
        IR_AC_Signal_min = 0;
    }

    // Track max in positive cycle
    if (positiveEdge && IR_AC_Signal_Current > IR_AC_Signal_Previous) {
        IR_AC_Signal_max = IR_AC_Signal_Current;
    }

    // Track min in negative cycle
    if (negativeEdge && IR_AC_Signal_Current < IR_AC_Signal_Previous) {
        IR_AC_Signal_min = IR_AC_Signal_Current;
    }

    return beatDetected;
}

void init_i2c() {
    i2c_init(i2c0, 400 * 1000); 
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
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

uint64_t last_peak = 0;
double intervals[8] = {0};
int int_count = 0;
int int_i = 0;

uint64_t total_bpm_sum = 0;
uint32_t total_beats = 0;

void max30102_read_red() {
    uint8_t reg = 0x07;
    uint8_t raw[3];

    i2c_write_blocking(i2c0, SENSOR_ADDRESS, &reg, 1, true);
    i2c_read_blocking(i2c0, SENSOR_ADDRESS, raw, 3, false);

    sample = ((raw[0] << 16) | (raw[1] << 8) | raw[2]) & 0x3FFFF;

    if (checkForBeat(sample)) {
        uint64_t now = time_us_64();
        if (last_peak != 0) {
            double dt = (now - last_peak) / 1e6; // seconds
            double bpm = 60.0 / dt;

            // Add to sum and increment beat count
            total_bpm_sum += bpm;
            total_beats++;

            // Display the average of all BPMs so far
            heart_rate = (uint32_t)(total_bpm_sum / total_beats);
        }
        last_peak = now;
    }
    if(sample < 10000){
        total_bpm_sum = 0;
        total_beats = 0;
        heart_rate = 0;
    }
}

/*
int main() {
    stdio_init_all();
    sleep_ms(2000);

    init_i2c();
    max30102_init();

    printf("MAX30102 Initialized.\n");

    while (1) {
        max30102_read_red();
        printf("Sample: %lu ---- BPM: %lu\n", sample, heart_rate);
        sleep_ms(10);
    }

    return 0;
}
*/

