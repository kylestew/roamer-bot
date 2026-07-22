#include "usb_cdc.h"

#include "board.h"
#include "stm32f1xx.h"

#include <stddef.h>

enum {
    USB_ENDPOINT_COUNT = 4U,
    USB_EP0 = 0U,
    USB_CDC_NOTIFICATION_EP = 1U,
    USB_CDC_OUT_EP = 2U,
    USB_CDC_IN_EP = 3U,
    USB_EP0_MAX_PACKET_SIZE = 64U,
    USB_CDC_MAX_PACKET_SIZE = 64U,
    USB_CDC_NOTIFICATION_PACKET_SIZE = 8U,
    USB_PMA_SIZE = 512U,
    USB_PMA_EP0_RX = 0x040U,
    USB_PMA_EP0_TX = 0x080U,
    USB_PMA_NOTIFICATION_TX = 0x0C0U,
    USB_PMA_CDC_OUT_RX = 0x0D0U,
    USB_PMA_CDC_IN_TX = 0x110U,
    USB_PMA_END = USB_PMA_CDC_IN_TX + USB_CDC_MAX_PACKET_SIZE,
    USB_DISCONNECT_PULSE_MS = 10U,
};

enum {
    USB_REQUEST_DIRECTION_IN = 0x80U,
    USB_REQUEST_TYPE_MASK = 0x60U,
    USB_REQUEST_TYPE_STANDARD = 0x00U,
    USB_REQUEST_TYPE_CLASS = 0x20U,
    USB_REQUEST_RECIPIENT_MASK = 0x1FU,
    USB_REQUEST_RECIPIENT_DEVICE = 0x00U,
    USB_REQUEST_RECIPIENT_INTERFACE = 0x01U,
    USB_REQUEST_RECIPIENT_ENDPOINT = 0x02U,
    USB_REQUEST_GET_STATUS = 0x00U,
    USB_REQUEST_CLEAR_FEATURE = 0x01U,
    USB_REQUEST_SET_FEATURE = 0x03U,
    USB_REQUEST_SET_ADDRESS = 0x05U,
    USB_REQUEST_GET_DESCRIPTOR = 0x06U,
    USB_REQUEST_GET_CONFIGURATION = 0x08U,
    USB_REQUEST_SET_CONFIGURATION = 0x09U,
    USB_REQUEST_GET_INTERFACE = 0x0AU,
    USB_REQUEST_SET_INTERFACE = 0x0BU,
    USB_FEATURE_ENDPOINT_HALT = 0x00U,
    USB_DESCRIPTOR_DEVICE = 0x01U,
    USB_DESCRIPTOR_CONFIGURATION = 0x02U,
    USB_DESCRIPTOR_STRING = 0x03U,
    USB_CDC_SET_LINE_CODING = 0x20U,
    USB_CDC_GET_LINE_CODING = 0x21U,
    USB_CDC_SET_CONTROL_LINE_STATE = 0x22U,
    USB_CDC_SEND_BREAK = 0x23U,
};

typedef struct {
    uint8_t request_type;
    uint8_t request;
    uint16_t value;
    uint16_t index;
    uint16_t length;
} usb_setup_request_t;

typedef enum {
    EP0_IDLE,
    EP0_DATA_IN,
    EP0_DATA_OUT,
    EP0_STATUS_IN,
    EP0_STATUS_OUT,
    EP0_STALLED,
} ep0_state_t;

typedef enum {
    EP0_OUT_NONE,
    EP0_OUT_SET_LINE_CODING,
} ep0_out_action_t;

static const uint8_t device_descriptor[] = {
    18U, 0x01U,             /* Device descriptor, USB 2.0. */
    0x00U, 0x02U,
    0x02U, 0x00U, 0x00U,   /* CDC device class. */
    USB_EP0_MAX_PACKET_SIZE,
    0xFEU, 0xCAU,           /* Development-only VID 0xCAFE. */
    0x01U, 0x40U,           /* Development-only PID 0x4001. */
    0x00U, 0x01U,           /* Device release 1.00. */
    0x01U, 0x02U, 0x03U,   /* Manufacturer, product, serial. */
    0x01U,                  /* One configuration. */
};

static const uint8_t configuration_descriptor[] = {
    /* Configuration: two interfaces, self-powered, no bus current. */
    0x09U, 0x02U, 0x43U, 0x00U, 0x02U, 0x01U, 0x00U, 0xC0U, 0x00U,

    /* CDC communication interface. */
    0x09U, 0x04U, 0x00U, 0x00U, 0x01U, 0x02U, 0x02U, 0x01U, 0x00U,
    0x05U, 0x24U, 0x00U, 0x10U, 0x01U, /* CDC header, version 1.10. */
    0x05U, 0x24U, 0x01U, 0x00U, 0x01U, /* Call management. */
    0x04U, 0x24U, 0x02U, 0x02U,       /* Abstract control management. */
    0x05U, 0x24U, 0x06U, 0x00U, 0x01U, /* Union: interfaces 0 and 1. */
    0x07U, 0x05U, 0x81U, 0x03U, 0x08U, 0x00U, 0x10U,

    /* CDC data interface. */
    0x09U, 0x04U, 0x01U, 0x00U, 0x02U, 0x0AU, 0x00U, 0x00U, 0x00U,
    0x07U, 0x05U, 0x02U, 0x02U, 0x40U, 0x00U, 0x00U,
    0x07U, 0x05U, 0x83U, 0x02U, 0x40U, 0x00U, 0x00U,
};

