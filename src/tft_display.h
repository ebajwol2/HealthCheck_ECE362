#ifndef TFT_DISPLAY_H
#define TFT_DISPLAY_H

#include <stdint.h>

void tft_init();
void tft_clear(uint16_t color);

void tft_show_heart(int bpm);
void tft_show_temp(float temp);
void tft_show_steps(int steps);

#endif
