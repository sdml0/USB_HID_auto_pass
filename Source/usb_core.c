#include "usb.h"
#include "usb_descriptors.h"
#include "stm32f10x.h"
//#include "air32f10x.h"

#define USB_NUM_ENDPOINTS (sizeof(usb_endpoints)/sizeof(*usb_endpoints))

const uint16_t KBRDLookupTable[127] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0x2A00, 0x2B00, 0x2800, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0x2C00, 0x1E02, 0x3402, 0x2002, 0x2102, 0x2202, 0x2402, 0x3400,
	0x2602, 0x2702, 0x2502, 0x2E02, 0x3600, 0x2D00, 0x3700, 0x3800,
	0x2700, 0x1E00, 0x1F00, 0x2000, 0x2100, 0x2200, 0x2300, 0x2400,
	0x2500, 0x2600, 0x3302, 0x3300, 0x3602, 0x2E00, 0x3702, 0x3802,
	0x1F02, 0x0402, 0x0502, 0x0602, 0x0702, 0x0802, 0x0902, 0x0A02,
	0x0B02, 0x0C02, 0x0D02, 0x0E02, 0x0F02, 0x1002, 0x1102, 0x1202,
	0x1302, 0x1402, 0x1502, 0x1602, 0x1702, 0x1802, 0x1902, 0x1A02,
	0x1B02, 0x1C02, 0x1D02, 0x2F00, 0x3100, 0x3000, 0x2302, 0x2D02,
	0x3500, 0x0400, 0x0500, 0x0600, 0x0700, 0x0800, 0x0900, 0x0A00,
	0x0B00, 0x0C00, 0x0D00, 0x0E00, 0x0F00, 0x1000, 0x1100, 0x1200,
	0x1300, 0x1400, 0x1500, 0x1600, 0x1700, 0x1800, 0x1900, 0x1A00,
	0x1B00, 0x1C00, 0x1D00, 0x2F02, 0x3102, 0x3002, 0x3502
};

uint8_t UsbTempBuf[USB_BUFSZ];
uint8_t* ExtChar;
uint8_t CharSent = 0;
const usb_device_descriptor_t* UsbDeviceDescriptor;
const usb_configuration_descriptor_t* UsbConfigurationDescriptor;

typedef enum {
    usb_setup_direction_host_to_device = 0,
    usb_setup_direction_device_to_host = 1
} usb_setup_direction_t;

typedef enum {
    usb_setup_type_standard    = 0,
    usb_setup_type_class       = 1,
    usb_setup_type_vendor      = 2,
    usb_setup_type_reserved    = 3,
} usb_setup_type_t;

typedef enum {
    usb_setup_recepient_device      = 0,
    usb_setup_recepient_interface   = 1,
    usb_setup_recepient_endpoint    = 2,
    usb_setup_recepient_other       = 3,
} usb_setup_recepient_t;

typedef struct {
    union {
        uint8_t bmRequestType;
        struct {
            usb_setup_recepient_t recepient:5;
            usb_setup_type_t type:2;
            usb_setup_direction_t direction:1;
        };
    };
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} __attribute__ ((packed)) usb_setup_t;

static struct {
	// Multipacket transfers and zlp not handled - every descriptor must be less than 64 bytes
	usb_setup_t setup;
	void* payload;
	uint16_t payload_size;
	uint8_t address;
	uint8_t configuration;
	uint8_t await_payload;
} usb_control_struct;

const usb_endpoint_t usb_endpoints[] = {			// for filling in init and reset
	{ .type = USB_EP_CONTROL,		.rx_size    = USB_BUFSZ,		.tx_size    = USB_BUFSZ},		// rx must be 32 divisible
	{ .type = USB_EP_BULK,			.rx_size    = USB_BUFSZ,		.tx_size    = USB_BUFSZ},
	{ .type = USB_EP_INTERRUPT,	.rx_size    = 0, 					.tx_size    = USB_EP2BUFSZ}
};

