#ifndef ROAMER_BOARD_H
#define ROAMER_BOARD_H

#include <stdbool.h>
#include <stdint.h>

bool board_init(void);
void board_led_set(bool on);
uint32_t board_millis(void);

#endif
