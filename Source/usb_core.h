#include "stm32f10x.h"
//#include "air32f10x.h"

void usb_io_init(uint8_t* one_char_buf);
void send_keypress(uint8_t key);
void cdc_print_string(char* str, uint8_t len);
