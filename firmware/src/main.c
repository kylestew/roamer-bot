#include "board.h"
#include "usb_cdc.h"

static const uint8_t usb_heartbeat[] = "roamer heartbeat\r\n";

int main(void) {
    const bool external_clock_ready  = board_init();
    const uint32_t blink_interval_ms = external_clock_ready ? 500U : 100U;
    uint32_t last_toggle_ms          = board_millis();
    uint32_t last_usb_heartbeat_ms   = board_millis();
    bool led_on                      = false;

    if (external_clock_ready) {
        usb_cdc_init();
    }

    for (;;) {
        const uint32_t now_ms = board_millis();

        if ((uint32_t) (now_ms - last_toggle_ms) >= blink_interval_ms) {
            last_toggle_ms += blink_interval_ms;
            led_on = !led_on;
            board_led_set(led_on);
        }

        if (external_clock_ready &&
            (uint32_t) (now_ms - last_usb_heartbeat_ms) >= 1000U) {
            last_usb_heartbeat_ms = now_ms;
            if (usb_cdc_connected()) {
                (void) usb_cdc_try_write(usb_heartbeat,
                                          (uint16_t) (sizeof(usb_heartbeat) - 1U));
            }
        }

        __asm volatile("wfi");
    }
}