static const uint8_t language_descriptor[] = {
    0x04U, 0x03U, 0x09U, 0x04U, /* English (United States). */
};

static const char manufacturer_string[] = "Roamer";
static const char product_string[] = "Roamer Rev A";

static volatile bool usb_configured;
static volatile bool usb_dtr_asserted;
static volatile bool usb_suspended;
static volatile bool cdc_in_busy;
static uint8_t configuration_value;

static ep0_state_t ep0_state;
static ep0_out_action_t ep0_out_action;
static const uint8_t *ep0_tx_data;
static uint16_t ep0_tx_remaining;
static bool ep0_tx_needs_zlp;
static uint8_t ep0_out_expected;
static uint8_t ep0_out_data[USB_EP0_MAX_PACKET_SIZE];
static uint8_t ep0_reply[USB_EP0_MAX_PACKET_SIZE];
static uint8_t string_descriptor[USB_EP0_MAX_PACKET_SIZE];
static uint8_t line_coding[7] = {
    0x00U, 0xC2U, 0x01U, 0x00U, /* 115200 bits/s. */
    0x00U,                       /* One stop bit. */
    0x00U,                       /* No parity. */
    0x08U,                       /* Eight data bits. */
};
static bool pending_address_valid;
static uint8_t pending_address;

_Static_assert(sizeof(device_descriptor) == 18U,
               "USB device descriptor has an invalid length");
_Static_assert(sizeof(configuration_descriptor) == 67U,
               "USB CDC configuration descriptor has an invalid length");
_Static_assert(USB_PMA_END <= USB_PMA_SIZE,
               "USB endpoint buffers exceed STM32F103 packet memory");

static volatile uint16_t *endpoint_register(uint8_t endpoint)
{
    return &USB->EP0R + ((uint32_t)endpoint * 2U);
}

static uint16_t endpoint_read(uint8_t endpoint)
{
    return *endpoint_register(endpoint);
}

static void endpoint_write(uint8_t endpoint, uint16_t value)
{
    *endpoint_register(endpoint) = value;
}

static void endpoint_set_type(uint8_t endpoint, uint16_t type)
{
    uint16_t value = endpoint_read(endpoint) & (uint16_t)USB_EP_T_MASK;
    value |= type;
    endpoint_write(endpoint, value | USB_EP_CTR_RX | USB_EP_CTR_TX);
}

static void endpoint_set_address(uint8_t endpoint, uint8_t address)
{
    uint16_t value = endpoint_read(endpoint) & (uint16_t)USB_EPREG_MASK;
    value |= address;
    endpoint_write(endpoint, value | USB_EP_CTR_RX | USB_EP_CTR_TX);
}

static void endpoint_set_tx_status(uint8_t endpoint, uint16_t status)
{
    uint16_t value = endpoint_read(endpoint) & (uint16_t)USB_EPTX_DTOGMASK;
    value ^= status;
    endpoint_write(endpoint, value | USB_EP_CTR_RX | USB_EP_CTR_TX);
}

static void endpoint_set_rx_status(uint8_t endpoint, uint16_t status)
{
    uint16_t value = endpoint_read(endpoint) & (uint16_t)USB_EPRX_DTOGMASK;
    value ^= status;
    endpoint_write(endpoint, value | USB_EP_CTR_RX | USB_EP_CTR_TX);
}

static void endpoint_clear_rx_interrupt(uint8_t endpoint)
{
    uint16_t value = endpoint_read(endpoint) &
                     (uint16_t)(0x7FFFU & USB_EPREG_MASK);
    endpoint_write(endpoint, value | USB_EP_CTR_TX);
}

static void endpoint_clear_tx_interrupt(uint8_t endpoint)
{
    uint16_t value = endpoint_read(endpoint) &
                     (uint16_t)(0xFF7FU & USB_EPREG_MASK);
    endpoint_write(endpoint, value | USB_EP_CTR_RX);
}

static void endpoint_clear_tx_toggle(uint8_t endpoint)
{
    const uint16_t current = endpoint_read(endpoint);
    if ((current & USB_EP_DTOG_TX) != 0U) {
        const uint16_t value = current & (uint16_t)USB_EPREG_MASK;
        endpoint_write(endpoint, value | USB_EP_CTR_RX | USB_EP_CTR_TX |
                       USB_EP_DTOG_TX);
    }
}

