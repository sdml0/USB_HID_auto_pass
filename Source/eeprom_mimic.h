#include "stm32f10x.h"
//#include "air32f10x.h"

void ee_find_data(uint16_t** pointers, uint8_t max_id, uint8_t max_size);
void ee_save_data(uint8_t id, uint8_t* data, uint8_t size);