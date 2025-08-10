#include "usb_core.h"
#include "eeprom_mimic.h"

//TODO: HID, Buttons, EEPROM, Pass gen, CDC

#define ID_QUANTITY 17
#define MAX_PASS_SIZE 60
#define DOUBLE_TIME 100
#define SHORT_TIME 11
#define LONG_TIME 200
#define BLINK_TIME 210
#define USB_FNR (*(uint32_t*)0x40005C48)
const uint8_t ASCIISpecLookup[32] = { 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
			58, 59, 60, 61, 62, 63, 64, 91, 92, 93, 94, 95, 96, 123, 124, 125, 126 };

/*
all: 33-126
0-9: 48-57, a-z: 97-122, A-Z: 65-90	(26)
spec: 33-47, 58-64, 91-96, 123-126
*/
uint32_t RandomNumber = 0xABCDEF;
uint8_t Action = 0, Button = 0;
uint16_t Blink_timer = 0;
			
uint32_t next_rand()
{
	static uint32_t lfsr;
	if (lfsr < 0xFFFF) lfsr = RandomNumber + (USB_FNR<<5);
	lfsr ^= lfsr>>7;
	lfsr ^= lfsr<<9;
	lfsr ^= lfsr>>13;
	return lfsr;
}

void pass_gen(uint8_t* pass, uint8_t len, uint8_t spec) // len >= 4
{
	uint32_t r = next_rand();
	
	*pass = 97 + r%26; r = r/26; pass++;
	*pass = 65 + r%26; r = r/26;  pass++;
	if (spec) {
		*pass = ASCIISpecLookup[r%32];
		r = r/32;  pass++; len--;
	}
	*pass = 48 + r%10;
	len -= 3; pass++;
	while(len) {
		*pass = 33 + (next_rand()>>3)%94;
		if (!spec)
			for (uint8_t i = 0; i < sizeof(ASCIISpecLookup); i++)
				if (*pass == ASCIISpecLookup[i]) { len++; pass--; break; }
		len--; pass++;
	}
}

static inline void print_string(uint8_t* str, uint8_t len)
{
	while(len--) send_keypress(*str++);
}

void SysTick_Handler()
{
	// handle only one button press
	static uint16_t hold = 0, release = 0;
	
	if (Blink_timer) {		// blink during CDC
		Blink_timer--;
		if (Blink_timer == 1) {
			Blink_timer = BLINK_TIME;
			BB_REG(&GPIOC->ODR)[13] = 1 - BB_REG(&GPIOC->ODR)[13];
		}
		return;
	}
	
	if (GPIOB->IDR) {
		hold++;
		if (hold == SHORT_TIME) Button = 31 - __CLZ(GPIOB->IDR);
		if (hold == LONG_TIME) BB_REG(&GPIOC->ODR)[13] = 1 - BB_REG(&GPIOC->ODR)[13];		// will be long click
		return;
	}
	if (release) {
		release++;
		if (release > DOUBLE_TIME) { release = 0; Action = 1; }	// ...only one short click
	}
	if (hold >= SHORT_TIME && hold < LONG_TIME) {
		if (!release) release = 1;															// short click...
		else { release = 0; Action = 3; }			// ...double click
	} else if (hold >= LONG_TIME) Action = 2;		// long click
	hold = 0;
	RandomNumber++;
}

void buttons_init()
{
	/* led */	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
	/* led */	GPIOC->CRH = 0x44644444;				//2MHz max, OD only
	/* led */	BB_REG(&GPIOC->ODR)[13] = 1;
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;
	AFIO->MAPR = 2<<24;					// Disable JTAG -> release pb3 pb4
	RCC->APB2ENR &= ~RCC_APB2ENR_AFIOEN;
	GPIOB->CRL = 0x88888888;
	GPIOB->CRH = 0x88888888;
	GPIOB->ODR = 0;						// Pull-Down
	SysTick->LOAD = 9000*4 - 1;		// 4ms
	SysTick->CTRL = SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

int main()
{
	uint8_t i, pg = 0, sz = 14;
	uint16_t* pointers[2*ID_QUANTITY];		// 34*4 + 
	uint8_t char_buf = 0;
	uint8_t ps[MAX_PASS_SIZE] = {0};		// 60		= 196
	
	buttons_init();
	ee_find_data(pointers, ID_QUANTITY, MAX_PASS_SIZE);
	if (!GPIOB->IDR) usb_io_init(0);
	else {
		Blink_timer = BLINK_TIME;
		usb_io_init(&char_buf);
		pg = 2;
	}
		
	if (pg) {
		pg = 0; sz = 0;
		while(1) if(char_buf) {		// one char at a time, paste doesn't work
			if (char_buf != 0x0D) {
				if (char_buf != 0x08) {
					ps[sz] = char_buf;
					if (sz < MAX_PASS_SIZE - 1) sz++;
				} else if (sz) sz--;
			} else {
				if (pg == 17) {
					if (sz == 2 && ps[0] >= '1' && ps[0] <= '5' && ps[1] >= '0' && ps[1] <= '9') ps[0] = 10*(ps[0] - '0') + ps[1] - '0';
					else { pg = 0; sz = 0; continue; }
					ps[1] = 0;
				}
				if (pg) {
					ee_save_data(pg - 1, ps, sz);
					cdc_print_string("Saved...\r\n", 10);
					pg = 0;
				} else {
					if (sz && sz < 3) {
						if (sz == 2 && ps[0] == '1' && ps[1] >= '0' && ps[1] <= '7') pg = 10 + ps[1] - '0';
						if (sz == 1 && ps[0] >= '1' && ps[0] <= '9') pg = ps[0] - '0';
					}
				}
				sz = 0;
			}
			char_buf = 0;
		}
	}
	
	if (pointers[2*ID_QUANTITY-1]) sz = pointers[2*ID_QUANTITY-1][1];
	while (1) if (Action) {
		if(Action == 1) {				// Short press
			if (!pg && pointers[2*Button+1]) {
				print_string((uint8_t*)pointers[2*Button+1] + 2, *pointers[2*Button+1]>>8);
				send_keypress('\n');
			} else if (pg == Button + 1 && pointers[2*Button+1]) {
				print_string((uint8_t*)pointers[2*Button+1] + 2, *pointers[2*Button+1]>>8);
			} else if (pg && pg != Button + 1) {
				if(!ps[0]) {
					if (Button < 8) pass_gen(ps, sz, 1);
					else pass_gen(ps, sz, 0);
				}
				print_string(ps, sz);
			}
		} else if (Action == 2) {		// Long press
			if (!pg) {
				BB_REG(&GPIOC->ODR)[13] = 0;
				pg = Button + 1;
			} else {
				if (pg == Button + 1) ee_save_data(Button, ps, sz);
				BB_REG(&GPIOC->ODR)[13] = 1;
				pg = 0;
				ps[0] = 0;
			}
		} else if (Action == 3 && pointers[2*Button]) {		// Double press
			print_string((uint8_t*)pointers[2*Button] + 2, *pointers[2*Button]>>8);
			if(!pg) send_keypress('\n');
		}
		Action = 0;
	}

}