static void endpoint_clear_rx_toggle(uint8_t endpoint)
{
    const uint16_t current = endpoint_read(endpoint);
    if ((current & USB_EP_DTOG_RX) != 0U) {
        const uint16_t value = current & (uint16_t)USB_EPREG_MASK;
        endpoint_write(endpoint, value | USB_EP_CTR_RX | USB_EP_CTR_TX |
                       USB_EP_DTOG_RX);
    }
}

static volatile uint16_t *pma_register(uint16_t logical_address)
{
    return (volatile uint16_t *)(USB_PMAADDR +
                                 ((uintptr_t)logical_address * 2U));
}

static void pma_write(uint16_t logical_address, const uint8_t *data,
                      uint16_t length)
{
    volatile uint16_t *destination = pma_register(logical_address);

    for (uint16_t offset = 0U; offset < length; offset += 2U) {
        uint16_t value = data[offset];
        if ((uint16_t)(offset + 1U) < length) {
            value |= (uint16_t)data[offset + 1U] << 8U;
        }
        *destination = value;
        destination += 2;
    }
}

static void pma_read(uint16_t logical_address, uint8_t *data, uint16_t length)
{
    volatile const uint16_t *source = pma_register(logical_address);

    for (uint16_t offset = 0U; offset < length; offset += 2U) {
        const uint16_t value = *source;
        data[offset] = (uint8_t)value;
        if ((uint16_t)(offset + 1U) < length) {
            data[offset + 1U] = (uint8_t)(value >> 8U);
        }
        source += 2;
    }
}

static volatile uint16_t *buffer_table_field(uint8_t endpoint,
                                              uint16_t field_offset)
{
    const uint16_t logical_address =
        (uint16_t)(((uint16_t)endpoint * 8U) + field_offset);
    return pma_register(logical_address);
}

static void endpoint_set_tx_buffer(uint8_t endpoint, uint16_t address)
{
    *buffer_table_field(endpoint, 0U) = address & 0xFFFEU;
}

static void endpoint_set_tx_count(uint8_t endpoint, uint16_t count)
{
    *buffer_table_field(endpoint, 2U) = count;
}

static void endpoint_set_rx_buffer(uint8_t endpoint, uint16_t address)
{
    *buffer_table_field(endpoint, 4U) = address & 0xFFFEU;
}

static void endpoint_set_rx_count(uint8_t endpoint, uint16_t count)
{
    uint16_t encoded_count;

    if (count > 62U) {
        uint16_t blocks = count >> 5U;
        if ((count & 0x1FU) == 0U) {
            --blocks;
        }
        encoded_count = (uint16_t)(0x8000U | (blocks << 10U));
    } else if (count == 0U) {
        encoded_count = 0x8000U;
    } else {
        const uint16_t blocks = (uint16_t)((count + 1U) >> 1U);
        encoded_count = (uint16_t)(blocks << 10U);
    }

    *buffer_table_field(endpoint, 6U) = encoded_count;
}

static uint16_t endpoint_get_rx_count(uint8_t endpoint)
{
    return *buffer_table_field(endpoint, 6U) & 0x03FFU;
}

static void endpoint_configure(uint8_t endpoint, uint16_t type)
{
    endpoint_set_type(endpoint, type);
    endpoint_set_address(endpoint, endpoint);
    endpoint_clear_tx_toggle(endpoint);
    endpoint_clear_rx_toggle(endpoint);
}

static void non_control_endpoints_set_configured(bool configured)
{
    cdc_in_busy = false;
    endpoint_clear_tx_toggle(USB_CDC_NOTIFICATION_EP);
    endpoint_clear_rx_toggle(USB_CDC_OUT_EP);
    endpoint_clear_tx_toggle(USB_CDC_IN_EP);
    endpoint_set_tx_status(USB_CDC_NOTIFICATION_EP, USB_EP_TX_NAK);
    endpoint_set_tx_status(USB_CDC_IN_EP, USB_EP_TX_NAK);
    endpoint_set_rx_count(USB_CDC_OUT_EP, USB_CDC_MAX_PACKET_SIZE);
    endpoint_set_rx_status(USB_CDC_OUT_EP,
                           configured ? USB_EP_RX_VALID : USB_EP_RX_NAK);
}

