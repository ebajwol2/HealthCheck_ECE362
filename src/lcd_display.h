#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

// LCD configuration - set your GPIO pins here
typedef struct {
    uint8_t spi_inst;      // SPI instance (0 or 1)
    uint8_t pin_cs;        // Chip Select pin
    uint8_t pin_dc;        // Data/Command pin
    uint8_t pin_reset;     // Reset pin
    uint8_t pin_sck;       // SPI Clock pin
    uint8_t pin_mosi;      // SPI MOSI pin
    uint8_t pin_miso;      // SPI MISO pin (use 255 if not used)
    uint32_t spi_baudrate; // SPI speed (try 8MHz or higher)
} lcd_config_t;

// Initialize LCD with SPI and reset sequence
int lcd_init(const lcd_config_t *config);

// Send command byte to LCD
void lcd_command(uint8_t cmd);

// Send data byte to LCD
void lcd_data(uint8_t data);

// Fill screen with color (RGB565 format)
void lcd_clear(uint16_t color);

#endif

