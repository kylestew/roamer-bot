#ifndef ROAMER_USB_CDC_H
#define ROAMER_USB_CDC_H

#include <stdbool.h>
#include <stdint.h>

void usb_cdc_init(void);
bool usb_cdc_connected(void);
bool usb_cdc_try_write(const uint8_t *data, uint16_t length);

#endif
