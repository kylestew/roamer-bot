#include "board.h"

int main(void)
{
    const bool external_clock_ready = board_init();
    const uint32_t blink_interval_ms = external_clock_ready ? 500U : 100U;
    uint32_t last_toggle_ms = board_millis();
    bool led_on = false;

    for (;;) {
        const uint32_t now_ms = board_millis();

        if ((uint32_t)(now_ms - last_toggle_ms) >= blink_interval_ms) {
            last_toggle_ms += blink_interval_ms;
            led_on = !led_on;
            board_led_set(led_on);
        }

        __asm volatile ("wfi");
    }
}
