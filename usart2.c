// usart2.c

#include "usart2.h"


/*
	; baud rate, 0x08, 12bit mantissa plus 4 bits fractional
; BRD = 8e6/(16*9600)= 52.083
  ; integer portion: int(52.083)=52 = 0b00110100      (0xDO if 32Mhz, 0x138 for 48MHz)
  ; fractional portion: int(0.083*2^6+0.5)=6 = 0b0110  (0x32 if 32Mhz, 0x5 for 48MHz)
  ; to put in register: 0b001101000110 = 0x0341         (0xDO3 if 32Mhz, 0x1385 for 48MHz)
BAUD_RATE  DCD 0x0341

; control register 1, 0x0C, bits 31-14 reserved (0), 0b10000000001100 = 0x200C   no interrupts, no break, active receiver, RX and TX enabled
USART_CTRL1_SETTINGS DCD 0x200C
	
	SETUPA  DCD 0x000A8AA8  ; Usart alt function settings -- see usart comments below
	; alternate functionality pins
;USART2_CTS PA0 PD3  - input pull-up (8)
;USART2_RTS PA1 PD4  - alt push-pull  (A)
;USART2_TX PA2 PD5  - alt push-pull
;USART2_RX PA3 PD6  - input pull up
;USART2_CK PA4 PD7  - alt push-pull
	*/

unsigned char usart2_data[USART2_DATA_SIZE];
int usart2_bytes_rec = 0;

void usart2_init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	//GPIOA->CRL = (GPIOA->CRL & 0xFFF00000) | 0x000A8AA8;
	GPIOA->CRL = (GPIOA->CRL & 0xFFF00000) | 0x000B8BB8;
	
	//USART2->BRR = 0x0341;
	USART2->BRR = 0x1385;
	USART2->CR1 = 0x202C; //Enable RXNEIE(bit 5) bit for receive interrupt
	NVIC->ISER[1] = NVIC_ISER_SETENA_6;

}

void usart2_tx(unsigned char byte)
{
	while(!(USART2->SR & USART_SR_TC));
	
	USART2->DR = byte;
}


void USART2_IRQHandler()
{
	if (usart2_bytes_rec < USART2_DATA_SIZE)
	{
		usart2_data[usart2_bytes_rec] = USART2->DR;
		usart2_bytes_rec++;
	}

	NVIC->ISER[1] = NVIC_ICPR_CLRPEND_6;
}

int usart2_memcpy(unsigned char * dst)
// Copies the usart2 buffer, then resets the usart2 buffer
{
	int ret, i;
	
	// turn off interrupts
	NVIC->ICER[1] = NVIC_ICER_CLRENA_6;
	
	// copy buffer
	for(i = 0; i < usart2_bytes_rec; ++i)
	{
		dst[i] = usart2_data[i];
	}
	ret = usart2_bytes_rec;
	usart2_bytes_rec = 0;
	
	// enable interrupts
	NVIC->ISER[1] = NVIC_ISER_SETENA_6;

	return ret;
}
