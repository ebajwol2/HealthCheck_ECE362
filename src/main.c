#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "hardware/regs/pads_bank0.h"
#include "hardware/regs/io_bank0.h"
#include "tft_display.h"
#include "MAX30205.h"
#include "ADX345.h"
#include "MAX30102.h"

// Button + LED pins
#define BUTTON_PIN 26
#define LED_PIN    25

typedef enum { 
    SCREEN_HEART = 0, 
    SCREEN_TEMP  = 1, 
    SCREEN_STEPS = 2 
} screen_t;

static volatile screen_t current_screen = SCREEN_HEART;
static volatile bool button_pressed = false;

// Dummy sensor values
static int heart_rate = 72;
// static float temperature = 36.5f;
static int steps = 1200;

// ----------------------------
// screen
// ----------------------------
static void render() {
    switch (current_screen) {
    case SCREEN_HEART:
        tft_show_heart(heart_rate);
        break;

    case SCREEN_TEMP:
        tft_show_temp(temperature);
        break;

    case SCREEN_STEPS:
        tft_show_steps(steps);
        break;
    }
}

// ----------------------------
// Button interrupt
// ----------------------------
static void button_isr(uint gpio, uint32_t events) {
    static uint32_t last = 0;
    uint32_t now = to_ms_since_boot(get_absolute_time());

    if (now - last < 200) return;  // debounce
    last = now;

    // Only trigger when the pin is actually pulled to GND (active-low).
    int level = gpio_get(gpio);
    printf("ISR gpio=%d level=%d\n", gpio, level);
    if (level == 0) {
        printf("Button pressed (active-low) on GPIO %d\n", gpio);
        button_pressed = true;
    } else {
        // Ignore if pin is high (VCC). This ensures we react only when at GND.
        printf("Ignored (pin high) on GPIO %d\n", gpio);
    }
}

// ----------------------------
// Configure GPIO26
// ----------------------------
static void configure_button_pin() {
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);

    // Disable any conflicting pulls first
    gpio_disable_pulls(BUTTON_PIN);

    // Add weak pull-up
    gpio_pull_up(BUTTON_PIN);

    // FIX HARDWARE PAD CONFIGURATION:

    hw_write_masked(
        &pads_bank0_hw->io[BUTTON_PIN],
        PADS_BANK0_GPIO0_IE_BITS,            // Input enable bit
        PADS_BANK0_GPIO0_IE_BITS  |
        PADS_BANK0_GPIO0_OD_BITS  |          // Allow open-drain, disables push-pull
        PADS_BANK0_GPIO0_DRIVE_BITS |
        PADS_BANK0_GPIO0_PUE_BITS |
        PADS_BANK0_GPIO0_PDE_BITS
    );

    // Set falling-edge IRQ
    gpio_set_irq_enabled_with_callback(
        BUTTON_PIN,
        GPIO_IRQ_EDGE_FALL,
        true,
        &button_isr
    );
}

// ----------------------------
// Main
// ----------------------------
int main() {
    stdio_init_all();
    sleep_ms(1000);

    printf("TFT Display UI starting...\n");

    // TFT setup
    tft_init();
    MAX30205_init_i2c();
    MAX30205_init_timer();
    accel_init();
    max30102_init();

    // LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Button
    configure_button_pin();

    // Show first screen
    render();

    while (1) {
        accel_update_steps();
        uint32_t heart = max30102_read_red();
       printf("Heart: %lu\n", heart);
        if (button_pressed) {
            button_pressed = false;
            current_screen = (current_screen + 1) % 3;
            printf("Screen changed to: %d\n", current_screen);

            render();

            gpio_put(LED_PIN, 1);
            sleep_ms(80);
            gpio_put(LED_PIN, 0);
        }

        sleep_ms(10);
    }
}
