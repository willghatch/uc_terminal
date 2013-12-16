//
//  lcd.c
//  ECE 3710 Microcontroller H&S
//  Utah State University
//  Written by Kelly Hathaway
//  And William Hatch and Scott Sorensen
//

#include "stm32f10x.h"
#include "lcd.h"
#include "ascii.h"

#define WR_low_Pin  0x0002  // Pin 1
#define RD_low_Pin  0x0004  // Pin 2
#define CS_low_Pin  0x0040  // Pin 6
#define DC_Pin      0x0080  // Pin 7


//  configuration of the LCD port pins
void LCD_Config(void)
{
    unsigned int config_temp;
    
    RCC->APB2ENR |= 0x1D;       // Enable port A, B, and C
    
    config_temp  = GPIOA->CRL;  // Pin A.3 for Back light
    config_temp &= ~0x0000F000;
    config_temp |=  0x00003000;
    GPIOA->CRL   = config_temp;
    
    GPIOB->CRL   = 0x33333333;  // Port B for Data[15:0] pins
    GPIOB->CRH   = 0x33333333;
    
    config_temp  = GPIOC->CRL;  // PC.0(LCD RST), PC.1(WR), PC.2(RD) , PC.6(CS), PC.7(DC)
    config_temp &= ~0xFF000FFF;
    config_temp |=  0x33000333;
    GPIOC->CRL   = config_temp;
}

void LCD_Initialization(void)
{
    unsigned int config_temp;
    
    LCD_Config();
    
    config_temp  = AFIO->MAPR;  // enable SW Disable JTAG
    config_temp &= ~0x07000000;
    config_temp |=  0x02000000;
    AFIO->MAPR   = config_temp;
    
    GPIOC->BRR  = 0x0001;   // LCD reset
    delay_ms(100);
    GPIOC->BSRR = 0x0001;
    GPIOA->BSRR = 0x0008;   // back light
    
    LCD_WriteReg(0x0000,0x0001);    delay_ms(50);   /* Enable LCD Oscillator */
    LCD_WriteReg(0x0003,0xA8A4);    delay_ms(50);   // Power control(1)
    LCD_WriteReg(0x000C,0x0000);    delay_ms(50);   // Power control(2)
    LCD_WriteReg(0x000D,0x080C);    delay_ms(50);   // Power control(3)
    LCD_WriteReg(0x000E,0x2B00);    delay_ms(50);   // Power control(4)
    LCD_WriteReg(0x001E,0x00B0);    delay_ms(50);   // Power control(5)
    LCD_WriteReg(0x0001,0x2B3F);    delay_ms(50);   // Driver Output Control /* 320*240 0x2B3F */
    LCD_WriteReg(0x0002,0x0600);    delay_ms(50);   // LCD Drive AC Control
    LCD_WriteReg(0x0010,0x0000);    delay_ms(50);   // Sleep Mode off
   // LCD_WriteReg(0x0011,0x6070);    delay_ms(50);   // Entry Mode                    ## flip bit 3 to switch horiz/vert auto-update on write
		LCD_WriteReg(0x0011,0x6078);    delay_ms(50);   // Entry Mode                    ## flip bit 3 to switch horiz/vert auto-update on write
    LCD_WriteReg(0x0005,0x0000);    delay_ms(50);   // Compare register(1)
    LCD_WriteReg(0x0006,0x0000);    delay_ms(50);   // Compare register(2)
    LCD_WriteReg(0x0016,0xEF1C);    delay_ms(50);   // Horizontal Porch
    LCD_WriteReg(0x0017,0x0003);    delay_ms(50);   // Vertical Porch
    LCD_WriteReg(0x0007,0x0133);    delay_ms(50);   // Display Control
    LCD_WriteReg(0x000B,0x0000);    delay_ms(50);   // Frame Cycle control
    LCD_WriteReg(0x000F,0x0000);    delay_ms(50);   // Gate scan start position
    LCD_WriteReg(0x0041,0x0000);    delay_ms(50);   // Vertical scroll control(1)
    LCD_WriteReg(0x0042,0x0000);    delay_ms(50);   // Vertical scroll control(2)
    LCD_WriteReg(0x0048,0x0000);    delay_ms(50);   // First window start
    LCD_WriteReg(0x0049,0x013F);    delay_ms(50);   // First window end
    LCD_WriteReg(0x004A,0x0000);    delay_ms(50);   // Second window start
    LCD_WriteReg(0x004B,0x0000);    delay_ms(50);   // Second window end
    LCD_WriteReg(0x0044,0xEF00);    delay_ms(50);   // Horizontal RAM address position
    LCD_WriteReg(0x0045,0x0000);    delay_ms(50);   // Vertical RAM address start position
    LCD_WriteReg(0x0046,0x013F);    delay_ms(50);   // Vertical RAM address end position
    LCD_WriteReg(0x0030,0x0707);    delay_ms(50);   // gamma control(1)
    LCD_WriteReg(0x0031,0x0204);    delay_ms(50);   // gamma control(2)
    LCD_WriteReg(0x0032,0x0204);    delay_ms(50);   // gamma control(3)
    LCD_WriteReg(0x0033,0x0502);    delay_ms(50);   // gamma control(4)
    LCD_WriteReg(0x0034,0x0507);    delay_ms(50);   // gamma control(5)
    LCD_WriteReg(0x0035,0x0204);    delay_ms(50);   // gamma control(6)
    LCD_WriteReg(0x0036,0x0204);    delay_ms(50);   // gamma control(7)
    LCD_WriteReg(0x0037,0x0502);    delay_ms(50);   // gamma control(8)
    LCD_WriteReg(0x003A,0x0302);    delay_ms(50);   // gamma control(9)
    LCD_WriteReg(0x003B,0x0302);    delay_ms(50);   // gamma control(10)
    LCD_WriteReg(0x0023,0x0000);    delay_ms(50);   // RAM write data mask(1)
    LCD_WriteReg(0x0024,0x0000);    delay_ms(50);   // RAM write data mask(2)
    LCD_WriteReg(0x0025,0x8000);    delay_ms(50);   // Frame Frequency
    LCD_WriteReg(0x004f,0);                         // Set GDDRAM Y address counter
    LCD_WriteReg(0x004e,0);                         // Set GDDRAM X address counter
    
    delay_ms(50);
}