void usb_io_init(uint8_t* one_char_buf)
{
	uint8_t i;
	uint16_t offset = USB_BTABLE_OFFSET>>1;									// Info for USB itself, so internal addressing, without word alignment (64)
	RCC->APB1ENR |= RCC_APB1ENR_USBEN;
	USB->CNTR = 1;																		// Clear power down, but remain in reset state
	fill_uid_string_descriptor();
	for (i = 0; i < USB_NUM_ENDPOINTS; i++) {
		USB_BTABLE[i].tx_offset = offset;
		offset += usb_endpoints[i].tx_size;
		USB_BTABLE[i].rx_offset = offset;
		USB_BTABLE[i].rx_count = ((usb_endpoints[i].rx_size / USB_BTABLE_LARGE_BLOCK_SIZE) - 1) << USB_RX_NUM_BLOCK_Pos;
		USB_BTABLE[i].rx_count |= USB_COUNT_RX_BLSIZE;
		offset += usb_endpoints[i].rx_size;
	}
	
	USB_BTABLE[2].tx_count = 3;									// constant HID report size
	if (!one_char_buf) {			// HID
		UsbDeviceDescriptor = &usb_device_descriptor_hid;
		UsbConfigurationDescriptor = (const usb_configuration_descriptor_t*)&usb_configuration_descriptor_hid;
		CharSent = 1;
	} else {							// CDC
		UsbDeviceDescriptor = &usb_device_descriptor_cdc;
		UsbConfigurationDescriptor = (const usb_configuration_descriptor_t*)&usb_configuration_descriptor_cdc;
		ExtChar = one_char_buf;
	}
	
	NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
	for (i = 0; i < 72; i++) __NOP();		// 1us delay, why not
	USB->CNTR = 0;																							// And clear reset state. I hope code above executes longer than 1uS.
	USB->ISTR = 0;																							// Clear event register, strangly it's not zero at this moment
	USB->CNTR = USB_CNTR_CTRM | USB_CNTR_RESETM | USB_CNTR_SUSPM | USB_CNTR_WKUPM;// | USB_CNTR_SOFM;
	while(!usb_control_struct.address);
}

void ep0_read(uint16_t* buf)
{
	uint32_t* ep_buf = (uint32_t*)(USB_PMAADDR + (USB_BTABLE[0].rx_offset<<1));		// External addressing, word alignment
	uint16_t count = USB_BTABLE[0].rx_count & USB_COUNT_RX_COUNT;
	
	while(count > 1) { *buf++ = *ep_buf++; count -= 2; }
	if (count & 1) *(uint8_t*)buf = (uint8_t)*ep_buf;
	EP_REG(0) = (EP_REG(0) ^ USB_EP_RX_VALID) & (USB_EP_T_FIELD|USB_EP_KIND|USB_EP_CTR_TX|USB_EPADDR_FIELD|USB_EP_RX_VALID);
	return;
}

void ep0_send(uint16_t* buf, uint16_t count)
{
	uint32_t* ep_buf = (uint32_t*)(USB_PMAADDR + (USB_BTABLE[0].tx_offset<<1));		// External addressing, word alignment

	USB_BTABLE[0].tx_count = count;
	count = (count + 1)>>1;
	while (count--) *ep_buf++ = *buf++;
	EP_REG(0) = (EP_REG(0) ^ USB_EP_TX_VALID) & (USB_EPREG_MASK | USB_EP_TX_VALID);
	return;
}

void send_keypress(uint8_t key)
{
	uint16_t k = KBRDLookupTable[key];
	while(CharSent);
	*(uint32_t*)(USB_PMAADDR + USB_BTABLE_OFFSET + 2*4*USB_BUFSZ) = k&0xFF;
	*(uint32_t*)(USB_PMAADDR + USB_BTABLE_OFFSET + 2*4*USB_BUFSZ + 4) = k>>8;
	EP_REG(2) = (EP_REG(2) ^ USB_EP_TX_VALID) & (USB_EPREG_MASK | USB_EP_TX_VALID);
	CharSent = 2;
	return;
}

void cdc_print_string(char* str, uint8_t len)
{
	uint32_t* ep_buf = (uint32_t*)(USB_PMAADDR + (USB_BTABLE[1].tx_offset<<1));		// External addressing, word alignment

	USB_BTABLE[1].tx_count = len;
	len = (len + 1)>>1;
	while (len--) { *ep_buf++ = *(uint16_t*)str; str+=2; }
	EP_REG(1) = (EP_REG(1) ^ USB_EP_TX_VALID) & (USB_EPREG_MASK | USB_EP_TX_VALID);
	return;		
}

