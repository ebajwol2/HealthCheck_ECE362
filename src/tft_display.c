#include "tft_display.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <stdio.h>
#include "font5x7.h"

// SPI1 pins
#define LCD_SPI_INST spi0
#define LCD_PIN_CS     17
#define LCD_PIN_DC     20
#define LCD_PIN_RESET  21
#define LCD_PIN_SCK    18
#define LCD_PIN_MOSI   19

// ===== Helpers =====
static void lcd_cs(int s) { gpio_put(LCD_PIN_CS, s ? 0 : 1); }
static void lcd_dc(int d) { gpio_put(LCD_PIN_DC, d); }

static void lcd_cmd(uint8_t cmd) {
    lcd_dc(0);
    lcd_cs(1);
    spi_write_blocking(LCD_SPI_INST, &cmd, 1);
    lcd_cs(0);
}

static void lcd_data(uint8_t data) {
    lcd_dc(1);
    lcd_cs(1);
    spi_write_blocking(LCD_SPI_INST, &data, 1);
    lcd_cs(0);
}

static void lcd_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    lcd_cmd(0x2A);  // Column address
    lcd_data(x0 >> 8); 
    lcd_data(x0 & 0xFF);
    lcd_data(x1 >> 8); 
    lcd_data(x1 & 0xFF);

    lcd_cmd(0x2B);  // Row address
    lcd_data(y0 >> 8); 
    lcd_data(y0 & 0xFF);
    lcd_data(y1 >> 8); 
    lcd_data(y1 & 0xFF);

    lcd_cmd(0x2C);  // Memory write
}

static void lcd_fill(uint16_t color) {
    lcd_set_window(0, 0, 239, 319);

    uint8_t hi = color >> 8, lo = color;
    uint8_t buf[2] = {hi, lo};

    lcd_dc(1);
    lcd_cs(1);
    for (int i = 0; i < 240 * 320; i++)
        spi_write_blocking(LCD_SPI_INST, buf, 2);
    lcd_cs(0);
}

static void lcd_char(int x, int y, char c, uint16_t fg, uint16_t bg) {
    if (c < 32 || c > 126) c = '?';
    const uint8_t *glyph = font5x7[c - 32];

    lcd_set_window(x, y, x + 4, y + 6);

    uint8_t fgH = fg >> 8, fgL = fg & 0xFF;
    uint8_t bgH = bg >> 8, bgL = bg & 0xFF;

    lcd_dc(1);
    lcd_cs(1);

    // Each column is stored in one byte, bits represent rows
    for (int row = 0; row < 7; row++) {
        for (int col = 0; col < 5; col++) {
            // Extract bit for this row/col
            uint8_t byte_val = glyph[col];
            int bit = (byte_val >> row) & 0x01;
            
            if (bit) {
                spi_write_blocking(LCD_SPI_INST, (uint8_t[]){fgH, fgL}, 2);
            } else {
                spi_write_blocking(LCD_SPI_INST, (uint8_t[]){bgH, bgL}, 2);
            }
        }
    }

    lcd_cs(0);
}

static void lcd_fill_box(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
    lcd_set_window(x0, y0, x1, y1);
    
    uint8_t hi = color >> 8, lo = color & 0xFF;
    uint8_t buf[2] = {hi, lo};
    
    lcd_dc(1);
    lcd_cs(1);
    uint32_t pixels = (uint32_t)(x1 - x0 + 1) * (y1 - y0 + 1);
    for (uint32_t i = 0; i < pixels; i++) {
        spi_write_blocking(LCD_SPI_INST, buf, 2);
    }
    lcd_cs(0);
}

static void lcd_text_large(int x, int y, const char *s, uint16_t fg, uint16_t bg) {
    // Fast 2x scaled rendering using batch fills
    int scale = 2;
    
    while (*s) {
        char c = *s;
        if (c < 32 || c > 126) c = '?';
        const uint8_t *glyph = font5x7[c - 32];

        // Draw each bit as a 2x2 scaled block
        for (int row = 0; row < 7; row++) {
            for (int col = 0; col < 5; col++) {
                // Extract bit: each column byte contains 7 bits (one per row)
                uint8_t byte_val = glyph[col];
                int bit = (byte_val >> row) & 0x01;
                uint16_t color = bit ? fg : bg;
                
                int x0 = x + col * scale;
                int y0 = y + row * scale;
                lcd_fill_box(x0, y0, x0 + scale - 1, y0 + scale - 1, color);
            }
        }
        x += (5 + 1) * scale;
        s++;
    }
}

