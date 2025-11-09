#include <stdio.h>
#include <stdlib.h>
#include "MAX30205.h"

#define MAX30205_TEST

int main()
{

    #ifdef MAX30205_TEST
    MAX30205_init_i2c();
    MAX30205_check_address();

    #endif

    return 0;
}