uint8_t usb_control_get_descriptor()
{
	usb_descriptor_type_t descriptor_type = usb_control_struct.setup.wValue >> 8;
	uint8_t descriptor_index = (usb_control_struct.setup.wValue & 0xff) - 1;
	switch (descriptor_type) {
		case usb_descriptor_type_device:
			usb_control_struct.payload = (void*)UsbDeviceDescriptor;
			usb_control_struct.payload_size = UsbDeviceDescriptor->bLength;
			break;
		case usb_descriptor_type_configuration:
			usb_control_struct.payload = (void*)UsbConfigurationDescriptor;
			usb_control_struct.payload_size = UsbConfigurationDescriptor->wTotalLength;
			break;
		case usb_descriptor_type_string:
			usb_control_struct.payload = (void*)usb_string_descriptors[descriptor_index];
			usb_control_struct.payload_size = usb_string_descriptors[descriptor_index]->bLength;
			break;
		case usb_descriptor_type_hidreport:
			usb_control_struct.payload = (void*)usb_hid_descriptor;
			usb_control_struct.payload_size = sizeof(usb_hid_descriptor);
			CharSent = 0;
			break;
		default: return 0;
	}
	return 1;
}

static void usb_control_request()
{
	static uint32_t baud_rate = 9600;
	usb_control_struct.payload = UsbTempBuf;
	switch(usb_control_struct.setup.bRequest) {
		case usb_cdc_request_set_control_line_state: break;
		case usb_cdc_request_set_line_coding:
			baud_rate = *(uint32_t*)usb_control_struct.payload;
			break;
		case usb_cdc_request_get_line_coding:
			if (usb_control_struct.setup.wLength == 7) {
				((uint32_t*)usb_control_struct.payload)[0] = baud_rate;
				((uint32_t*)usb_control_struct.payload)[1] = 0x80000;
				usb_control_struct.payload_size = 7;
				break;
			} else goto stall;
		case usb_device_request_get_configuration:
			*(uint8_t*)(usb_control_struct.payload) = usb_control_struct.configuration;
			usb_control_struct.payload_size = 1;
			break;
		case usb_device_request_get_status:
			*(uint16_t*)(usb_control_struct.payload) = 0;
			usb_control_struct.payload_size = 2;
			break;
		case usb_device_request_set_address:
			usb_control_struct.address = usb_control_struct.setup.wValue & USB_DADDR_ADD;
			break;
		case usb_device_request_get_descriptor:
			if (usb_control_get_descriptor()) break;
			else goto stall;
		case usb_device_request_set_configuration:
			if ((usb_control_struct.setup.wValue & 0xFF) == 1) {
				usb_control_struct.configuration = 1;
				break;
			}
		default:stall: EP_REG(0) = (EP_REG(0) ^ USB_EP_TX_STALL) & (USB_EPREG_MASK | USB_EP_TX_STALL); return;
	}
	if (usb_control_struct.setup.direction == usb_setup_direction_device_to_host) {						// Must have something in payload to reply
		if (usb_control_struct.payload_size > usb_control_struct.setup.wLength) usb_control_struct.payload_size = usb_control_struct.setup.wLength;
		ep0_send(usb_control_struct.payload, usb_control_struct.payload_size);
		EP_REG(0) = (EP_REG(0) & USB_EPREG_MASK) | USB_EP_KIND;													// STATUS_OUT - await handshake packet
	} else {
		USB_BTABLE[0].tx_count = 0;																						// Must reply handshake - empty packet
		EP_REG(0) = (EP_REG(0) ^ USB_EP_TX_VALID) & (USB_EPREG_MASK | USB_EP_TX_VALID);
	}
}

