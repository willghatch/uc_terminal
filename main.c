// main.c
// ECE 3710
// Final Project
// By William Hatch and Scott Sorensen


#include "stm32f10x.h"
#include "lcd.h"
#include "usart2.h"
#include "ps2_over_gpioc.h"
#include "code_to_ascii.h"
#include "terminal.h"
#include "dac.h"



void SystemInit(void)
{
}

void handlePs2Data(void);
void handleUsartData(void);



extern int cursorCol, cursorRow;
extern char screenChars[ROWS][COLS];
extern unsigned short screenFgColor[ROWS][COLS];
extern unsigned short screenBgColor[ROWS][COLS];



int main()
{
	// Enable external oscillator
  //RCC->CFGR = 0x0418000A;                     // Mult PLL by 8 = 32 MHz
	RCC->CFGR = 0x0428000A;                     // 48MHz
	/// 418->428 makes it 48Mhz
  RCC->CR = 0x03004583; //USe PLL Clock for SW and MC
	
	
	// Initialize everything
	LCD_Initialization();
	usart2_init();
	ps2_over_gpioc_init();
	
	LCD_Clear(Black);
	scan_code_init();
	DAC_init();
	bufClear();
	
	while(1)
	{
		handlePs2Data(); 
		handleUsartData();
		flushScreen();
	}
	
}

void handlePs2Data()
{
	int bytes;
	int i;
	unsigned char buf[PS2_DATA_SIZE];

	// copy buffer
	bytes = ps2_memcpy(buf);

	// tx buffer
	for (i = 0; i < bytes; ++i)
	{
		usart2_tx(buf[i]);
	}
}

void handleUsartData()
{
	int bytes;
	unsigned char buf[USART2_DATA_SIZE];
	// copy buffer
	bytes = usart2_memcpy(buf);
	
	// push buffered data to display
	handleAscii(buf, bytes);
}


void HardFault_Handler()
{
	LCD_Clear(Red);
	//Reset_Handler();
}

