#include "lcd_display.h"
#include "hardware/gpio.h"

// Static variables to store SPI and pin info
static spi_inst_t *lcd_spi = NULL;
static uint8_t lcd_pin_cs = 0;
static uint8_t lcd_pin_dc = 0;
static uint8_t lcd_pin_reset = 0;

// Set CS pin (0 = select, 1 = deselect)
static void lcd_cs_select(bool level) {
    gpio_put(lcd_pin_cs, level);
}

// Set DC pin (0 = command, 1 = data)
static void lcd_dc_set(bool level) {
    gpio_put(lcd_pin_dc, level);
}

// Write data to SPI bus
static void lcd_spi_write(const uint8_t *data, size_t len) {
    lcd_cs_select(0);
    spi_write_blocking(lcd_spi, data, len);
    lcd_cs_select(1);
}

// Initialize LCD: setup SPI, configure pins, reset, and send init commands
int lcd_init(const lcd_config_t *config) {
    if (config == NULL) {
        return -1;
    }

    // Save pin numbers
    lcd_pin_cs = config->pin_cs;
    lcd_pin_dc = config->pin_dc;
    lcd_pin_reset = config->pin_reset;

    // Pick SPI instance
    if (config->spi_inst == 0) {
        lcd_spi = spi0;
    } else if (config->spi_inst == 1) {
        lcd_spi = spi1;
    } else {
        return -1;
    }

    // Setup SPI (mode 0, 8 bits, MSB first)
    spi_init(lcd_spi, config->spi_baudrate);
    spi_set_format(lcd_spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    // Setup CS pin (default high = deselected)
    gpio_init(lcd_pin_cs);
    gpio_set_dir(lcd_pin_cs, GPIO_OUT);
    gpio_put(lcd_pin_cs, 1);

    // Setup DC pin (default low = command mode)
    gpio_init(lcd_pin_dc);
    gpio_set_dir(lcd_pin_dc, GPIO_OUT);
    gpio_put(lcd_pin_dc, 0);

    // Setup RESET pin (default high = not reset)
    gpio_init(lcd_pin_reset);
    gpio_set_dir(lcd_pin_reset, GPIO_OUT);
    gpio_put(lcd_pin_reset, 1);

    // Set SPI pins to SPI function
    gpio_set_function(config->pin_sck, GPIO_FUNC_SPI);
    gpio_set_function(config->pin_mosi, GPIO_FUNC_SPI);
    if (config->pin_miso != 255) {
        gpio_set_function(config->pin_miso, GPIO_FUNC_SPI);
    }

    // Hardware reset: pull reset low, wait, then high
    gpio_put(lcd_pin_reset, 0);
    sleep_ms(10);
    gpio_put(lcd_pin_reset, 1);
    sleep_ms(120);

    // Send init sequence (replace these with actual AIP31068L commands)
    lcd_command(0x01);  // Software reset
    sleep_ms(5);
    
    lcd_command(0x11);  // Exit sleep
    sleep_ms(120);
    
    lcd_command(0x29);  // Display on
    sleep_ms(20);

    return 0;
}

// Send command byte (DC = 0)
void lcd_command(uint8_t cmd) {
    lcd_dc_set(0);
    lcd_spi_write(&cmd, 1);
    lcd_dc_set(0);
}

// Send data byte (DC = 1)
void lcd_data(uint8_t data) {
    lcd_dc_set(1);
    lcd_spi_write(&data, 1);
    lcd_dc_set(0);
}

// Clear screen with solid color
void lcd_clear(uint16_t color) {
    uint8_t color_high = (color >> 8) & 0xFF;
    uint8_t color_low = color & 0xFF;

    // Set column address (replace 0x2A with actual command from datasheet)
    lcd_command(0x2A);
    lcd_data(0x00);  // Start X high
    lcd_data(0x00);   // Start X low
    lcd_data(0x00);  // End X high (adjust for your LCD width)
    lcd_data(0xEF);  // End X low (adjust for your LCD width)

    // Set row address (replace 0x2B with actual command from datasheet)
    lcd_command(0x2B);
    lcd_data(0x00);  // Start Y high
    lcd_data(0x00);  // Start Y low
    lcd_data(0x01);  // End Y high (adjust for your LCD height)
    lcd_data(0x3F);  // End Y low (adjust for your LCD height)

    // Memory write command (replace 0x2C with actual command from datasheet)
    lcd_command(0x2C);

    // Send color for each pixel
    const uint32_t total_pixels = 320 * 240;  // Adjust for your LCD size
    const uint8_t pixel_data[] = {color_high, color_low};
    
    lcd_dc_set(1);
    lcd_cs_select(0);
    
    for (uint32_t i = 0; i < total_pixels; i++) {
        spi_write_blocking(lcd_spi, pixel_data, 2);
    }
    
    lcd_cs_select(1);
    lcd_dc_set(0);
}
