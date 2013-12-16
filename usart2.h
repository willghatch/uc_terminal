// usart2.h

#ifndef __USART2
#define __USART2
#include "stm32f10x.h"


#define USART2_DATA_SIZE 1000

void usart2_init(void);

void usart2_tx(unsigned char byte);
int usart2_memcpy(unsigned char * dst);

#endif