static void usb_bus_reset(void)
{
    usb_configured = false;
    usb_dtr_asserted = false;
    usb_suspended = false;
    configuration_value = 0U;
    cdc_in_busy = false;
    pending_address_valid = false;
    ep0_state = EP0_IDLE;
    ep0_out_action = EP0_OUT_NONE;

    USB->BTABLE = 0U;
    USB->DADDR = USB_DADDR_EF;

    endpoint_configure(USB_EP0, USB_EP_CONTROL);
    endpoint_set_tx_buffer(USB_EP0, USB_PMA_EP0_TX);
    endpoint_set_tx_count(USB_EP0, 0U);
    endpoint_set_rx_buffer(USB_EP0, USB_PMA_EP0_RX);
    endpoint_set_rx_count(USB_EP0, USB_EP0_MAX_PACKET_SIZE);
    endpoint_set_tx_status(USB_EP0, USB_EP_TX_NAK);
    endpoint_set_rx_status(USB_EP0, USB_EP_RX_VALID);

    endpoint_configure(USB_CDC_NOTIFICATION_EP, USB_EP_INTERRUPT);
    endpoint_set_tx_buffer(USB_CDC_NOTIFICATION_EP,
                           USB_PMA_NOTIFICATION_TX);
    endpoint_set_tx_count(USB_CDC_NOTIFICATION_EP, 0U);
    endpoint_set_tx_status(USB_CDC_NOTIFICATION_EP, USB_EP_TX_NAK);
    endpoint_set_rx_status(USB_CDC_NOTIFICATION_EP, USB_EP_RX_DIS);

    endpoint_configure(USB_CDC_OUT_EP, USB_EP_BULK);
    endpoint_set_rx_buffer(USB_CDC_OUT_EP, USB_PMA_CDC_OUT_RX);
    endpoint_set_rx_count(USB_CDC_OUT_EP, USB_CDC_MAX_PACKET_SIZE);
    endpoint_set_tx_status(USB_CDC_OUT_EP, USB_EP_TX_DIS);
    endpoint_set_rx_status(USB_CDC_OUT_EP, USB_EP_RX_NAK);

    endpoint_configure(USB_CDC_IN_EP, USB_EP_BULK);
    endpoint_set_tx_buffer(USB_CDC_IN_EP, USB_PMA_CDC_IN_TX);
    endpoint_set_tx_count(USB_CDC_IN_EP, 0U);
    endpoint_set_tx_status(USB_CDC_IN_EP, USB_EP_TX_NAK);
    endpoint_set_rx_status(USB_CDC_IN_EP, USB_EP_RX_DIS);
}

static uint16_t minimum_u16(uint16_t left, uint16_t right)
{
    return left < right ? left : right;
}

static uint16_t make_string_descriptor(const char *text)
{
    uint16_t characters = 0U;

    while (text[characters] != '\0' &&
           characters < ((sizeof(string_descriptor) - 2U) / 2U)) {
        string_descriptor[2U + (characters * 2U)] =
            (uint8_t)text[characters];
        string_descriptor[3U + (characters * 2U)] = 0U;
        ++characters;
    }

    const uint16_t length = (uint16_t)(2U + (characters * 2U));
    string_descriptor[0] = (uint8_t)length;
    string_descriptor[1] = USB_DESCRIPTOR_STRING;
    return length;
}

static uint16_t make_serial_descriptor(void)
{
    static const char hexadecimal[] = "0123456789ABCDEF";
    volatile const uint32_t *uid = (volatile const uint32_t *)UID_BASE;
    uint16_t characters = 0U;

    for (uint8_t word = 0U; word < 3U; ++word) {
        const uint32_t value = uid[word];
        for (uint8_t nibble = 0U; nibble < 8U; ++nibble) {
            const uint8_t shift = (uint8_t)(28U - (nibble * 4U));
            const uint8_t digit = (uint8_t)((value >> shift) & 0x0FU);
            string_descriptor[2U + (characters * 2U)] =
                (uint8_t)hexadecimal[digit];
            string_descriptor[3U + (characters * 2U)] = 0U;
            ++characters;
        }
    }

    const uint16_t length = (uint16_t)(2U + (characters * 2U));
    string_descriptor[0] = (uint8_t)length;
    string_descriptor[1] = USB_DESCRIPTOR_STRING;
    return length;
}

static const uint8_t *find_descriptor(uint8_t type, uint8_t index,
                                      uint16_t *length)
{
    switch (type) {
    case USB_DESCRIPTOR_DEVICE:
        if (index == 0U) {
            *length = sizeof(device_descriptor);
            return device_descriptor;
        }
        break;

    case USB_DESCRIPTOR_CONFIGURATION:
        if (index == 0U) {
            *length = sizeof(configuration_descriptor);
            return configuration_descriptor;
        }
        break;

    case USB_DESCRIPTOR_STRING:
        switch (index) {
        case 0U:
            *length = sizeof(language_descriptor);
            return language_descriptor;
        case 1U:
            *length = make_string_descriptor(manufacturer_string);
            return string_descriptor;
        case 2U:
            *length = make_string_descriptor(product_string);
            return string_descriptor;
        case 3U:
            *length = make_serial_descriptor();
            return string_descriptor;
        default:
            break;
        }
        break;

    default:
        break;
    }

    *length = 0U;
    return NULL;
}

static void ep0_send_packet(const uint8_t *data, uint16_t length)
{
    if (length > 0U) {
        pma_write(USB_PMA_EP0_TX, data, length);
    }
    endpoint_set_tx_count(USB_EP0, length);
    endpoint_set_tx_status(USB_EP0, USB_EP_TX_VALID);
}

