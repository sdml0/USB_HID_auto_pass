#ifndef USB_DESCRIPTORS_H
#define USB_DESCRIPTORS_H
#include <stdint.h>

#define USB_BUFSZ             64
#define USB_EP2BUFSZ          8

typedef enum {
    usb_descriptor_type_device          = 0x01,
    usb_descriptor_type_configuration   = 0x02,
    usb_descriptor_type_string          = 0x03,
    usb_descriptor_type_interface       = 0x04,
    usb_descriptor_type_endpoint        = 0x05,
    usb_descriptor_type_qualifier       = 0x06,
    usb_descriptor_type_other           = 0x07,
    usb_descriptor_type_interface_power = 0x08,
    usb_descriptor_type_otg             = 0x09,
    usb_descriptor_type_debug           = 0x0a,
    usb_descriptor_type_interface_assoc = 0x0b,
	 usb_descriptor_type_hid				 = 0x21,
	 usb_descriptor_type_hidreport		 = 0x22,
    usb_descriptor_type_cs_interface    = 0x24,
    usb_descriptor_type_cs_endpoint     = 0x25,
} __attribute__ ((packed)) usb_descriptor_type_t;

typedef struct {
    uint8_t     bLength;
    uint8_t     bDescriptorType;
    uint16_t    bcdUSB;
    uint8_t     bDeviceClass;
    uint8_t     bDeviceSubClass;
    uint8_t     bDeviceProtocol;
    uint8_t     bMaxPacketSize;
    uint16_t    idVendor;
    uint16_t    idProduct;
    uint16_t    bcdDevice;
    uint8_t     iManufacturer;
    uint8_t     iProduct;
    uint8_t     iSerialNumber;
    uint8_t     bNumConfigurations;
} __attribute__ ((packed)) usb_device_descriptor_t;

typedef struct  {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t wTotalLength;
    uint8_t  bNumInterfaces;
    uint8_t  bConfigurationValue;
    uint8_t  iConfiguration;
    uint8_t  bmAttributes;
    uint8_t  bMaxPower;
} __attribute__((packed)) usb_configuration_descriptor_t;

typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;
} __attribute__((packed)) usb_interface_descriptor_t;

typedef struct  {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bEndpointAddress;
    uint8_t  bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t  bInterval;
} __attribute__((packed)) usb_endpoint_descriptor_t;

typedef struct  {
    uint8_t  	bLength;
    uint8_t  	bDescriptorType;
    uint16_t	bcdHID;
    uint8_t		bCountryCode;
    uint8_t 	bNumDescriptors;
    uint8_t		bReportType;
	 uint16_t	wDescriptorLength;
} __attribute__((packed)) usb_hid_descriptor_t;

/* Configuration Descriptor */
typedef struct {
    usb_configuration_descriptor_t      config;
    usb_interface_descriptor_t          interface;
    usb_hid_descriptor_t		          hid;
    usb_endpoint_descriptor_t           endpoint;
} __attribute__((packed)) usb_device_configuration_descriptor_hid_t;

typedef struct {
	usb_configuration_descriptor_t      config;			// acm must be first and without it not works((
	usb_interface_descriptor_t          acm;
	usb_endpoint_descriptor_t           acm_ep;
	usb_interface_descriptor_t          data;
	usb_endpoint_descriptor_t           data_eprx;
	usb_endpoint_descriptor_t           data_eptx;

} __attribute__((packed)) usb_device_configuration_descriptor_cdc_t;

/* USB String Descriptor */
typedef struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t wString[];
} __attribute__((packed, aligned(2))) usb_string_descriptor_t;

typedef struct {
	uint32_t    type;
	uint8_t     rx_size;
	uint8_t     tx_size;
} usb_endpoint_t;

typedef enum {
	usb_device_request_get_status					= 0x00,
	usb_device_request_clear_feature				= 0x01,
	usb_device_request_set_feature				= 0x03,
	usb_device_request_set_address				= 0x05,
	usb_device_request_get_descriptor			= 0x06,
	usb_device_request_set_descriptor   	   = 0x07,
	usb_device_request_get_configuration		= 0x08,
	usb_device_request_set_configuration   	= 0x09,
	usb_cdc_request_set_line_coding				= 0x20,
	usb_cdc_request_get_line_coding				= 0x21,
	usb_cdc_request_set_control_line_state		= 0x22
} usb_device_request_t;

#ifndef USB_DESCRIPTORS_C_ITSELF
extern const usb_device_descriptor_t usb_device_descriptor_hid;
extern const usb_device_descriptor_t usb_device_descriptor_cdc;
extern const usb_device_configuration_descriptor_hid_t usb_configuration_descriptor_hid;
extern const usb_device_configuration_descriptor_cdc_t usb_configuration_descriptor_cdc;
extern const usb_string_descriptor_t* usb_string_descriptors[];
extern const uint8_t usb_hid_descriptor[45];
#endif
void fill_uid_string_descriptor();

#endif /* USB_DESCRIPTORS_H */
