//
//  lcd.h
//  ECE 3710 Microcontroller H&S
//  Utah State University
//

#ifndef __LCD_H
#define __LCD_H

#include "stm32f10x.h"

#define DISP_ORIENTATION 90

#if  ( DISP_ORIENTATION == 90 ) || ( DISP_ORIENTATION == 270 )

#define  MAX_X  320
#define  MAX_Y  240
//#define  CHARS_HORIZ_ON_Y 30
#define  CHARS_HORIZ_ON_Y 28
#define  CHARS_VERT_ON_Y 20
#define  CHARS_HORIZ_ON_X 40
#define  CHARS_VERT_ON_X 15

#elif  ( DISP_ORIENTATION == 0 ) || ( DISP_ORIENTATION == 180 )

#define  MAX_X  240
#define  MAX_Y  320
#define  CHARS_HORIZ_ON_Y 40
#define  CHARS_VERT_ON_Y 15

#endif


/* some LCD colors */
#define White          0xFFFF
#define Black          0x0000
#define Grey           0xF7DE
#define Blue           0x001F
#define Blue2          0x051F
#define Red            0xF800
#define Magenta        0xF81F
#define Green          0x07E0
#define Cyan           0x7FFF
#define Yellow         0xFFE0


#define TermBlack Black
#define TermBlackBright 0x52aa
#define TermRed 0xa800
#define TermRedBright 0xfaaa
#define TermGreen 0x540
#define TermGreenBright 0x57ea
#define TermBrown 0xaaa0
#define TermBrownBright 0xffea
#define TermBlue 0x15
#define TermBlueBright 0x52bf
#define TermMagenta 0xa815
#define TermMagentaBright 0xfabf
#define TermCyan 0x555
#define TermCyanBright 0x57ff
#define TermWhite 0xa554
#define TermWhiteBright White
#define TermDefault TermWhite
#define TermDefaultBright TermWhiteBright


void LCD_Config(void);
void LCD_Initialization(void);
void LCD_Clear( unsigned short Color );

void LCD_WriteIndex( unsigned short index );
void LCD_WriteData( unsigned short data );
void LCD_Write_Generic(unsigned short toWrite, unsigned short dataBool);
void LCD_WriteReg( unsigned short LCD_Reg, unsigned short LCD_RegValue );

void LCD_SetCursor( unsigned short x, unsigned int y );
void delay_ms( unsigned int ms );

void LCD_DrawSquare( unsigned short x, unsigned short y, unsigned short h, unsigned short w, unsigned short color );


void LCD_DrawCharacterOnY (unsigned short x, unsigned short y, unsigned short fgColor, unsigned short bgColor, unsigned char symbol);
void LCD_WriteCharactersOnY (unsigned short x, unsigned short y, unsigned short fgColor, unsigned short bgColor, char* words, int maxLength);
void LCD_WriteLinesOnY(unsigned short x, unsigned short fgColor, unsigned short bgColor, char* words, char drawToLineEnd);

void LCD_DrawChar_rc (unsigned int row, unsigned int col, unsigned short fgColor, unsigned short bgColor, unsigned char symbol, unsigned char underline);

#endif

//  END OF FILE