static void ep0_send_next_data_packet(void)
{
    const uint16_t length =
        minimum_u16(ep0_tx_remaining, USB_EP0_MAX_PACKET_SIZE);
    ep0_send_packet(ep0_tx_data, length);
    ep0_tx_data += length;
    ep0_tx_remaining -= length;
}

static void ep0_start_data_in(const uint8_t *data, uint16_t available,
                              uint16_t requested)
{
    const uint16_t length = minimum_u16(available, requested);

    ep0_state = EP0_DATA_IN;
    ep0_tx_data = data;
    ep0_tx_remaining = length;
    ep0_tx_needs_zlp =
        length < requested && (length % USB_EP0_MAX_PACKET_SIZE) == 0U;
    endpoint_set_rx_count(USB_EP0, USB_EP0_MAX_PACKET_SIZE);
    endpoint_set_rx_status(USB_EP0, USB_EP_RX_VALID);

    if (length == 0U) {
        ep0_tx_needs_zlp = false;
        ep0_send_packet(NULL, 0U);
    } else {
        ep0_send_next_data_packet();
    }
}

static void ep0_start_status_in(void)
{
    ep0_state = EP0_STATUS_IN;
    endpoint_set_rx_count(USB_EP0, USB_EP0_MAX_PACKET_SIZE);
    endpoint_set_rx_status(USB_EP0, USB_EP_RX_VALID);
    ep0_send_packet(NULL, 0U);
}

static void ep0_start_data_out(uint8_t length, ep0_out_action_t action)
{
    ep0_state = EP0_DATA_OUT;
    ep0_out_expected = length;
    ep0_out_action = action;
    endpoint_set_tx_status(USB_EP0, USB_EP_TX_NAK);
    endpoint_set_rx_count(USB_EP0, USB_EP0_MAX_PACKET_SIZE);
    endpoint_set_rx_status(USB_EP0, USB_EP_RX_VALID);
}

static void ep0_stall(void)
{
    ep0_state = EP0_STALLED;
    endpoint_set_tx_status(USB_EP0, USB_EP_TX_STALL);
    endpoint_set_rx_status(USB_EP0, USB_EP_RX_STALL);
}

static bool endpoint_address_valid(uint8_t address)
{
    return address == 0x81U || address == 0x02U || address == 0x83U;
}

static bool endpoint_is_stalled(uint8_t address)
{
    const uint8_t endpoint = address & 0x0FU;
    const uint16_t status = endpoint_read(endpoint);

    if ((address & USB_REQUEST_DIRECTION_IN) != 0U) {
        return (status & USB_EPTX_STAT) == USB_EP_TX_STALL;
    }
    return (status & USB_EPRX_STAT) == USB_EP_RX_STALL;
}

static void endpoint_set_halt(uint8_t address, bool halt)
{
    const uint8_t endpoint = address & 0x0FU;

    if ((address & USB_REQUEST_DIRECTION_IN) != 0U) {
        if (halt) {
            endpoint_set_tx_status(endpoint, USB_EP_TX_STALL);
        } else {
            endpoint_clear_tx_toggle(endpoint);
            endpoint_set_tx_status(endpoint, USB_EP_TX_NAK);
        }
    } else if (halt) {
        endpoint_set_rx_status(endpoint, USB_EP_RX_STALL);
    } else {
        endpoint_clear_rx_toggle(endpoint);
        endpoint_set_rx_count(endpoint, USB_CDC_MAX_PACKET_SIZE);
        endpoint_set_rx_status(endpoint,
                               usb_configured ? USB_EP_RX_VALID : USB_EP_RX_NAK);
    }
}

static bool handle_get_status(const usb_setup_request_t *setup)
{
    const uint8_t recipient =
        setup->request_type & USB_REQUEST_RECIPIENT_MASK;

    if (setup->length != 2U || setup->value != 0U) {
        return false;
    }

    ep0_reply[0] = 0U;
    ep0_reply[1] = 0U;

    if (recipient == USB_REQUEST_RECIPIENT_DEVICE && setup->index == 0U) {
        ep0_reply[0] = 0x01U; /* Self-powered. */
    } else if (recipient == USB_REQUEST_RECIPIENT_INTERFACE &&
               usb_configured && setup->index < 2U) {
        /* Interfaces report no status flags. */
    } else if (recipient == USB_REQUEST_RECIPIENT_ENDPOINT &&
               usb_configured && setup->index <= 0xFFU &&
               endpoint_address_valid((uint8_t)setup->index)) {
        ep0_reply[0] = endpoint_is_stalled((uint8_t)setup->index) ? 1U : 0U;
    } else {
        return false;
    }

    ep0_start_data_in(ep0_reply, 2U, setup->length);
    return true;
}

