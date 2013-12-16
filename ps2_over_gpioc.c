//ps2_over_gpioc.c

#include "stm32f10x.h"
#include "ps2_over_gpioc.h"
#include "usart2.h"
#include "code_to_ascii.h"
#include "lcd.h"

unsigned char ps2_data[PS2_DATA_SIZE];
int ps2_bytes_rec = 0;

extern unsigned char map[0x100];
extern unsigned char shift_map[0x100];
extern unsigned char ctl_map[0x100];

void ps2_insert_to_buffer(char *insert, int size)
{
	int i;
	
	if (ps2_bytes_rec + size > PS2_DATA_SIZE)
		return;
	
	NVIC->ICER[0] = NVIC_ICER_CLRENA_10;
	for (i = 0; i < size; ++i)
	{
		ps2_data[ps2_bytes_rec++] = insert[i];
	}
	NVIC->ISER[0] = NVIC_ISER_SETENA_10;
}

// Switch PC0 to PC4, and PC1 to PC3
void ps2_over_gpioc_init(void)
{
	// Setup EXTI0
	EXTI->IMR |= EXTI_IMR_MR4;
	//EXTI->EMR |= EXTI_EMR_MR4;
	EXTI->RTSR |= EXTI_RTSR_TR4;
    
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
	
	// Setup AFIO events
	RCC->APB2ENR |= 1; // enable AFIO clock
	//AFIO->EVCR = 0b10100000; // Events enabled on PC0
	//AFIO->EVCR = AFIO_EVCR_EVOE | AFIO_EVCR_PORT_PC | AFIO_EVCR_PIN_PX0;
	AFIO->EXTICR[1] = (AFIO->EXTICR[2] & 0xFFFFFFF0) | 0x2;
	GPIOC->CRL = (GPIOC->CRL & 0xFFF00FFF) | 0x88000; // Configure PC[3-4] for pull-up/pull-down input
	GPIOC->ODR |= 0x18; // set  bits of ODR so we can read the input.
	
	// Setup NVIC
	// EXT4 is interrupt number 10
	NVIC->ISER[0] = NVIC_ISER_SETENA_10;
	
	// Set priority - this should be our high priority interrupt
	//NVIC->IP[1] = (NVIC->IP[1] & 0xFF00FFFF) | 0x00FF0000;

}

void EXTI4_IRQHandler(void)
{
	static unsigned int calls = 0;
	static unsigned char rx_state = 0; // 0 not receiving, 1 receiving
	static unsigned char data_bits_rec = 0;
	static unsigned char data = 0;
	static unsigned char stop_bit_rec = 0;
	static unsigned char last_byte_was_escape = 0;
	static unsigned char shift_on = 0;	
	static unsigned char ctl_on = 0;
	static unsigned char alt_on = 0;
	
	unsigned char bit, ascii;
	
	calls++;
	bit = GPIOC->IDR & 0x8; // read pc3 (data line)
	bit = bit << 4;
	while(1)
  {
	// Handle start bit
	if (!rx_state)
	{
		if(bit)
			break;
		rx_state = 1;
		data_bits_rec = 0;
		data = 0;
		stop_bit_rec = 0;
		
		break;
	}
	
	// Receive data
	if (data_bits_rec < 8)
	{
		data = data >> 1;
		data |= bit;
		data_bits_rec++;
		break;
	}
	
	// Handle stop/acknowledge bits
	if (! stop_bit_rec)
	{
		stop_bit_rec = 1;
		
		
		// Map scancodes to ascii, throwing away everything but lowercase
		// alphanumeric characters and spaces.
		
		if (last_byte_was_escape)
		{
			last_byte_was_escape = 0;
			if(shift_on && (data == 0x12 | data == 0x59))
				shift_on = 0;
			if(ctl_on && (data == 0x14))
				ctl_on = 0;
			if(alt_on && (data == 0x11))
				alt_on = 0;
			break;
		}
		if (data == 0xF0) // key up escape
		{
			last_byte_was_escape = 1;
			break;
		}
		
		if (data == 0x12 | data == 0x59) // key down shift
		{
			shift_on = 1;
			break;
		}
		if (data == 0x14) // key down ctl
		{
			ctl_on = 1;
			break;
		}
		if (data == 0x11) // key down alt
		{
			alt_on = 1;
			break;
		}
				
		if (ctl_on)
			ascii = ctl_map[data];
		else if (shift_on)
			ascii = shift_map[data];
		else
			ascii = map[data];
		

		
		
		// put data into buffer
		if (ps2_bytes_rec < PS2_DATA_SIZE-1)
		{
			if(alt_on)
			{
				ps2_data[ps2_bytes_rec] = 0x1b; // escape
				ps2_bytes_rec++;
			}
			ps2_data[ps2_bytes_rec] = ascii;
			//ps2_data[ps2_bytes_rec] = data;
			ps2_bytes_rec++;

		}

		break;
	}
	
	
	// Here we will let one clock cycle pass, because there may
	// be an acknowledgement bit, but we're not sure how to handle it.
	
	rx_state = 0;
	break;
    }
		
	  //NVIC->ICPR[0] = 0x40;
		EXTI->PR = 0x10; // Clear EXTI pending
		NVIC->ICPR[0] = NVIC_ICPR_CLRPEND_10;
    return;
}

void ps2_dump_data_over_usart2()
// Dumps data in ps2_data over usart2
{
	int i;
	
	for (i = 0; i < ps2_bytes_rec; ++i)
	{
		usart2_tx(ps2_data[i]);
		LCD_DrawCharacterOnY(40,40,Blue, Black, ps2_data[i]);
	}
	ps2_bytes_rec =0;
}

int ps2_memcpy(unsigned char * dst)
{
	int ret, i;
	// turn off interrupts
	NVIC->ICER[0] = NVIC_ICER_CLRENA_10;
	// copy buffer
	for(i = 0; i < ps2_bytes_rec; ++i)
	{
		dst[i] = ps2_data[i];
	}
	ret = ps2_bytes_rec;
	
	ps2_bytes_rec = 0;
	// enable interrupts
	NVIC->ISER[0] = NVIC_ISER_SETENA_10;
	return ret;
}



