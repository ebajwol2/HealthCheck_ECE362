#ifndef CHARDISP_H
#define CHARDISP_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

// Make sure to define SPI_DISP_SCK, SPI_DISP_CSn, SPI_DISP_TX in main.c
void init_chardisp_pins();
void send_spi_cmd(spi_inst_t* spi, uint16_t value);
void send_spi_data(spi_inst_t* spi, uint16_t value);
void cd_init();
void cd_display1(const char *str);
void cd_display2(const char *str);

#endif