static void lcd_text(int x, int y, const char *s, uint16_t fg, uint16_t bg) {
    while (*s) {
        lcd_char(x, y, *s, fg, bg);
        x += 6;
        s++;
    }
}

// ===== Public functions =====

void tft_init() {
    spi_init(LCD_SPI_INST, 8000000);
    spi_set_format(LCD_SPI_INST, 8, 0, 0, SPI_MSB_FIRST);

    gpio_init(LCD_PIN_CS); gpio_set_dir(LCD_PIN_CS, GPIO_OUT); gpio_put(LCD_PIN_CS, 1);
    gpio_init(LCD_PIN_DC); gpio_set_dir(LCD_PIN_DC, GPIO_OUT);
    gpio_init(LCD_PIN_RESET); gpio_set_dir(LCD_PIN_RESET, GPIO_OUT);

    gpio_set_function(LCD_PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(LCD_PIN_MOSI, GPIO_FUNC_SPI);

    // Reset sequence
    gpio_put(LCD_PIN_RESET, 0); sleep_ms(20);
    gpio_put(LCD_PIN_RESET, 1); sleep_ms(150);

    // ILI9341 init sequence
    lcd_cmd(0x01); sleep_ms(10);      // Software reset
    lcd_cmd(0x11); sleep_ms(120);     // Sleep out
    
    // Power control A
    lcd_cmd(0xCB);
    lcd_data(0x39); lcd_data(0x2C); lcd_data(0x00); lcd_data(0x34); lcd_data(0x02);
    
    // Power control B
    lcd_cmd(0xCF);
    lcd_data(0x00); lcd_data(0xC1); lcd_data(0x30);
    
    // Driver timing control A
    lcd_cmd(0xE8);
    lcd_data(0x85); lcd_data(0x00); lcd_data(0x78);
    
    // Driver timing control B
    lcd_cmd(0xEA);
    lcd_data(0x00); lcd_data(0x00);
    
    // Power on sequence control
    lcd_cmd(0xED);
    lcd_data(0x64); lcd_data(0x03); lcd_data(0x12); lcd_data(0x81);
    
    // Pump ratio control
    lcd_cmd(0xF7);
    lcd_data(0x20);
    
    // Display function control
    lcd_cmd(0xB6);
    lcd_data(0x0A); lcd_data(0xA2);
    
    // Gamma set
    lcd_cmd(0x26);
    lcd_data(0x01);
    
    // Positive gamma correction
    lcd_cmd(0xE0);
    lcd_data(0x0F); lcd_data(0x31); lcd_data(0x2B); lcd_data(0x0C); lcd_data(0x0E);
    lcd_data(0x08); lcd_data(0x4E); lcd_data(0xF1); lcd_data(0x37); lcd_data(0x07);
    lcd_data(0x10); lcd_data(0x03); lcd_data(0x0E); lcd_data(0x09); lcd_data(0x00);
    
    // Negative gamma correction
    lcd_cmd(0xE1);
    lcd_data(0x00); lcd_data(0x0E); lcd_data(0x14); lcd_data(0x03); lcd_data(0x11);
    lcd_data(0x07); lcd_data(0x31); lcd_data(0xC1); lcd_data(0x48); lcd_data(0x08);
    lcd_data(0x0F); lcd_data(0x0C); lcd_data(0x31); lcd_data(0x36); lcd_data(0x0F);
    
    // Memory access control (rotation)
    lcd_cmd(0x36);
    lcd_data(0x08);
    
    // Pixel format
    lcd_cmd(0x3A);
    lcd_data(0x55);  // 16-bit/pixel
    
    // Frame rate
    lcd_cmd(0xB1);
    lcd_data(0x00); lcd_data(0x10);
    
    // Display on
    lcd_cmd(0x29); sleep_ms(20);

    tft_clear(0x0000);
}

void tft_clear(uint16_t color) {
    lcd_fill(color);
}

void tft_show_heart(int bpm) {
    tft_clear(0x0000);
    char b[32];
    snprintf(b, sizeof(b), "Heart: %d BPM", bpm);
    lcd_text_large(20, 100, b, 0xF800, 0x0000);  // Red on black
}

void tft_show_temp(float t) {
    tft_clear(0x001F);
    char b[32];
    snprintf(b, sizeof(b), "Temp: %.1f C", t);
    lcd_text_large(30, 100, b, 0xFFFF, 0x001F);  // White on blue
}

void tft_show_steps(int s) {
    tft_clear(0x07E0);
    char b[32];
    snprintf(b, sizeof(b), "Steps: %d", s);
    lcd_text_large(40, 100, b, 0x0000, 0x07E0);  // Black on green
}
