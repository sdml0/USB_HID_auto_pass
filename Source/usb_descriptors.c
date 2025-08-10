#include "usb.h"
#define USB_DESCRIPTORS_C_ITSELF
#include "usb_descriptors.h"

#define UID_BASE								0x1FFFF7E8UL
#define USB_ID_VENDOR   					0x1209		// VID
#define USB_ID_PRODUCT  					0xFFFE		// PID
#define USB_BCD_VERSION(maj, min, rev)  (((maj & 0xFF) << 8) | ((min & 0x0F) << 4) | (rev & 0x0F))
#define USB_STRING_DESC(s)  {.bLength = sizeof(L##s), .bDescriptorType = usb_descriptor_type_string, .wString = {L##s}}

typedef enum {
    usb_endpoint_direction_out  = 0x00,
    usb_endpoint_direction_in   = 0x80
} usb_endpoint_direction_t;

typedef enum {
    usb_device_class_device                 = 0x00,
    usb_device_class_audio                  = 0x01,
    usb_device_class_comm                   = 0x02,
    usb_device_class_hid                    = 0x03,
    usb_device_class_physical               = 0x05,
    usb_device_class_image                  = 0x06,
    usb_device_class_printer                = 0x07,
    usb_device_class_mass_storage           = 0x08,
    usb_device_class_hub                    = 0x09,
    usb_device_class_cdc_data               = 0x0a,
    usb_device_class_smart_card             = 0x0b,
    usb_device_class_content_security       = 0x0d,
    usb_device_class_video                  = 0x0e,
    usb_device_class_personal_healthcare    = 0x0f,
    usb_device_class_audio_video            = 0x10,
    usb_device_class_billboard              = 0x11,
    usb_device_class_usbc_bridge            = 0x12,
    usb_device_class_diagnostic             = 0xdc,
    usb_device_class_wireless_controller    = 0xe0,
    usb_device_class_misc                   = 0xef,
    usb_device_class_application_specific   = 0xfe,
    usb_device_class_vendor_specific        = 0xff
} usb_device_class_t;

const usb_string_descriptor_t str1		= USB_STRING_DESC("NO NAME");					    	// iManufacturer
const usb_string_descriptor_t str2		= USB_STRING_DESC("Generic Keyboard");  		    	// iProduct
		usb_string_descriptor_t str3		= USB_STRING_DESC("XXXXXXXXXXXXXXXXXXXXXXXX");  	// Placeholder, replaced by STM32 UID, 48 bytes
const usb_string_descriptor_t str4		= USB_STRING_DESC("ACM");								// iInterface (ACM)
const usb_string_descriptor_t* usb_string_descriptors[] = { &str1, &str2, &str3, &str4 };

const usb_device_descriptor_t usb_device_descriptor_hid = {
    .bLength            = sizeof(usb_device_descriptor_hid),
    .bDescriptorType    = usb_descriptor_type_device,
    .bcdUSB             = USB_BCD_VERSION(2, 0, 0),
    .bDeviceClass       = 0,	// 0 for HID
    .bDeviceSubClass    = 0,	// 0 for HID
    .bDeviceProtocol    = 0,
    .bMaxPacketSize     = 64,
    .idVendor           = USB_ID_VENDOR,
    .idProduct          = USB_ID_PRODUCT,
    .bcdDevice          = USB_BCD_VERSION(0, 0, 0),
    .iManufacturer      = 1,									// str1
    .iProduct           = 2,									// str2
    .iSerialNumber      = 3,									// str3
    .bNumConfigurations = 1
};

const usb_device_descriptor_t usb_device_descriptor_cdc = {
    .bLength            = sizeof(usb_device_descriptor_cdc),
    .bDescriptorType    = usb_descriptor_type_device,
    .bcdUSB             = USB_BCD_VERSION(2, 0, 0),
    .bDeviceClass       = 2,        // usb_device_class_comm
    .bDeviceSubClass    = 2,        // usb_device_class_comm
    .bDeviceProtocol    = 0,
    .bMaxPacketSize     = 64,
    .idVendor           = USB_ID_VENDOR,
    .idProduct          = USB_ID_PRODUCT,
    .bcdDevice          = USB_BCD_VERSION(0, 0, 0),
    .iManufacturer      = 1,									// str1
    .iProduct           = 2,									// str2
    .iSerialNumber      = 3,									// str3
    .bNumConfigurations = 1
};

const uint8_t usb_hid_descriptor[45] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,                    //   USAGE_MINIMUM (from Keyboard LeftControl)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (to Keyboard Right GUI)		8bits of modifiers
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)				end of modifiers
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)				end of reserved byte by spec
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)				keys
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)			end of keys
    0xc0                           // END_COLLECTION				3 bytes
};