static bool handle_standard_request(const usb_setup_request_t *setup)
{
    switch (setup->request) {
    case USB_REQUEST_GET_STATUS:
        if ((setup->request_type & USB_REQUEST_DIRECTION_IN) != 0U) {
            return handle_get_status(setup);
        }
        break;

    case USB_REQUEST_GET_DESCRIPTOR:
        if (setup->request_type ==
            (USB_REQUEST_DIRECTION_IN | USB_REQUEST_RECIPIENT_DEVICE)) {
            uint16_t descriptor_length;
            const uint8_t descriptor_type = (uint8_t)(setup->value >> 8U);
            const uint8_t descriptor_index = (uint8_t)setup->value;
            const uint8_t *descriptor =
                find_descriptor(descriptor_type, descriptor_index,
                                &descriptor_length);
            if (descriptor != NULL) {
                ep0_start_data_in(descriptor, descriptor_length, setup->length);
                return true;
            }
        }
        break;

    case USB_REQUEST_SET_ADDRESS:
        if (setup->request_type == USB_REQUEST_RECIPIENT_DEVICE &&
            setup->index == 0U && setup->length == 0U &&
            setup->value <= 127U) {
            pending_address = (uint8_t)setup->value;
            pending_address_valid = true;
            ep0_start_status_in();
            return true;
        }
        break;

    case USB_REQUEST_GET_CONFIGURATION:
        if (setup->request_type ==
                (USB_REQUEST_DIRECTION_IN | USB_REQUEST_RECIPIENT_DEVICE) &&
            setup->value == 0U && setup->index == 0U && setup->length == 1U) {
            ep0_reply[0] = configuration_value;
            ep0_start_data_in(ep0_reply, 1U, setup->length);
            return true;
        }
        break;

    case USB_REQUEST_SET_CONFIGURATION:
        if (setup->request_type == USB_REQUEST_RECIPIENT_DEVICE &&
            setup->index == 0U && setup->length == 0U &&
            setup->value <= 1U) {
            configuration_value = (uint8_t)setup->value;
            usb_configured = configuration_value == 1U;
            usb_dtr_asserted = false;
            non_control_endpoints_set_configured(usb_configured);
            ep0_start_status_in();
            return true;
        }
        break;

    case USB_REQUEST_GET_INTERFACE:
        if (setup->request_type ==
                (USB_REQUEST_DIRECTION_IN |
                 USB_REQUEST_RECIPIENT_INTERFACE) &&
            usb_configured && setup->value == 0U && setup->index < 2U &&
            setup->length == 1U) {
            ep0_reply[0] = 0U;
            ep0_start_data_in(ep0_reply, 1U, setup->length);
            return true;
        }
        break;

    case USB_REQUEST_SET_INTERFACE:
        if (setup->request_type == USB_REQUEST_RECIPIENT_INTERFACE &&
            usb_configured && setup->value == 0U && setup->index < 2U &&
            setup->length == 0U) {
            ep0_start_status_in();
            return true;
        }
        break;

    case USB_REQUEST_CLEAR_FEATURE:
    case USB_REQUEST_SET_FEATURE:
        if (setup->request_type == USB_REQUEST_RECIPIENT_ENDPOINT &&
            usb_configured && setup->value == USB_FEATURE_ENDPOINT_HALT &&
            setup->index <= 0xFFU && setup->length == 0U &&
            endpoint_address_valid((uint8_t)setup->index)) {
            endpoint_set_halt((uint8_t)setup->index,
                              setup->request == USB_REQUEST_SET_FEATURE);
            ep0_start_status_in();
            return true;
        }
        break;

    default:
        break;
    }

    return false;
}

static bool handle_cdc_request(const usb_setup_request_t *setup)
{
    if (!usb_configured || setup->index != 0U ||
        (setup->request_type & USB_REQUEST_RECIPIENT_MASK) !=
            USB_REQUEST_RECIPIENT_INTERFACE) {
        return false;
    }

    switch (setup->request) {
    case USB_CDC_SET_LINE_CODING:
        if (setup->request_type ==
                (USB_REQUEST_TYPE_CLASS |
                 USB_REQUEST_RECIPIENT_INTERFACE) &&
            setup->length == sizeof(line_coding)) {
            ep0_start_data_out(sizeof(line_coding),
                               EP0_OUT_SET_LINE_CODING);
            return true;
        }
        break;

    case USB_CDC_GET_LINE_CODING:
        if (setup->request_type ==
                (USB_REQUEST_DIRECTION_IN | USB_REQUEST_TYPE_CLASS |
                 USB_REQUEST_RECIPIENT_INTERFACE) &&
            setup->length == sizeof(line_coding)) {
            ep0_start_data_in(line_coding, sizeof(line_coding), setup->length);
            return true;
        }
        break;

    case USB_CDC_SET_CONTROL_LINE_STATE:
        if (setup->request_type ==
                (USB_REQUEST_TYPE_CLASS |
                 USB_REQUEST_RECIPIENT_INTERFACE) &&
            setup->length == 0U) {
            usb_dtr_asserted = (setup->value & 0x0001U) != 0U;
            ep0_start_status_in();
            return true;
        }
        break;

    case USB_CDC_SEND_BREAK:
        if (setup->request_type ==
                (USB_REQUEST_TYPE_CLASS |
                 USB_REQUEST_RECIPIENT_INTERFACE) &&
            setup->length == 0U) {
            ep0_start_status_in();
            return true;
        }
        break;

    default:
        break;
    }

    return false;
}

