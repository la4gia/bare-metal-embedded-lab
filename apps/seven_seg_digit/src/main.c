/*
 * Wiring
 * QA -> Pin07 (seg A)
 * QB -> Pin06 (seg B)
 * QC -> Pin04 (seg C)
 * QD -> Pin02 (seg D)
 * QE -> Pin01 (seg E)
 * QF -> Pin09 (seg F)
 * QG -> Pin10 (seg G)
 * QH -> Pin05 (seg dp)
 *
 *       A
 *     F   B
 *       G
 *     E   C
 *       D    dp
 */

#include "stm32f407xx.h"
#include "rcc.h"
#include "gpio.h"
#include "board.h"

#define SR_PORT      GPIOA
#define SER_PA_PIN   1 // Serial Input. DS. Data Line
#define SRCLK_PA_PIN 2 // Shift Register CLK. SHCP.
#define RCLK_PA_PIN  3 // Storage Register CLK. STCP.

#define CYCLES_PER_MS     (BOARD_SYSCLK_FREQ / 1000) // 168,000 cycles per ms
#define ITERATIONS_PER_MS (CYCLES_PER_MS / 8) // 8 cycles per iteration, 21,000 iterations per MS
#define TEN_MS            (ITERATIONS_PER_MS * 10)
#define THIRTY_EIGHT_MS   (ITERATIONS_PER_MS * 38)
#define HALF_SECOND       (ITERATIONS_PER_MS * 500)
#define ONE_SECOND        (ITERATIONS_PER_MS * 1000)
#define THREE_SECONDS     (ITERATIONS_PER_MS * 3000)
#define FIVE_SECONDS      (ITERATIONS_PER_MS * 5000)

// wire bytes. 0 = lit (0xA4 = 1010 0100)
// bit 7 is dp (1000 1111)
static const uint8_t digits[10] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};

/* Simple busy-wait delay*/
static void delay(volatile uint32_t count)
{
    while (count--)
        ;
}

static void pulse(void)
{
    gpio_set(SR_PORT, RCLK_PA_PIN);
    delay(THIRTY_EIGHT_MS);
    gpio_clear(SR_PORT, RCLK_PA_PIN);
    delay(THIRTY_EIGHT_MS);
}

static void shift_out(uint8_t value)
{
    int8_t i = 7;
    while (i >= 0) {
        uint8_t bit = (value >> i) & 1;
        if (bit) {
            gpio_set(SR_PORT, SER_PA_PIN);
        } else {
            gpio_clear(SR_PORT, SER_PA_PIN);
        }
        delay(THIRTY_EIGHT_MS);
        gpio_set(SR_PORT, SRCLK_PA_PIN);
        delay(THIRTY_EIGHT_MS);
        gpio_clear(SR_PORT, SRCLK_PA_PIN);
        delay(THIRTY_EIGHT_MS);
        i--;
    }
}

static void show_digit(uint8_t d)
{
    if (d < 10) {
        shift_out(digits[d]);
        pulse();
    } else {
        uint8_t i;
        for (i = 0; i < 3; i++) {
            shift_out(0x7F);
            pulse();
            delay(TEN_MS);
            shift_out(0xFF);
            pulse();
            delay(TEN_MS);
        }
    }
}

int main(void)
{
    RCC_EnableGPIOClock(SR_PORT);

    // small delay after enabling clocks
    delay(TEN_MS);

    gpio_config_t sr_cfg = {.mode = GPIO_MODE_OUTPUT,
                            .otype = GPIO_OTYPE_PP,
                            .speed = GPIO_SPEED_LOW,
                            .pull = GPIO_PULL_NONE,
                            .af = 0};

    gpio_init(SR_PORT, SER_PA_PIN, &sr_cfg);
    gpio_init(SR_PORT, SRCLK_PA_PIN, &sr_cfg);
    gpio_init(SR_PORT, RCLK_PA_PIN, &sr_cfg);

    // set all pins low
    gpio_clear(SR_PORT, SER_PA_PIN);
    gpio_clear(SR_PORT, SRCLK_PA_PIN);
    gpio_clear(SR_PORT, RCLK_PA_PIN);

    show_digit(20);
    uint8_t i = 0;
    while (i < 10) {
        show_digit(i);
        i++;
    };

    // endless loop
    while (1)
        ;

    return 0;
}