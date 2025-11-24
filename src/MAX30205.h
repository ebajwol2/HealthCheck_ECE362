#ifndef __MAX30205_H__
#define __MAX30205_H__

void MAX30205_init_i2c();
void MAX30205_check_address();
float read_temperature();
#endif