static usb_setup_request_t read_setup_request(void)
{
    uint8_t raw[8];
    pma_read(USB_PMA_EP0_RX, raw, sizeof(raw));

    const usb_setup_request_t setup = {
        .request_type = raw[0],
        .request = raw[1],
        .value = (uint16_t)(raw[2] | ((uint16_t)raw[3] << 8U)),
        .index = (uint16_t)(raw[4] | ((uint16_t)raw[5] << 8U)),
        .length = (uint16_t)(raw[6] | ((uint16_t)raw[7] << 8U)),
    };
    return setup;
}

static void handle_ep0_setup(void)
{
    const usb_setup_request_t setup = read_setup_request();

    pending_address_valid = false;
    ep0_out_action = EP0_OUT_NONE;
    endpoint_set_tx_status(USB_EP0, USB_EP_TX_NAK);
    endpoint_set_rx_count(USB_EP0, USB_EP0_MAX_PACKET_SIZE);
    endpoint_set_rx_status(USB_EP0, USB_EP_RX_VALID);

    const uint8_t type = setup.request_type & USB_REQUEST_TYPE_MASK;
    bool handled = false;

    if (type == USB_REQUEST_TYPE_STANDARD) {
        handled = handle_standard_request(&setup);
    } else if (type == USB_REQUEST_TYPE_CLASS) {
        handled = handle_cdc_request(&setup);
    }

    if (!handled) {
        ep0_stall();
    }
}

static void handle_ep0_out(void)
{
    const uint16_t count = endpoint_get_rx_count(USB_EP0);

    if (ep0_state == EP0_DATA_OUT && count == ep0_out_expected) {
        pma_read(USB_PMA_EP0_RX, ep0_out_data, count);
        if (ep0_out_action == EP0_OUT_SET_LINE_CODING) {
            for (uint8_t index = 0U; index < sizeof(line_coding); ++index) {
                line_coding[index] = ep0_out_data[index];
            }
        }
        ep0_out_action = EP0_OUT_NONE;
        ep0_start_status_in();
        return;
    }

    if (ep0_state == EP0_STATUS_OUT && count == 0U) {
        ep0_state = EP0_IDLE;
        endpoint_set_tx_status(USB_EP0, USB_EP_TX_NAK);
        endpoint_set_rx_count(USB_EP0, USB_EP0_MAX_PACKET_SIZE);
        endpoint_set_rx_status(USB_EP0, USB_EP_RX_VALID);
        return;
    }

    ep0_stall();
}

static void handle_ep0_in(void)
{
    if (ep0_state == EP0_DATA_IN) {
        if (ep0_tx_remaining > 0U) {
            ep0_send_next_data_packet();
        } else if (ep0_tx_needs_zlp) {
            ep0_tx_needs_zlp = false;
            ep0_send_packet(NULL, 0U);
        } else {
            ep0_state = EP0_STATUS_OUT;
            endpoint_set_tx_status(USB_EP0, USB_EP_TX_NAK);
            endpoint_set_rx_count(USB_EP0, USB_EP0_MAX_PACKET_SIZE);
            endpoint_set_rx_status(USB_EP0, USB_EP_RX_VALID);
        }
        return;
    }

    if (ep0_state == EP0_STATUS_IN) {
        endpoint_set_tx_status(USB_EP0, USB_EP_TX_NAK);
        if (pending_address_valid) {
            USB->DADDR = (uint16_t)(USB_DADDR_EF | pending_address);
            pending_address_valid = false;
        }
        ep0_state = EP0_IDLE;
        endpoint_set_rx_count(USB_EP0, USB_EP0_MAX_PACKET_SIZE);
        endpoint_set_rx_status(USB_EP0, USB_EP_RX_VALID);
    }
}

