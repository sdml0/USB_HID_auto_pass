#ifndef USB_CDC_H
#define USB_CDC_H

#include <stdint.h>

/* USB CDC Class Codes */

typedef enum {
    usb_class_cdc           = 0x02,
    usb_class_cdc_data      = 0x0a,
} __attribute__ ((packed)) usb_class_cdc_t;

typedef enum {
    usb_subclass_cdc_none   = 0x00,
    usb_subclass_cdc_acm    = 0x02,
} __attribute__ ((packed)) usb_subclass_cdc_t;

typedef enum {
    usb_protocol_cdc_none   = 0x00,
    usb_protocol_cdc_v25ter = 0x01,
} __attribute__ ((packed)) usb_protocol_cdc_t;

#define USB_PROTOCOL_CDC_DEFAULT usb_protocol_cdc_none

typedef enum {
    usb_descriptor_subtype_cdc_header          = 0x00,
    usb_descriptor_subtype_cdc_call_management = 0x01,
    usb_descriptor_subtype_cdc_acm             = 0x02,
    usb_descriptor_subtype_cdc_union           = 0x06,
    usb_descriptor_subtype_cdc_country         = 0x07,
} __attribute__ ((packed)) usb_descriptor_subtype_cdc_t;

#define USB_CDC_ACM_CAPABILITY_COMM_FEATURE         0x01
#define USB_CDC_ACM_CAPABILITY_LINE_CODING          0x02
#define USB_CDC_ACM_CAPABILITY_SEND_BREAK           0x04
#define USB_CDC_ACM_CAPABILITY_NETWORK_CONNECTION   0x08

#define USB_CDC_ACM_CAPABILITIES (USB_CDC_ACM_CAPABILITY_COMM_FEATURE)

/* USB CDC Header Functional Descriptor */

typedef struct  {
    uint8_t     bFunctionLength;
    uint8_t     bDescriptorType;
    uint8_t     bDescriptorSubType;
    uint16_t    bcdCDC;
} __attribute__ ((packed)) usb_cdc_header_desc_t;

/* USB CDC Union Functional Descriptor */

typedef struct {
    uint8_t     bFunctionLength;
    uint8_t     bDescriptorType;
    uint8_t     bDescriptorSubType;
    uint8_t     bMasterInterface0;
    uint8_t     bSlaveInterface0;
} __attribute__ ((packed)) usb_cdc_union_desc_t;

typedef struct {
    uint8_t     bFunctionLength;
    uint8_t     bDescriptorType;
    uint8_t     bDescriptorSubType;
    uint8_t     bmCapabilities;
    uint8_t     bDataInterface;
} __attribute__ ((packed)) usb_cdc_call_mgmt_desc_t;

/* USB CDC Abstract Control Management Functional Descriptor */

typedef struct {
    uint8_t     bFunctionLength;
    uint8_t     bDescriptorType;
    uint8_t     bDescriptorSubType;
    uint8_t     bmCapabilities;
} __attribute__ ((packed)) usb_cdc_acm_desc_t;

/* USB CDC Notifications */

#define USB_CDC_NOTIFICATION_REQUEST_TYPE   0xa1

typedef enum {
    usb_cdc_notification_serial_state   = 0x20,
} __attribute__ ((packed)) usb_cdc_notification_type_t;

typedef struct {
    uint8_t     bmRequestType;
    uint8_t     bNotificationType;
    uint16_t    wValue;
    uint16_t    wIndex;
    uint16_t    wLength;
    uint8_t     data[0];
} __attribute__ ((packed)) usb_cdc_notification_t;

/* Serial State Notification Payload */
typedef uint16_t usb_cdc_serial_state_t;

#define USB_CDC_SERIAL_STATE_DCD            0x01
#define USB_CDC_SERIAL_STATE_DSR            0x02
#define USB_CDC_SERIAL_STATE_RI             0x08
#define USB_CDC_SERIAL_STATE_PARITY_ERROR   0x20
#define USB_CDC_SERIAL_STATE_OVERRUN        0x40

/* USB CDC Line Coding */

typedef enum {
    usb_cdc_char_format_1_stop_bit      = 0x00,
    usb_cdc_char_format_1p5_stop_bits   = 0x01,
    usb_cdc_char_format_2_stop_bits     = 0x02,
    usb_cdc_char_format_last
} __attribute__ ((packed)) usb_cdc_char_format_t;

typedef enum {
    usb_cdc_parity_type_none    = 0x00,
    usb_cdc_parity_type_odd     = 0x01,
    usb_cdc_parity_type_even    = 0x02,
    usb_cdc_parity_type_mark    = 0x03,
    usb_cdc_parity_type_space   = 0x04,
} __attribute__ ((packed)) usb_cdc_parity_type_t;

typedef enum {
    usb_cdc_data_bits_5     = 0x05,
    usb_cdc_data_bits_6     = 0x06,
    usb_cdc_data_bits_7     = 0x07,
    usb_cdc_data_bits_8     = 0x08,
    usb_cdc_data_bits_16    = 0x10,
} __attribute__ ((packed)) usb_cdc_data_bits_t;

typedef struct {
    uint32_t                dwDTERate;
    usb_cdc_char_format_t   bCharFormat;
    usb_cdc_parity_type_t   bParityType;
    usb_cdc_data_bits_t     bDataBits;
} __attribute__ ((packed)) usb_cdc_line_coding_t;

/* USB CDC Control Line State */

#define USB_CDC_CONTROL_LINE_STATE_DTR_MASK  0x01
#define USB_CDC_CONTROL_LINE_STATE_RTS_MASK  0x02

/* CDC Device Definitions */
#define USB_CDC_NUM_PORTS                       3
#define USB_CDC_BUF_SIZE                        0x400
#define USB_CDC_CRTL_LINES_POLLING_INTERVAL     20 /* ms */
#define USB_CDC_CONFIG_PORT                     0


#endif /* USB_CDC_H */
