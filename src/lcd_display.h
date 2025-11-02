#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

// LCD pin definitions - adjust these to match your hardware
extern const int LCD_SPI_INST;     // SPI instance (0 or 1)
extern const int LCD_PIN_CS;       // Chip Select
extern const int LCD_PIN_DC;       // Data/Command
extern const int LCD_PIN_RESET;    // Reset
extern const int LCD_PIN_SCK;      // SPI Clock
extern const int LCD_PIN_MOSI;     // SPI MOSI

// Initialize LCD with SPI
void lcd_init();

// Send command byte
void lcd_command(uint8_t cmd);

// Send data byte
void lcd_data(uint8_t data);

// Clear screen with color (RGB565)
void lcd_clear(uint16_t color);

// Display health data on screen
void lcd_display_heart_rate(int heart_rate);
void lcd_display_temperature(float temp);
void lcd_display_steps(int steps);

// Update entire display with all health data
void lcd_update_display(int heart_rate, float temperature, int steps);

#endif
