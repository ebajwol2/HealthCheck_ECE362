#ifndef __MAX30205_H__
#define __MAX30205_H__

volatile float temperature;

void MAX30205_init_i2c();
void MAX30205_check_address();
void read_temperature();
void MAX30205_init_timer();
#endif