// Paints the LCD with Color
void LCD_Clear( unsigned short Color )
{
    unsigned int i;
    
    LCD_SetCursor(0,0);
    
    GPIOC->BRR  = CS_low_Pin;
    
    LCD_WriteIndex( 0x0022 );
    for( i=0; i< MAX_X*MAX_Y; i++ )
        LCD_WriteData( Color );
    
    GPIOC->BSRR = CS_low_Pin;
}

// Write a command
void LCD_WriteIndex( unsigned short index )
{
  LCD_Write_Generic(index, 0);
}

// Write data
void LCD_WriteData( unsigned short data )
{    
  LCD_Write_Generic(data, 1);
}

// Write generic...
void LCD_Write_Generic(unsigned short toWrite, unsigned short dataBool)
{
	unsigned short pc_ops = GPIOC->ODR;
	
	// Configure Ports - done in LCD init function
	// Set control bits (RD, WR, D/C, CS)
	// PC.0(LCD RST = ?), PC.1(WR = 1, then 0, then 1), PC.2(RD = 1) , PC.6(CS = unset then set), PC.7(DC = dataBool)

	pc_ops &= 0xFF38; // unset RST,WR,RD,CS,DC
	pc_ops |= 0x0007; // set RD = 1, and WR = 1, and RST = 1
	if (dataBool) pc_ops |= 0x80; // set DC = 1 if we want to write data

	GPIOC->ODR = pc_ops;
	delay_ms(0);
	
	// Write data bits
	GPIOB->ODR = toWrite;
	delay_ms(0);
	
	GPIOC->ODR = (pc_ops & 0xFFFD); // unset WR
	GPIOC->ODR = pc_ops; // set WR
	
}

// 
void LCD_WriteReg( unsigned short LCD_Reg, unsigned short LCD_RegValue )
{
    GPIOC->BRR  = CS_low_Pin;
    
    LCD_WriteIndex( LCD_Reg );
    LCD_WriteData( LCD_RegValue );
    
    GPIOC->BSRR = CS_low_Pin;
}

// Set cursor to x y address
void LCD_SetCursor( unsigned short x, unsigned int y )
{
    #if   ( DISP_ORIENTATION == 90 ) || ( DISP_ORIENTATION == 270 )
    
        unsigned short swap_temp;
    
        y = (MAX_Y-1) - y;
        swap_temp = y;
        y = x;
        x = swap_temp;
    
    #elif ( DISP_ORIENTATION ==  0 ) || ( DISP_ORIENTATION == 180 )
    
        y = (MAX_Y-1) - y;
    
    #endif
    
    LCD_WriteReg( 0x004E, x );
    LCD_WriteReg( 0x004F, y );
}

void delay_ms( unsigned int ms )
{
	int i;
	while(ms--)
	{
		//for(i = 0; i < 1669; ++i); // 1 ms delay loop
		for(i = 0; i < 8676; ++i); // 1 ms delay loop
	}
}


