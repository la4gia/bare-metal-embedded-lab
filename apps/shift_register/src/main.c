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

    // endless loop for counter
    uint8_t value = 0x00;
    while (value <= 0xFF) {

        shift_out(value);
        pulse();
        value++;
    };

    return 0;
}