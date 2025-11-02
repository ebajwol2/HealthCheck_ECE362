/**
 * @file lcd_display.c
 * @brief Implementation of AIP31068L TFT LCD display driver
 * 
 * This file implements the basic LCD driver functions for the AIP31068L TFT LCD
 * display using SPI communication on the RP2350 microcontroller.
 * 
 * Reference: AIP31068L datasheet
 * https://ece362-purdue.github.io/proton-labs/assets/kit-components/AIP31068L.pdf
 * 
 * @author HealthCheck Team
 * @date 2025
 */

#include "lcd_display.h"
#include "hardware/gpio.h"
#include <stdio.h>

// Static variable to store the SPI instance and configuration
static spi_inst_t *lcd_spi = NULL;
static uint8_t lcd_pin_cs = 0;
static uint8_t lcd_pin_dc = 0;
static uint8_t lcd_pin_reset = 0;

/**
 * @brief Helper function to set chip select (CS) pin
 * @param level 1 for high (deselect), 0 for low (select)
 */
static void lcd_cs_select(bool level) {
    gpio_put(lcd_pin_cs, level);
}

/**
 * @brief Helper function to set data/command (DC) pin
 * @param level 1 for data mode, 0 for command mode
 */
static void lcd_dc_set(bool level) {
    gpio_put(lcd_pin_dc, level);
}

/**
 * @brief Write data to SPI bus
 * @param data Pointer to data buffer
 * @param len Number of bytes to write
 */
static void lcd_spi_write(const uint8_t *data, size_t len) {
    lcd_cs_select(0);  // Select LCD
    spi_write_blocking(lcd_spi, data, len);
    lcd_cs_select(1);  // Deselect LCD
}

/**
 * @brief Initialize the LCD display
 * 
 * Sets up SPI communication, configures GPIO pins, performs hardware reset,
 * and sends the initialization sequence to the LCD.
 * 
 * @param config Pointer to LCD configuration structure
 * @return 0 on success, -1 on failure
 */