static void handle_correct_transfer(void)
{
    usb_suspended = false;

    while ((USB->ISTR & USB_ISTR_CTR) != 0U) {
        const uint8_t endpoint = (uint8_t)(USB->ISTR & USB_ISTR_EP_ID);
        const uint16_t status = endpoint_read(endpoint);

        if ((status & USB_EP_CTR_RX) != 0U) {
            const bool setup = endpoint == USB_EP0 &&
                               (status & USB_EP_SETUP) != 0U;
            endpoint_clear_rx_interrupt(endpoint);

            if (endpoint == USB_EP0) {
                if (setup) {
                    handle_ep0_setup();
                } else {
                    handle_ep0_out();
                }
            } else if (endpoint == USB_CDC_OUT_EP) {
                endpoint_set_rx_count(USB_CDC_OUT_EP,
                                      USB_CDC_MAX_PACKET_SIZE);
                endpoint_set_rx_status(USB_CDC_OUT_EP,
                                       usb_configured ? USB_EP_RX_VALID
                                                      : USB_EP_RX_NAK);
            }
            continue;
        }

        if ((status & USB_EP_CTR_TX) != 0U) {
            endpoint_clear_tx_interrupt(endpoint);

            if (endpoint == USB_EP0) {
                handle_ep0_in();
            } else if (endpoint == USB_CDC_IN_EP) {
                cdc_in_busy = false;
                endpoint_set_tx_status(USB_CDC_IN_EP, USB_EP_TX_NAK);
            } else if (endpoint == USB_CDC_NOTIFICATION_EP) {
                endpoint_set_tx_status(USB_CDC_NOTIFICATION_EP, USB_EP_TX_NAK);
            }
            continue;
        }

        break;
    }
}

static void clear_usb_interrupt(uint16_t interrupt)
{
    USB->ISTR = (uint16_t)~interrupt;
}

void USB_LP_CAN1_RX0_IRQHandler(void)
{
    for (;;) {
        const uint16_t interrupts = USB->ISTR;

        if ((interrupts & USB_ISTR_RESET) != 0U) {
            clear_usb_interrupt(USB_ISTR_RESET);
            usb_bus_reset();
        } else if ((interrupts & USB_ISTR_CTR) != 0U) {
            handle_correct_transfer();
        } else if ((interrupts & USB_ISTR_PMAOVR) != 0U) {
            clear_usb_interrupt(USB_ISTR_PMAOVR);
        } else if ((interrupts & USB_ISTR_ERR) != 0U) {
            clear_usb_interrupt(USB_ISTR_ERR);
        } else if ((interrupts & USB_ISTR_WKUP) != 0U) {
            usb_suspended = false;
            clear_usb_interrupt(USB_ISTR_WKUP);
        } else if ((interrupts & USB_ISTR_SUSP) != 0U) {
            usb_suspended = true;
            clear_usb_interrupt(USB_ISTR_SUSP);
        } else {
            break;
        }
    }
}

static void usb_disconnect_pulse(void)
{
    const uint32_t data_plus_pin = 1UL << 12U;
    const uint32_t configuration_shift = (12U - 8U) * 4U;

    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    (void)RCC->APB2ENR;

    GPIOA->BSRR = data_plus_pin << 16U;
    uint32_t configuration = GPIOA->CRH;
    configuration &= ~(0xFUL << configuration_shift);
    configuration |= 0x2UL << configuration_shift;
    GPIOA->CRH = configuration;

    const uint32_t started_ms = board_millis();
    while ((uint32_t)(board_millis() - started_ms) < USB_DISCONNECT_PULSE_MS) {
        __NOP();
    }

    configuration = GPIOA->CRH;
    configuration &= ~(0xFUL << configuration_shift);
    configuration |= 0x4UL << configuration_shift;
    GPIOA->CRH = configuration;
}

void usb_cdc_init(void)
{
    usb_disconnect_pulse();

    RCC->APB1ENR |= RCC_APB1ENR_USBEN;
    RCC->APB1RSTR |= RCC_APB1RSTR_USBRST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_USBRST;
    (void)RCC->APB1RSTR;

    USB->CNTR = USB_CNTR_FRES;
    USB->CNTR = 0U;
    USB->ISTR = 0U;
    usb_bus_reset();

    NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 5U);
    NVIC_ClearPendingIRQ(USB_LP_CAN1_RX0_IRQn);
    USB->CNTR = USB_CNTR_CTRM | USB_CNTR_RESETM | USB_CNTR_PMAOVRM |
                USB_CNTR_ERRM | USB_CNTR_WKUPM | USB_CNTR_SUSPM;
    NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
}

bool usb_cdc_connected(void)
{
    return usb_configured && usb_dtr_asserted && !usb_suspended;
}

bool usb_cdc_try_write(const uint8_t *data, uint16_t length)
{
    if ((data == NULL && length != 0U) || length > USB_CDC_MAX_PACKET_SIZE) {
        return false;
    }

    const uint32_t interrupt_mask = __get_PRIMASK();
    __disable_irq();

    bool accepted = false;
    if (usb_configured && usb_dtr_asserted && !usb_suspended && !cdc_in_busy) {
        if (length > 0U) {
            pma_write(USB_PMA_CDC_IN_TX, data, length);
        }
        endpoint_set_tx_count(USB_CDC_IN_EP, length);
        cdc_in_busy = true;
        endpoint_set_tx_status(USB_CDC_IN_EP, USB_EP_TX_VALID);
        accepted = true;
    }

    if (interrupt_mask == 0U) {
        __enable_irq();
    }
    return accepted;
}
