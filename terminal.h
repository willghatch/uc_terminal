// terminal.h
#ifndef __TERMINAL
#define __TERMINAL

#include "lcd.h"

#define ROWS CHARS_VERT_ON_X // Num lines
#define COLS CHARS_HORIZ_ON_X // Num columns



void flushScreen(void);
void bufClear(void);
void handleAscii(unsigned char *buf, int bytes);

#endif

