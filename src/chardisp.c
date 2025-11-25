#include "chardisp.h"
#include "hardware/spi.h"

// These match the Proton board lab kit
const int SPI_DISP_SCK = 18;
const int SPI_DISP_TX  = 19;
const int SPI_DISP_CSn = 17;

void init_chardisp_pins() {
    spi_inst_t *spi = spi0;

    gpio_set_function(SPI_DISP_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI_DISP_TX,  GPIO_FUNC_SPI);
    gpio_set_function(SPI_DISP_CSn, GPIO_FUNC_SPI);

    spi_init(spi, 10000); // slow
    spi_set_format(spi, 9, 0, 0, SPI_MSB_FIRST); // 9-bit frames
}

static void send_cmd(uint8_t value) {
    spi_inst_t *spi = spi0;
    uint16_t frame = (0 << 8) | value;
    spi_write16_blocking(spi, &frame, 1);
    sleep_us(40);
}

static void send_data(uint8_t value) {
    spi_inst_t *spi = spi0;
    uint16_t frame = (1 << 8) | value;
    spi_write16_blocking(spi, &frame, 1);
    sleep_us(40);
}

void cd_init() {
    sleep_ms(20);

    send_cmd(0x38);
    send_cmd(0x0C);
    send_cmd(0x01);
    sleep_ms(2);
    send_cmd(0x06);
}

void cd_display1(const char *s) {
    send_cmd(0x80);
    for (int i = 0; i < 16 && s[i]; i++) send_data(s[i]);
}

void cd_display2(const char *s) {
    send_cmd(0xC0);
    for (int i = 0; i < 16 && s[i]; i++) send_data(s[i]);
}
