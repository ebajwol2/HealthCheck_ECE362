/**
 * @file lcd_display.h
 * @brief Header file for AIP31068L TFT LCD display driver
 * 
 * This driver provides basic functionality for initializing and controlling
 * the AIP31068L TFT LCD display via SPI communication on the RP2350.
 * 
 * @author HealthCheck Team
 * @date 2025
 */

#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

/**
 * @brief LCD display configuration structure
 * 
 * This structure holds the GPIO pin assignments and SPI configuration
 * for the LCD display. Adjust these pin numbers based on your hardware setup.
 */
typedef struct {
    uint8_t spi_inst;      ///< SPI instance (0 or 1 on RP2350)
    uint8_t pin_cs;        ///< Chip Select (CS) GPIO pin
    uint8_t pin_dc;        ///< Data/Command (DC) GPIO pin
    uint8_t pin_reset;     ///< Reset GPIO pin
    uint8_t pin_sck;       ///< SPI Clock (SCK) GPIO pin
    uint8_t pin_mosi;      ///< SPI Master Out Slave In (MOSI) GPIO pin
    uint8_t pin_miso;      ///< SPI Master In Slave Out (MISO) GPIO pin (optional)
    uint32_t spi_baudrate; ///< SPI baudrate (typically 8MHz or higher for TFT LCDs)
} lcd_config_t;

/**
 * @brief Initialize the LCD display
 * 
 * This function:
 * 1. Initializes the SPI interface
 * 2. Configures all GPIO pins (CS, DC, RESET, SCK, MOSI, MISO)
 * 3. Performs a hardware reset of the LCD
 * 4. Sends the initialization sequence to configure the LCD
 * 
 * @param config Pointer to LCD configuration structure with GPIO pin assignments
 * @return 0 on success, -1 on failure
 */
int lcd_init(const lcd_config_t *config);

/**
 * @brief Send a command byte to the LCD
 * 
 * Sets the DC pin low to indicate command mode, then sends
 * the command byte over SPI.
 * 
 * @param cmd Command byte to send (see AIP31068L datasheet for command list)
 */
void lcd_command(uint8_t cmd);

/**
 * @brief Send a data byte to the LCD
 * 
 * Sets the DC pin high to indicate data mode, then sends
 * the data byte over SPI.
 * 
 * @param data Data byte to send
 */
void lcd_data(uint8_t data);

/**
 * @brief Clear the entire LCD screen with a solid color
 * 
 * Fills the entire display with the specified color.
 * The color is a 16-bit RGB565 format value.
 * 
 * Common colors (RGB565 format):
 * - Black:   0x0000
 * - White:   0xFFFF
 * - Red:     0xF800
 * - Green:   0x07E0
 * - Blue:    0x001F
 * 
 * @param color 16-bit color value in RGB565 format
 */
void lcd_clear(uint16_t color);

#endif // LCD_DISPLAY_H

