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
#define LED_R 37
#define LED_G 38
#define LED_B 39


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
// Buzzer in LED
// ----------------------------
static void rgb_off() {
    gpio_put(LED_R, 1);
    gpio_put(LED_G, 1);
    gpio_put(LED_B, 1);
}

static void rgb_red() {
    gpio_put(LED_R, 0);
    gpio_put(LED_G, 1);
    gpio_put(LED_B, 1);
}

static void rgb_green() {
    gpio_put(LED_R, 1);
    gpio_put(LED_G, 0);
    gpio_put(LED_B, 1);
}

static void rgb_blue() {
    gpio_put(LED_R, 1);
    gpio_put(LED_G, 1);
    gpio_put(LED_B, 0);
}

static void rgb_yellow() {
    gpio_put(LED_R, 0);
    gpio_put(LED_G, 0);
    gpio_put(LED_B, 1);
}

static void check_alarm() {
    bool temp_alert  = (temperature > 28.0f);
    bool heart_alert = (heart_rate > 120);

    if (temp_alert && heart_alert) {
        // RED-YELLOW rapid alert
        rgb_red();
        sleep_ms(40);
        rgb_yellow();
        sleep_ms(40);
        return;
    }

    if (temp_alert) {
        // Blue flash for fever
        rgb_blue();
        sleep_ms(60);
        rgb_off();
        sleep_ms(60);
        return;
    }

    if (heart_alert) {
        // Red heartbeat flashing pattern
        rgb_red();
        sleep_ms(80);
        rgb_off();
        sleep_ms(80);
        return;
    }

    // Safe
    rgb_off();
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
    gpio_init(LED_R);
    gpio_set_dir(LED_R, GPIO_OUT);
    gpio_put(LED_R, 1);   // off (common anode)

    gpio_init(LED_G);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_put(LED_G, 1);

    gpio_init(LED_B);
    gpio_set_dir(LED_B, GPIO_OUT);
    gpio_put(LED_B, 1);


    // Button
    configure_button_pin();

    // Show first screen
    render();

    uint64_t last_refresh = time_us_64();

    while (1) {
        // Update accelerometer step count
        accel_update_steps();

        // Read heart rate (raw)
        uint32_t heart_raw = max30102_read_red();
        heart_rate = heart_raw;  // You may refine later

        uint64_t now = time_us_64();

        // Refresh display every 20 ms = 20,000 µs
        if (now - last_refresh >= 20000) {
            last_refresh = now;
            render();  // redraw current screen
        }

        // Button press then switch screen
        if (button_pressed) {
            button_pressed = false;

            current_screen = (current_screen + 1) % 3;
            render();

            gpio_put(LED_PIN, 1);
            sleep_ms(80);
            gpio_put(LED_PIN, 0);
        }

        // Buzzer
        check_alarm();
}

}
