#include "lcd_display.h"
#include <stdio.h>

// LCD pin definitions - adjust these in your main.c
const int LCD_SPI_INST = 1;
const int LCD_PIN_CS = 13;
const int LCD_PIN_DC = 14;
const int LCD_PIN_RESET = 15;
const int LCD_PIN_SCK = 10;
const int LCD_PIN_MOSI = 11;

static spi_inst_t *lcd_spi = NULL;

// Helper functions
static void lcd_cs(bool select) {
    gpio_put(LCD_PIN_CS, select ? 0 : 1);
}

static void lcd_dc(bool data) {
    gpio_put(LCD_PIN_DC, data ? 1 : 0);
}

// Initialize LCD
void lcd_init() {
    // Pick SPI instance
    lcd_spi = (LCD_SPI_INST == 0) ? spi0 : spi1;

    // Setup SPI: 8 MHz, 8-bit, mode 0
    spi_init(lcd_spi, 8000000);
    spi_set_format(lcd_spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    // Setup GPIO pins
    gpio_init(LCD_PIN_CS);
    gpio_set_dir(LCD_PIN_CS, GPIO_OUT);
    gpio_put(LCD_PIN_CS, 1);

    gpio_init(LCD_PIN_DC);
    gpio_set_dir(LCD_PIN_DC, GPIO_OUT);
    gpio_put(LCD_PIN_DC, 0);

    gpio_init(LCD_PIN_RESET);
    gpio_set_dir(LCD_PIN_RESET, GPIO_OUT);
    gpio_put(LCD_PIN_RESET, 1);

    // Set SPI pins
    gpio_set_function(LCD_PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(LCD_PIN_MOSI, GPIO_FUNC_SPI);

    // Hardware reset
    gpio_put(LCD_PIN_RESET, 0);
    sleep_ms(10);
    gpio_put(LCD_PIN_RESET, 1);
    sleep_ms(120);

    // Init sequence (update with AIP31068L commands from datasheet)
    lcd_command(0x01);
    sleep_ms(5);
    lcd_command(0x11);
    sleep_ms(120);
    lcd_command(0x29);
    sleep_ms(20);

    lcd_clear(0x0000);
}

// Send command
void lcd_command(uint8_t cmd) {
    lcd_dc(0);
    lcd_cs(1);
    spi_write_blocking(lcd_spi, &cmd, 1);
    lcd_cs(0);
}

// Send data
void lcd_data(uint8_t data) {
    lcd_dc(1);
    lcd_cs(1);
    spi_write_blocking(lcd_spi, &data, 1);
    lcd_cs(0);
}

// Clear screen with color (RGB565)
void lcd_clear(uint16_t color) {
    uint8_t color_high = (color >> 8) & 0xFF;
    uint8_t color_low = color & 0xFF;

    // Set column address (update 0x2A with actual command)
    lcd_command(0x2A);
    lcd_data(0x00);
    lcd_data(0x00);
    lcd_data(0x01);
    lcd_data(0x3F);

    // Set row address (update 0x2B with actual command)
    lcd_command(0x2B);
    lcd_data(0x00);
    lcd_data(0x00);
    lcd_data(0x01);
    lcd_data(0xEF);

    // Memory write (update 0x2C with actual command)
    lcd_command(0x2C);

    // Fill screen
    const uint32_t total_pixels = 320 * 240;
    const uint8_t pixel_data[] = {color_high, color_low};

    lcd_dc(1);
    lcd_cs(1);
    for (uint32_t i = 0; i < total_pixels; i++) {
        spi_write_blocking(lcd_spi, pixel_data, 2);
    }
    lcd_cs(0);
    lcd_dc(0);
}

// Display heart rate
void lcd_display_heart_rate(int heart_rate) {
    // Clear area and draw text
    // Use font rendering to show: "Heart Rate: XX BPM"
    // Example layout at y=20
}

// Display temperature
void lcd_display_temperature(float temp) {
    // Clear area and draw text
    // Use font rendering to show: "Temp: XX.X C"
    // Example layout at y=60
}

// Display steps
void lcd_display_steps(int steps) {
    // Clear area and draw text
    // Use font rendering to show: "Steps: XXXX"
    // Example layout at y=100
}

// Update all health data on screen
void lcd_update_display(int heart_rate, float temperature, int steps) {
    lcd_clear(0x0000);
    lcd_display_heart_rate(heart_rate);
    lcd_display_temperature(temperature);
    lcd_display_steps(steps);
}
