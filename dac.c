// dac.c

#include "stm32f10x.h"
#include "dac.h"


static int wave[40] = {
	2047,2367,2679,2976,3250,3494,3703,3870,3993,4068,4094,4068,3993,3870,3703,3494,3250,2976,
2679,2367,2047,1726,1414,1117,843,599,390,223,100,25,0,25,100,223,390,599,843,1117,1414,1726
};
int beepCount = 0;


void DAC_beep(void)
{
	beepCount = 100;
}


void Tim3_init()
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	NVIC->ISER[0] = NVIC_ISER_SETENA_29; 
	NVIC->IP[7] = 0; // Highest priority!
	TIM3->CR1 = 0x94; // Count down, restart automatically, only update on under/overflow
	TIM3->DIER = 1; // enable interrupt on timer finish
	
	TIM3->ARR = 0xFFFF; // 8000 is 1ms on 8Mhz clock
	
	TIM3->CR1 |= 1; // enable timer.
}

void TIM3_IRQHandler()
// Interrupt on ISER[0]0x20000000
// Output new DAC value along the wave form
{
	if (beepCount)
	{
		static int count = 0;
		DAC->DHR12R2 = wave[count++];
		//DAC->SWTRIGR = 2;
		if (count == 40)
			count = 0;
		beepCount--;
	}
	
	
	// reset interrupt pending in NVIC
	TIM3->SR &= 0xFFFFFFFE;
	NVIC->ICPR[0] = NVIC_ICPR_CLRPEND_29;
}

void DAC_init()
{
	// Enable gpio clock
	RCC->APB1ENR |= RCC_APB1ENR_DACEN;
	// Setup GPIOS
	// DAC_OUT2 is PA5
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	GPIOA->CRL = (GPIOA->CRL & 0xFF0FFFFF) | 0x00B00000; // PC5 set to output AF push-pull
	
	// Write configs
	DAC->CR = 0x010000; // enable DAC channel 2, turn off buffering
	//DAC->CR |= 0x3C0000; // enable triggers on software trigger
	
	Tim3_init();
}