int lcd_init(const lcd_config_t *config) {
    if (config == NULL) {
        return -1;
    }

    // Store configuration for use in other functions
    lcd_pin_cs = config->pin_cs;
    lcd_pin_dc = config->pin_dc;
    lcd_pin_reset = config->pin_reset;

    // Select SPI instance based on configuration
    if (config->spi_inst == 0) {
        lcd_spi = spi0;
    } else if (config->spi_inst == 1) {
        lcd_spi = spi1;
    } else {
        return -1;  // Invalid SPI instance
    }

    // Initialize SPI interface
    // Mode 0: CPOL=0, CPHA=0 (common for TFT LCDs)
    // If the LCD requires a different mode, adjust accordingly
    spi_init(lcd_spi, config->spi_baudrate);
    spi_set_format(lcd_spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    // Initialize GPIO pins
    // CS (Chip Select) - Output, default high (deselected)
    gpio_init(lcd_pin_cs);
    gpio_set_dir(lcd_pin_cs, GPIO_OUT);
    gpio_put(lcd_pin_cs, 1);

    // DC (Data/Command) - Output, default low (command mode)
    gpio_init(lcd_pin_dc);
    gpio_set_dir(lcd_pin_dc, GPIO_OUT);
    gpio_put(lcd_pin_dc, 0);

    // RESET - Output, default high (not reset)
    gpio_init(lcd_pin_reset);
    gpio_set_dir(lcd_pin_reset, GPIO_OUT);
    gpio_put(lcd_pin_reset, 1);

    // Configure SPI pins
    gpio_set_function(config->pin_sck, GPIO_FUNC_SPI);
    gpio_set_function(config->pin_mosi, GPIO_FUNC_SPI);
    if (config->pin_miso != 255) {  // 255 indicates MISO not used
        gpio_set_function(config->pin_miso, GPIO_FUNC_SPI);
    }

    // Perform hardware reset sequence
    // Pull reset low for at least 10ms, then high
    gpio_put(lcd_pin_reset, 0);
    sleep_ms(10);  // Minimum reset pulse duration
    gpio_put(lcd_pin_reset, 1);
    sleep_ms(120);  // Wait for LCD to stabilize after reset

    // Send initialization sequence
    // Note: The exact initialization commands depend on the AIP31068L datasheet
    // These are placeholder commands - replace with actual commands from datasheet
    
    // Software reset command (typically 0x01)
    lcd_command(0x01);
    sleep_ms(5);

    // Exit sleep mode (typically 0x11)
    lcd_command(0x11);
    sleep_ms(120);

    // Display on (typically 0x29)
    lcd_command(0x29);
    sleep_ms(20);

    // Additional initialization commands should be added here
    // based on the AIP31068L datasheet specifications
    
    return 0;
}

/**
 * @brief Send a command byte to the LCD
 * 
 * Sets DC pin low (command mode), sends the command byte over SPI,
 * then returns DC to its default state.
 * 
 * @param cmd Command byte to send
 */
void lcd_command(uint8_t cmd) {
    lcd_dc_set(0);  // Command mode
    lcd_spi_write(&cmd, 1);
    lcd_dc_set(0);  // Return to default (command mode)
}

/**
 * @brief Send a data byte to the LCD
 * 
 * Sets DC pin high (data mode), sends the data byte over SPI,
 * then returns DC to command mode.
 * 
 * @param data Data byte to send
 */
void lcd_data(uint8_t data) {
    lcd_dc_set(1);  // Data mode
    lcd_spi_write(&data, 1);
    lcd_dc_set(0);  // Return to command mode
}

/**
 * @brief Clear the entire LCD screen with a solid color
 * 
 * This function fills the entire display area with the specified color.
 * The color is sent in 16-bit RGB565 format (5 bits red, 6 bits green, 5 bits blue).
 * 
 * Steps:
 * 1. Set the column address range to cover full width
 * 2. Set the row address range to cover full height
 * 3. Send memory write command
 * 4. Send color data for each pixel
 * 
 * Note: The exact commands depend on the AIP31068L datasheet.
 * Replace the placeholder commands (0x2A, 0x2B, 0x2C) with actual commands
 * from the datasheet.
 * 
 * @param color 16-bit color value in RGB565 format
 */
void lcd_clear(uint16_t color) {
    // Extract high and low bytes from 16-bit color
    uint8_t color_high = (color >> 8) & 0xFF;
    uint8_t color_low = color & 0xFF;

    // Set column address (X start and end)
    // These commands are placeholders - replace with actual AIP31068L commands
    lcd_command(0x2A);  // Column Address Set command (typical for TFT LCDs)
    lcd_data(0x00);     // Start column high byte (0)
    lcd_data(0x00);     // Start column low byte (0)
    lcd_data(0x00);     // End column high byte (adjust for actual width, e.g., 0x01 for 320px)
    lcd_data(0xEF);     // End column low byte (adjust for actual width, e.g., 0x3F for 320px)

    // Set row address (Y start and end)
    lcd_command(0x2B);  // Row Address Set command (typical for TFT LCDs)
    lcd_data(0x00);     // Start row high byte (0)
    lcd_data(0x00);     // Start row low byte (0)
    lcd_data(0x01);     // End row high byte (adjust for actual height, e.g., 0x01 for 240px)
    lcd_data(0x3F);     // End row low byte (adjust for actual height, e.g., 0xEF for 240px)

    // Memory write command - start sending pixel data
    lcd_command(0x2C);  // Memory Write command (typical for TFT LCDs)

    // Send color data for all pixels
    // Note: For a typical 320x240 display, we need to send 76,800 pixels (320 * 240)
    // Each pixel requires 2 bytes (16-bit color)
    // For efficiency, we can optimize this later by using DMA or sending in larger chunks
    const uint32_t total_pixels = 320 * 240;  // Adjust based on actual LCD dimensions
    const uint8_t pixel_data[] = {color_high, color_low};
    
    lcd_dc_set(1);  // Data mode
    lcd_cs_select(0);  // Select LCD
    
    for (uint32_t i = 0; i < total_pixels; i++) {
        spi_write_blocking(lcd_spi, pixel_data, 2);
    }
    
    lcd_cs_select(1);  // Deselect LCD
    lcd_dc_set(0);     // Return to command mode
}

