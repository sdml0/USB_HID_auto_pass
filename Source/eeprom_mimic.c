#include "eeprom_mimic.h"
#define FLASH_END 0x8020000	// 128kb stangly works
#define DATA_PAGES 6				// even, flash page is 1kb
#define DATA0_START (FLASH_END - 1024*DATA_PAGES)
#define DATA1_START (FLASH_END - 512*DATA_PAGES)

uint16_t* New_data;
uint16_t** ExtPointersLocation;
uint8_t IdQuantity;

static inline void erase_page(uint32_t adr)
{
	BB_REG(&FLASH->CR)[0] = 0;		// PG
	BB_REG(&FLASH->CR)[1] = 1;		// PER
	FLASH->AR = adr;
	BB_REG(&FLASH->CR)[6] = 1;		// STRT erase
	while (FLASH->SR & FLASH_SR_BSY);
	BB_REG(&FLASH->CR)[1] = 0;
	BB_REG(&FLASH->CR)[0] = 1;	
}

static inline void program16bit(uint32_t adr, uint16_t data)
{
	*(uint16_t*)adr = data;
	while (FLASH->SR & FLASH_SR_BSY);	
}

void ee_find_data(uint16_t** pointers, uint8_t id_quantity, uint8_t max_size)	// max_id < 255, pointers array has double size
{
	uint16_t* ptr;
	uint16_t sz;
	uint8_t id;
	
	ExtPointersLocation = pointers;
	IdQuantity = id_quantity;
	for (id = 0; id < 2*id_quantity; id++) pointers[id] = 0;
	id = 0; New_data = (uint16_t*)FLASH_END - 1;
	if (!*(uint16_t*)DATA0_START) ptr = (uint16_t*)DATA1_START;
	else ptr = (uint16_t*)DATA0_START;
	while (*ptr != 0xFFFF) {
		id = *ptr&0xFF; sz = *ptr>>8;
		if(id && (id <= id_quantity) && sz && (sz <= max_size)) {
			pointers[2*id-2] = pointers[2*id-1];
			pointers[2*id-1] = ptr;
			ptr += ((sz+1)>>1) + 1;
		} else { id  = 0; break; }
	}
	if (id) {
		New_data = ptr;
		sz = 1022 - (((uint32_t)ptr)&1023);
		ptr++;
		while(sz && *ptr == 0xFFFF) sz-=2;
	}
	if (sz) {
		New_data = (uint16_t*)FLASH_END - 1;
		for (id = 0; id < 2*id_quantity; id++) pointers[id] = 0;
	}
}

void ee_save_data(uint8_t id, uint8_t* data, uint8_t size)
{
	uint8_t i, sz;
	uint16_t* ptr;
	uint32_t tail = ((uint32_t)New_data + 2 + size + (size&1))&(~1023);
	
	FLASH->KEYR = 0x45670123;
	FLASH->KEYR = 0xCDEF89AB;
	BB_REG(&FLASH->CR)[0] = 1;		// PG
	
	if(((uint32_t)New_data^tail) >> 10) {
		if (tail >= FLASH_END) tail = DATA0_START;
		if (tail == DATA1_START) program16bit(DATA0_START, 0);
		erase_page(tail);
		if (tail == DATA0_START || tail == DATA1_START) {		// Max amount of all data must be less than page
			for (i = 0; i < 2*IdQuantity; i++) {
				ptr = ExtPointersLocation[i];
				if (ptr && (i != 2*id)) {
					ExtPointersLocation[i] = (uint16_t*)tail;
					sz = ((*ptr>>8) + 1)/2 + 1;
					while (sz--) { program16bit(tail, *ptr++); tail+=2; }
				}
			}
			New_data = (uint16_t*)tail;
		}
	}
	
	ptr = New_data;
	ExtPointersLocation[2*id] = ExtPointersLocation[2*id+1];
	ExtPointersLocation[2*id+1] = New_data;
	*ptr = ((uint16_t)size<<8) | (id+1);
	ptr++;
	for(sz = 0; sz < size; sz+=2, data+=2) program16bit((uint32_t)ptr++, *(uint16_t*)data);
	New_data = ptr;
	BB_REG(&FLASH->CR)[0] = 0;		// PG
	BB_REG(&FLASH->CR)[7] = 1;		// LOCK
}