void LCD_DrawSquareY( unsigned short x, unsigned short y, unsigned short w, unsigned short h, unsigned short color )
{
	unsigned int i,j;
    
    LCD_SetCursor(x,y);
    
    GPIOC->BRR  = CS_low_Pin;
    

		for (j=0; j < w; j++ )
	  {
			LCD_SetCursor(x+j, y);
			LCD_WriteIndex( 0x0022 );
			for( i=0; i < h; i++ )
					LCD_WriteData( color );
    }
		
    GPIOC->BSRR = CS_low_Pin;
}
void LCD_DrawSquare( unsigned short x, unsigned short y, unsigned short w, unsigned short h, unsigned short color )
{
	unsigned int i,j;
    
    LCD_SetCursor(x,y);
    
    GPIOC->BRR  = CS_low_Pin;
    

		for (j=0; j < w; j++ )
	  {
			LCD_SetCursor(x, y+j);
			LCD_WriteIndex( 0x0022 );
			for( i=0; i < h; i++ )
					LCD_WriteData( color );
    }
		
    GPIOC->BSRR = CS_low_Pin;
}

void LCD_DrawCharacterOnY (unsigned short x, unsigned short y, unsigned short fgColor, unsigned short bgColor, unsigned char symbol)
// Draws a character oriented so that left to right goes along the positive y axis
{
	unsigned char ascii_buf[16];
	unsigned char line;
	int i, j;
	
	LCD_SetCursor(x,y);
	GPIOC->BRR = CS_low_Pin;
	
	get_ascii(ascii_buf, symbol);
	
	for (i = 0; i < 16; ++i)
	{
		line = ascii_buf[i];
		LCD_SetCursor(x+i, y);
		LCD_WriteIndex( 0x0022 );
		for (j = 0; j < 8; ++j)
		{
			if (line & (0x80 >> j))
				LCD_WriteData( fgColor );
			else
				LCD_WriteData( bgColor );
			//delay_ms(1);
		}
	}
	GPIOC->BSRR = CS_low_Pin;
}
void LCD_DrawCharacterOnX (unsigned short x, unsigned short y, unsigned short fgColor, unsigned short bgColor, unsigned char symbol, unsigned char underline)
// Draws a character oriented so that left to right goes along the positive X axis
{
	unsigned char ascii_buf[16];
	unsigned char line;
	int i, j;
	
	LCD_SetCursor(x,y);
	GPIOC->BRR = CS_low_Pin;
	
	get_ascii(ascii_buf, symbol);
	
	for (i = 0; i < 16; ++i)
	{
		line = ascii_buf[i];
		if (i == 15 && underline)
			line = 0xFF;
		LCD_SetCursor(x, y+i);
		LCD_WriteIndex( 0x0022 );
		for (j = 0; j < 8; ++j)
		{
			if (line & (0x80 >> j))
				LCD_WriteData( fgColor );
			else
				LCD_WriteData( bgColor );
			//delay_ms(1);
		}
	}
	GPIOC->BSRR = CS_low_Pin;
}
void LCD_DrawChar_rc (unsigned int row, unsigned int col, unsigned short fgColor, unsigned short bgColor, unsigned char symbol, unsigned char underline)
// Draws a character on the givel row and column
{
	//LCD_DrawCharacterOnY(row * 16, MAX_Y - 12 - (col*8), fgColor, bgColor, symbol);
	LCD_DrawCharacterOnX(col*8, row*16, fgColor, bgColor, symbol, underline);
}

void LCD_WriteCharactersOnY (unsigned short x, unsigned short y, unsigned short fgColor, unsigned short bgColor, char* words, int maxLength)
// Draws a line of characters increasing on Y axis
{
	int i;
	for (i = 0; i < maxLength; ++i)
	{
		if(words[i] == 0)
			break;
		LCD_DrawCharacterOnY(x, y - (8*i), fgColor, bgColor, words[i]);
	}
}
void LCD_WriteLinesOnY(unsigned short lineNumber, unsigned short fgColor, unsigned short bgColor, char* words, char drawToLineEnd)
{
	int len, numLines, lastLineLength, i, curLine;
	char spaces[CHARS_HORIZ_ON_Y];
	len = strlen(words);
	numLines = len / CHARS_HORIZ_ON_Y;
	lastLineLength = len % CHARS_HORIZ_ON_Y;
	if(lastLineLength != 0)
		numLines++;
	
	for(curLine = 0; curLine < numLines; ++curLine)
	{
		LCD_WriteCharactersOnY((lineNumber + curLine) * 16, MAX_Y-12, fgColor, bgColor, words+((CHARS_HORIZ_ON_Y) * curLine), CHARS_HORIZ_ON_Y);
	}
	// TODO: fix draw to end of last line
	if(lastLineLength != 0)
	{
		for(i = 0; i < CHARS_HORIZ_ON_Y; ++i)
		{
			spaces[i] = ' ';
		}
		LCD_WriteCharactersOnY((lineNumber + curLine - 1) * 16, MAX_Y-12-(lastLineLength*8), fgColor, bgColor, spaces, CHARS_HORIZ_ON_Y-lastLineLength);
	}
}




//  END OF FILE
