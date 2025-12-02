#ifndef ADXL345_H
#define ADXL345_H

#include "pico/stdlib.h"
#include <math.h>

void accel_init();
void accel_update_steps();
int accel_get_steps();
float accel_read_magnitude();
void adxl_write(uint8_t reg, uint8_t value);
void adxl_read(uint8_t reg, uint8_t *buf, uint8_t len);

#endif