const usb_device_configuration_descriptor_hid_t usb_configuration_descriptor_hid = {
    .config = {
        .bLength                = sizeof(usb_configuration_descriptor_hid.config),
        .bDescriptorType        = usb_descriptor_type_configuration,
        .wTotalLength           = sizeof(usb_configuration_descriptor_hid),				// 48bytes
        .bNumInterfaces         = 1,
        .bConfigurationValue    = 1,
        .iConfiguration         = 0,			// no string
        .bmAttributes           = 0x80,		// must be 0x80
        .bMaxPower              = 200			// 510mA :)
    },
    .interface = {
        .bLength                = sizeof(usb_configuration_descriptor_hid.interface),
        .bDescriptorType        = usb_descriptor_type_interface,
        .bInterfaceNumber       = 0,
        .bAlternateSetting      = 0,
        .bNumEndpoints          = 1,
        .bInterfaceClass        = 3,			// HID
        .bInterfaceSubClass     = 0,			// 1 for boot
        .bInterfaceProtocol     = 1,			// keyboard
        .iInterface             = 0				// no string
    },      // common
    .hid = {
        .bLength						= sizeof(usb_configuration_descriptor_hid.hid),
        .bDescriptorType			= usb_descriptor_type_hid,
        .bcdHID						= 0x0110,
        .bCountryCode				= 0,
        .bNumDescriptors         = 1,
        .bReportType					= usb_descriptor_type_hidreport,
		  .wDescriptorLength			= sizeof(usb_hid_descriptor)
		  
    },
    .endpoint = {
        .bLength                = sizeof(usb_configuration_descriptor_hid.endpoint),
        .bDescriptorType        = usb_descriptor_type_endpoint,
        .bEndpointAddress       = usb_endpoint_direction_in | 2,	// direction and number
        .bmAttributes           = 3,										// 2-bulk, 3-interrupt
        .wMaxPacketSize         = USB_EP2BUFSZ,
        .bInterval              = 20										// 20ms
    }
};

const usb_device_configuration_descriptor_cdc_t usb_configuration_descriptor_cdc = {
    .config = {
        .bLength                = sizeof(usb_configuration_descriptor_cdc.config),
        .bDescriptorType        = usb_descriptor_type_configuration,
        .wTotalLength           = sizeof(usb_configuration_descriptor_cdc),				// 48bytes
        .bNumInterfaces         = 2,
        .bConfigurationValue    = 1,
        .iConfiguration         = 0,
        .bmAttributes           = 0x80,									// some kind default value is 0x80
        .bMaxPower              = 200										// 510mA :)
    },
    .data = {
        .bLength                = sizeof(usb_configuration_descriptor_cdc.data),
        .bDescriptorType        = usb_descriptor_type_interface,
        .bInterfaceNumber       = 0,
        .bAlternateSetting      = 0,
        .bNumEndpoints          = 2,
        .bInterfaceClass        = 0x0a,		   // usb_class_cdc_data
        .bInterfaceSubClass     = 0,
        .bInterfaceProtocol     = 0,
        .iInterface             = 0
    },
    .data_eprx = {
        .bLength                = sizeof(usb_configuration_descriptor_cdc.data_eprx),
        .bDescriptorType        = usb_descriptor_type_endpoint,
        .bEndpointAddress       = usb_endpoint_direction_out | 1,
        .bmAttributes           = 2,										// bulk
        .wMaxPacketSize         = USB_BUFSZ,
        .bInterval              = 0
    },
    .data_eptx = {
        .bLength                = sizeof(usb_configuration_descriptor_cdc.data_eptx),
        .bDescriptorType        = usb_descriptor_type_endpoint,
        .bEndpointAddress       = usb_endpoint_direction_in | 1,
        .bmAttributes           = 2,										// bulk
        .wMaxPacketSize         = USB_BUFSZ,
        .bInterval              = 0
    },
	     .acm = {
        .bLength                = sizeof(usb_configuration_descriptor_cdc.acm),
        .bDescriptorType        = usb_descriptor_type_interface,
        .bInterfaceNumber       = 1,
        .bAlternateSetting      = 0,
        .bNumEndpoints          = 1,
        .bInterfaceClass        = 2,        //usb_class_cdc
        .bInterfaceSubClass     = 2,        //usb_class_cdc
        .bInterfaceProtocol     = 0,
        .iInterface             = 4											// str4
    },
    .acm_ep = {
        .bLength                = sizeof(usb_configuration_descriptor_cdc.acm_ep),
        .bDescriptorType        = usb_descriptor_type_endpoint,
        .bEndpointAddress       = usb_endpoint_direction_in | 2,
        .bmAttributes           = 3,										// interrupt
        .wMaxPacketSize         = USB_EP2BUFSZ,
        .bInterval              = 255
    }
};

void fill_uid_string_descriptor() {
    const char hex_digits[] = "0123456789ABCDEF";
    uint32_t* str_p = (uint32_t*)str3.wString;
    for (uint16_t i = 0; i < 12; i++) str_p[i] = hex_digits[((uint8_t*)UID_BASE)[i] >> 4] | (hex_digits[((uint8_t*)UID_BASE)[i] & 0xF]<<16);
}