void USB_LP_CAN1_RX0_IRQHandler()	// Make it lower priority ?
{
	uint8_t ep_num;

	if (USB->ISTR & USB_ISTR_CTR) {													// Correct Transfer
		ep_num = USB->ISTR & USB_ISTR_EP_ID;
		if (ep_num) {			// app data
			if (ep_num == 2) {			// HID
				if (CharSent>>1) {
					*(uint32_t*)(USB_PMAADDR + USB_BTABLE_OFFSET + 2*4*USB_BUFSZ) = 0;
					*(uint32_t*)(USB_PMAADDR + USB_BTABLE_OFFSET + 2*4*USB_BUFSZ + 4) = 0;
					EP_REG(2) = (EP_REG(2) ^ USB_EP_TX_VALID) & (USB_EPREG_MASK | USB_EP_TX_VALID);
					CharSent = 1;
				} else CharSent = 0;
				EP_REG(2) &= USB_EP_SETUP | USB_EP_T_FIELD | USB_EP_KIND | USB_EPADDR_FIELD;
			} else if (ep_num == 1) {	// CDC
				if ((EP_REG(1) & USB_EP_CTR_RX) && !*ExtChar) {
					ep_num = *(uint8_t*)(USB_PMAADDR + (USB_BTABLE[1].rx_offset<<1));		// External addresation, word alignment
					EP_REG(1) = (EP_REG(1) ^ USB_EP_RX_VALID) & (USB_EP_T_FIELD|USB_EP_KIND|USB_EP_CTR_TX|USB_EPADDR_FIELD|USB_EP_RX_VALID);
					*ExtChar = ep_num;
					*(uint32_t*)(USB_PMAADDR + (USB_BTABLE[1].tx_offset<<1)) = ep_num;
					USB_BTABLE[1].tx_count = 1;
					if (ep_num == 0x0D) {					// enter
						*(uint32_t*)(USB_PMAADDR + (USB_BTABLE[1].tx_offset<<1)) = 0x0D0A;
						USB_BTABLE[1].tx_count = 2;
					} else if (ep_num == 0x08) {			// backspace
						*(uint32_t*)(USB_PMAADDR + (USB_BTABLE[1].tx_offset<<1)) = 0x2008;
						*(uint32_t*)(USB_PMAADDR + (USB_BTABLE[1].tx_offset<<1) + 4) = 0x08;
						USB_BTABLE[1].tx_count = 3;
					}
					EP_REG(1) = (EP_REG(1) ^ USB_EP_TX_VALID) & (USB_EPREG_MASK | USB_EP_TX_VALID);
				}
				EP_REG(1) &= USB_EP_SETUP | USB_EP_T_FIELD | USB_EP_KIND | USB_EPADDR_FIELD;
			}
		} else {		// control request
			if (EP_REG(0) & USB_EP_CTR_TX) {
				if (usb_control_struct.address) { USB->DADDR = USB_DADDR_EF | usb_control_struct.address; usb_control_struct.address = 0; }
				EP_REG(0) &= USB_EP_SETUP | USB_EP_CTR_RX | USB_EP_T_FIELD | USB_EP_KIND | USB_EPADDR_FIELD;
			}
			if (EP_REG(0) & USB_EP_SETUP) {
				ep0_read((uint16_t*)&usb_control_struct.setup);
				if(usb_control_struct.setup.direction || !usb_control_struct.setup.wLength) usb_control_request();
				else usb_control_struct.await_payload = 1;			// Must be data packet after this
			}
			else if (EP_REG(0) & USB_EP_CTR_RX) {
				if (!usb_control_struct.await_payload) EP_REG(0) = (EP_REG(0) ^ USB_EP_RX_VALID) & (USB_EP_T_FIELD | USB_EP_CTR_TX | USB_EPADDR_FIELD | USB_EP_RX_VALID);	// Clear STATUS_OUT
				else {
					usb_control_struct.await_payload = 0;
					ep0_read((uint16_t*)UsbTempBuf);
					usb_control_request();
				}
			}
		}
	} else if (USB->ISTR & USB_ISTR_RESET) {
		USB->ISTR = (uint16_t)(~USB_ISTR_RESET);
		usb_control_struct.await_payload = 0;
		usb_control_struct.configuration = 0;
		usb_control_struct.address = 0;					// After reset EP_REG contains zeroes
		for (ep_num = 0; ep_num < USB_NUM_ENDPOINTS; ep_num++) EP_REG(ep_num) = USB_EP_RX_VALID | USB_EP_TX_NAK | usb_endpoints[ep_num].type | ep_num;
	}	else if (USB->ISTR & USB_ISTR_WKUP) {
		USB->ISTR = (uint16_t)(~USB_ISTR_WKUP);
		USB->CNTR &= ~USB_CNTR_FSUSP;
	} else if (USB->ISTR & USB_ISTR_SUSP) {
		USB->ISTR = (uint16_t)(~USB_ISTR_SUSP);
		USB->CNTR |= USB_CNTR_FSUSP;
		USB->DADDR = USB_DADDR_EF;
	}
}




