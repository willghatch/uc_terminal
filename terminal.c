// terminal.c

#include "terminal.h"
#include "lcd.h"
#include "ps2_over_gpioc.h"
#include "dac.h"

#define CURSOR_STACK_SIZE 200
int cursorStack[2][2][CURSOR_STACK_SIZE]; // one for CSI stack, one for ESC (non-CSI stack)
int CSIcursorStackPointer=-1;
int ESCcursorStackPointer=-1;
int cursorCol = 0; // cursor column number
int cursorRow = 0; // cursor line number

unsigned short currFgColor = TermDefault;
unsigned short currBgColor = TermBlack;

unsigned char currDisplayOps = 0; // for flags for underline, blink, etc

char screenChars[ROWS][COLS];
char oldScreenChars[ROWS][COLS];
unsigned short screenFgColor[ROWS][COLS];
unsigned short screenBgColor[ROWS][COLS];
unsigned short oldScreenFgColor[ROWS][COLS];
unsigned short oldScreenBgColor[ROWS][COLS];
int screenTop = 0;
int oldScreenTop = 0;

unsigned char screenDisplayOps[ROWS][COLS];
unsigned char oldScreenDisplayOps[ROWS][COLS];
unsigned char drawCursor = 1;
#define CURSOR_CHAR 1 // attribute for character that the cursor is on
#define UNDERLINE 2
#define BOLD 4
#define BLINK 8
#define REVERSE_VIDEO 16
#define HALF_BRIGHT 32



//////////////////////// Ascii Control Functions //////////////////////

void addCursorAttr()
{ // add cursor attribute to screenChar at current cursor location
	int rowTr; // translated row
	rowTr = (cursorRow + screenTop) % ROWS;
	screenDisplayOps[rowTr][cursorCol] |= CURSOR_CHAR;
}
void remCursorAttr()
{ // remove cursor attribute to screenChar at current cursor location
	int rowTr; // translated row
	rowTr = (cursorRow + screenTop) % ROWS;
	screenDisplayOps[rowTr][cursorCol] &= ~CURSOR_CHAR;
}
void clearExtraneousCursors()
{ // since we have extra cursors, let's add this cludge to clear them.
	int i,j;
	for(i = 0; i < ROWS; ++i)
	{
		for(j = 0; j < COLS; ++j)
		{
			screenDisplayOps[i][j] &= ~CURSOR_CHAR;
		}
	}
	addCursorAttr();
}

void do_LF()
// Do a standard line feed
{
	int i, rowTr;
	
	remCursorAttr();
	if (cursorRow < ROWS -1)
		cursorRow = (cursorRow + 1);
	else
	{
		screenTop = (screenTop+1) % ROWS;
		rowTr = (ROWS - 1 + screenTop) % ROWS;
		for(i = 0; i < COLS; ++i)
		{
			screenChars[rowTr][i] = ' ';
		}
	}
	addCursorAttr();
}

void do_CR()
// Do a standard carriage return
{
	remCursorAttr();
	cursorCol = 0;
	addCursorAttr();
}

void handle_LF()
// Handle the \n character
// In newline mode this gives both LF and CR
{
	do_LF();
	// TODO - check somehow whether newline mode is on (it probably always will be
	// if (newLineMode)
		do_CR();
}

void handle_CR()
// handle CR character
{
	// I think this may be the place to do this line blackout...
	int j;
	for(j = cursorCol; j < COLS; ++j)
	{
		screenChars[(cursorRow + screenTop)%ROWS][j] = ' ';
	}
	do_CR();
}

void advance_cursor()
// advances the cursor, duh.
{
	int i;
	remCursorAttr();
	if(cursorCol >= COLS - 1)
	{   // It seems many terminals don't automatically scroll, and it breaks some functionality.
		  // Unless I make a termcap entry for this terminal or figure out how it works, it breaks functionality of
		  // programs like vim in screen... odd...
		cursorCol = 0;
		if (cursorRow < ROWS -1)
			cursorRow = (cursorRow +1);
		else
		{ // Scroll screen
			screenTop = (screenTop +1) % ROWS;
			for (i = 0; i < COLS; ++i)
			{ // clear the new bottom line to be blank as it scrolls in.
				screenChars[(screenTop+ROWS-1)%ROWS][i] = ' ';
				screenDisplayOps[(screenTop+ROWS-1)%ROWS][i] = 0;
				screenFgColor[(screenTop+ROWS-1)%ROWS][i] = TermDefault;
				screenBgColor[(screenTop+ROWS-1)%ROWS][i] = TermBlack;
			}
		}
	}
	else
		cursorCol++;
	addCursorAttr();
}

void handle_normal(char ascii)
// handles normal characters for printing to the screen
{
	int rowTr; // translated row
	rowTr = (cursorRow + screenTop) % ROWS;
	screenChars[rowTr][cursorCol] = ascii;
	screenFgColor[rowTr][cursorCol] = currFgColor;
	screenBgColor[rowTr][cursorCol] = currBgColor;
	screenDisplayOps[rowTr][cursorCol] = currDisplayOps;
	advance_cursor();
}

void moveCursor(unsigned int num, unsigned int dir)
{
	#define UP 1
	#define DOWN 2
	#define LEFT 3
	#define RIGHT 4
	remCursorAttr();
	if (dir == UP)
	{
		cursorRow -= num;
		if (cursorRow < 0)
			cursorRow = 0;
	}
	else if (dir == DOWN)
	{
		cursorRow += num;
		if (cursorRow > ROWS -1)
			cursorRow = ROWS - 1;
	}
	else if (dir == RIGHT)
	{
		cursorCol += num;
		if (cursorCol > COLS - 1)
			cursorCol = COLS -1;
	}
	else if (dir == LEFT)
	{
		cursorCol -= num;
		if (cursorCol < 0)
			cursorCol = 0;
	}
	addCursorAttr();
}

void handle_colorCodes(unsigned int *csi_numbers, unsigned int csi_numbers_rec)
{
	int i;
	for (i = 0; i < csi_numbers_rec; ++i)
	{
		switch (csi_numbers[i])
		{
			case 0:
				currBgColor = TermBlack;
				currFgColor = TermDefault;
				currDisplayOps = 0;
				break;
			case 1:
				currDisplayOps |= BOLD;
				break;
			case 2:
				// set half-bright
				currDisplayOps |= HALF_BRIGHT;
				break;
			case 4:
				// set underscore
				currDisplayOps |= UNDERLINE;
				break;
			case 5:
				// set blink
				currDisplayOps |= BLINK;
				break;
			case 7:
				// set reverse video (whatever that is)
				currDisplayOps |= REVERSE_VIDEO;
				break;
			case 21:
				// set normal intensity (bold off?)
				currDisplayOps &= ~BOLD;
				break;
			case 22:
				// set normal intensity (half-bright off?)
				currDisplayOps &= ~HALF_BRIGHT;
				break;
			case 24:
				// set underline off
				currDisplayOps &= ~UNDERLINE;
				break;
			case 25:
				// set blink off
				currDisplayOps &= ~BLINK;
				break;
			case 27:
				// set reverse video off
				currDisplayOps &= ~REVERSE_VIDEO;
				break;
			
			// 30-49 are mostly basic color options
			case 30:
				currFgColor = TermBlack;
				if(currDisplayOps & BOLD)
					currFgColor = TermBlackBright;
				break;
			case 31:
				currFgColor = TermRed;
				if(currDisplayOps & BOLD)
					currFgColor = TermRedBright;
				break;
			case 32:
				currFgColor = TermGreen;
				if(currDisplayOps & BOLD)
					currFgColor = TermGreenBright;
				break;
			case 33:
				currFgColor = TermBrown;
				if(currDisplayOps & BOLD)
						currFgColor = TermBrownBright;
				break;
			case 34:
				currFgColor = TermBlue;
				if(currDisplayOps & BOLD)
					currFgColor = TermBlueBright;
				break;
			case 35:
				currFgColor = TermMagenta;
				if(currDisplayOps & BOLD)
					currFgColor = TermMagentaBright;
				break;
			case 36:
				currFgColor = TermCyan;
				if(currDisplayOps & BOLD)
					currFgColor = TermCyanBright;
				break;
			case 37:
				currFgColor = TermWhite;
				if(currDisplayOps & BOLD)
					currFgColor = TermWhiteBright;
				break;
			case 38:
				currFgColor = TermDefault;
				currDisplayOps |= UNDERLINE;
				break;
			case 39:
				currFgColor = TermDefault;
				currDisplayOps &= ~UNDERLINE;
				break;
			case 40:
				currBgColor = TermBlack;
				break;
			case 41:
				currBgColor = TermRed;
				break;
			case 42:
				currBgColor = TermGreen;
				break;
			case 43:
				currBgColor = TermBrown;
				break;
			case 44:
				currBgColor = TermBlue;
				break;
			case 45:
				currBgColor = TermMagenta;
				break;
			case 46:
				currBgColor = TermCyan;
				break;
			case 47:
				currBgColor = TermWhite;
				break;
			case 48:
				// ???
				break;
			case 49:
				currBgColor = TermBlack;
				break;
			
			default:
				break;
		}
	}
}



void keepCursorInBounds()
{
	if (cursorRow >= ROWS)
	{
		cursorRow = ROWS-1;
	}
	else if (cursorRow < 0)
		cursorRow = 0;
	if (cursorCol >= COLS-1)
		cursorCol = COLS-1;
	else if (cursorCol < 0)
		cursorCol = 0;
}

void reportCursorToHost()
{
	char rowColResponse[10];
	char rowNum[3];
	char colNum[3];
	int divisor, i,j;
	char firstSeen = 0;
	int rowsSent=0, colsSent=0;
	// echo ESC[<ROW>;<COL>R
	
	for (i = 0, divisor = 100; divisor; ++i, divisor/=10)
	{
		rowNum[i] = ((cursorRow+1)/divisor) % 10;
		colNum[i] = ((cursorCol+1)/divisor) % 10;
	}
	rowColResponse[0] = 033; // ESC
	rowColResponse[1]='[';
	for(i = 2, j = 0; j < 4; ++j) // i is response position, j is loop position
	{
		if(rowNum[j] || j == 3) // only start putting out characters if we've seen the first character or it's all 0
			firstSeen = 1;
		if(firstSeen)
		{
			rowColResponse[i] = rowNum[j];
			++i;
			++rowsSent;
		}
	}
	rowColResponse[i++] = ';';
	firstSeen = 0;
	for(j = 0; j < 4; ++j) // i is response position and keeps its previous value, j is loop position
	{
		if(colNum[j])
			firstSeen = 1;
		if(firstSeen)
		{
			rowColResponse[i] = colNum[j];
			++i;
			++colsSent;
		}
	}
	rowColResponse[i] = 'R';
	
	ps2_insert_to_buffer(rowColResponse, rowsSent+colsSent+4);
}

void CSIpushCursor()
{
	if(CSIcursorStackPointer < CURSOR_STACK_SIZE-1)
	{
		CSIcursorStackPointer++;
		cursorStack[0][0][CSIcursorStackPointer] = cursorRow;
		cursorStack[0][1][CSIcursorStackPointer] = cursorCol;
	}
}
void CSIpopCursor()
{
	remCursorAttr();
	if(CSIcursorStackPointer >= 0)
	{
		cursorRow = cursorStack[0][0][CSIcursorStackPointer];
		cursorCol = cursorStack[0][1][CSIcursorStackPointer];
		CSIcursorStackPointer--;
	}
	addCursorAttr();
}
void ESCpushCursor()
{
	if(ESCcursorStackPointer < CURSOR_STACK_SIZE-1)
	{
		ESCcursorStackPointer++;
		cursorStack[0][0][ESCcursorStackPointer] = cursorRow;
		cursorStack[0][1][ESCcursorStackPointer] = cursorCol;
	}
}
void ESCpopCursor()
{
	remCursorAttr();
	if(ESCcursorStackPointer >= 0)
	{
		cursorRow = cursorStack[0][0][ESCcursorStackPointer];
		cursorCol = cursorStack[0][1][ESCcursorStackPointer];
		ESCcursorStackPointer--;
	}
	addCursorAttr();
}

/////////////////// Screen Manipulation ///////////////////////

void bufClear(void)
{
	int i,j;
	for(i = 0; i < ROWS; ++i)
	{
		for(j = 0; j < COLS; ++j)
		{
			screenChars[i][j] = ' ';
			screenFgColor[i][j] = Grey;
			screenBgColor[i][j] = Black;
			screenDisplayOps[i][j] = 0;
		}
	}
}


void flushScreen()
{
	int i,j;
	int Io, In; // Translated i for screen top differences due to scrolling
	char letter;
	static int count = 0;
	static unsigned int refreshes = 0;
	#define REFRESH_COUNT 20
	clearExtraneousCursors();
	count = (count+1) % (REFRESH_COUNT +1);
	if (count == REFRESH_COUNT)
		refreshes++; // count number of refreshes, for blinking

	for(i = ROWS-1; i >= 0; --i) // Start from the bottom of the screen
	{
		Io = (i + oldScreenTop) % ROWS;
		In = (i + screenTop) % ROWS;
		for(j = 0; j < COLS; ++j)
		{
			if (oldScreenChars[Io][j] != screenChars[In][j] 
				|| oldScreenFgColor[Io][j] != screenFgColor[In][j] 
				|| oldScreenBgColor[Io][j] != screenBgColor[In][j]
			  || oldScreenDisplayOps[Io][j] != screenDisplayOps[In][j]
				|| count == REFRESH_COUNT) // redraw everything every so often to clear out glitches
			{
				letter = screenChars[In][j];
				if(screenDisplayOps[In][j] & BLINK && refreshes%2)
				{ // blink every other whole screen refresh cycle
					letter = ' ';
				}
				if ((screenDisplayOps[In][j] & CURSOR_CHAR) && drawCursor || screenDisplayOps[In][j] & REVERSE_VIDEO)
				{
					LCD_DrawChar_rc (i, j, screenBgColor[In][j], screenFgColor[In][j], letter, screenDisplayOps[In][j] & UNDERLINE);
				}
				else
					LCD_DrawChar_rc (i, j, screenFgColor[In][j], screenBgColor[In][j], letter, screenDisplayOps[In][j] & UNDERLINE);
			}
		}
	}

	for(i = 0; i < ROWS; ++i)
	{
		for (j = 0; j < COLS; ++j)
		{
			oldScreenChars[i][j] = screenChars[i][j];
			oldScreenFgColor[i][j] = screenFgColor[i][j];
			oldScreenBgColor[i][j] = screenBgColor[i][j];
		}
	}
	oldScreenTop = screenTop;
	

}

/////////////////// Main Handler ///////////////////////////////

void handleAscii(unsigned char *buf, int bytes)
// Outputs ascii to screen or handles escape codes
// Much of the escape code handling is in "man console_codes" in in the Linux Programmer's Manual
{
	int i,j,k, rowTr;
	static unsigned int escStat = 0; // variable to hold current escape sequence status
	#define ESC_ON 0x00000001
	#define CSI_ON 0x00000002
	#define CSI_QUESTION_ON 0x00000004
	// TODO - add flags for multi-stage escapes
	#define CSI_NUM_MAX 25
	static unsigned int csi_nums[CSI_NUM_MAX];
	static unsigned int csi_nums_rec = 0;
	static unsigned int csi_digits = 0; // to hold digits until a separator is hit
	
	keepCursorInBounds();
	for(i = 0; i < bytes; ++i)
	{
		if(escStat & CSI_ON)
		{
			// handle csi codes
			// if you get:
			// <CSI escape><number>m
			// you change colors.  You can add multiple color options like this:
			// <CSI esc><number>;<number>m
			// Other CSI sequences behave similarly, a semicolon separated list of decimal numbers
			// The function is determined by the terminating character (m for color options, others for other things)
			// Note that the decimal number can be multiple bytes long... but probably not more than 2.
			// My testing on the Linux console shows that numbers with more than 2 digits are to be simply ignored.
			
			if (buf[i] >= 48 && buf[i] <= 57)
			// numbers in ascii are 48-57 (0-9)
			{
				csi_digits *= 10; // decimal left shift
				csi_digits += buf[i] - 48; // add in the new number
			}
			else if (buf[i] == ';')
			{ // separator for another number
				if (csi_nums_rec < CSI_NUM_MAX)
				{
					csi_nums[csi_nums_rec++] = csi_digits;
				}
				csi_digits = 0;
			}
			else if (buf[i] == '?')
			{
				escStat |= CSI_QUESTION_ON;
			}
			else
			{ // Here we handle the numbers received based on the terminating character.
				// if no numbers have been received, treat it as if a 0 were received
				csi_nums[csi_nums_rec++] = csi_digits;
				csi_digits = 0;
				
				switch (buf[i]) 
				{
					case 'm': // Colors!!!
						handle_colorCodes(csi_nums, csi_nums_rec);
						break;
					case 'A': // move cursor up n rows
						for(j = 0; j < csi_nums_rec; ++j)
						{
							if(csi_nums[j] == 0)
								csi_nums[j] = 1;
							moveCursor(csi_nums[j], UP);
						}
						break;
					case 'B': // move down n rows
					case 'e': // same as B
						for(j = 0; j < csi_nums_rec; ++j)
						{
							if(csi_nums[j] == 0)
								csi_nums[j] = 1;
							moveCursor(csi_nums[j], DOWN);
						}
						break;
					case 'C': // move right n rows
					case 'a': // same as C
						for(j = 0; j < csi_nums_rec; ++j)
						{
							if(csi_nums[j] == 0)
								csi_nums[j] = 1;
							moveCursor(csi_nums[j], RIGHT);
						}
						break;
					case 'D': // move left n rows
						for(j = 0; j < csi_nums_rec; ++j)
						{
							if(csi_nums[j] == 0)
								csi_nums[j] = 1;
							moveCursor(csi_nums[j], LEFT);
						}
						break;
					case 'E': // move cursor down n rows, to column 1
						for(j = 0; j < csi_nums_rec; ++j)
						{
							if(csi_nums[j] == 0)
								csi_nums[j] = 1;
							moveCursor(csi_nums[j], DOWN);
						}
						cursorCol = 0;
						break;
					case 'F': // move cursor up n rows, to column 1
						for(j = 0; j < csi_nums_rec; ++j)
						{
							if(csi_nums[j] == 0)
								csi_nums[j] = 1;
							moveCursor(csi_nums[j], UP);
						}
						cursorCol = 0;
						break;
					case 'G': // move cursor to indicated column, current row
						remCursorAttr();
						cursorCol = csi_nums[csi_nums_rec-1] - 1;
						keepCursorInBounds();
						addCursorAttr();
						break;
					case 'H':  // move cursor to indicated row,column
					case 'f': // same as H
						remCursorAttr();
						if (csi_nums_rec <2)
							cursorCol = 0;
						else
							cursorCol = csi_nums[1] -1;
						cursorRow = csi_nums[0] -1;
					
						keepCursorInBounds();
						addCursorAttr();
						break;
					case 'd': // move to indicated row, current column
						remCursorAttr();
						cursorRow = csi_nums[csi_nums_rec-1] -1;
						if (cursorRow < 0)
							cursorRow = 0;
						else if (cursorRow > ROWS - 1)
							cursorRow = ROWS-1;
						addCursorAttr();
						break;
					case 'J':
						// Erase display
						if (csi_nums[csi_nums_rec-1] == 1)
						{ // erase from start to cursor
							for(j = 0; j < cursorRow; ++j)
							{
								rowTr = (j + screenTop) % ROWS;
								for(k = 0; k < COLS; ++k)
								{
									screenChars[rowTr][k] = ' ';
								}
							}
							for(j = 0; j < cursorCol; ++j)
							{
								screenChars[rowTr+1][j] = ' ';
							}
						}
						if (csi_nums[csi_nums_rec-1] == 0)
						{ // erase from cursor to end
							for(j = cursorRow; j < ROWS; ++j)
							{
								for(k = 0; k < COLS; ++k)
								{
									rowTr = (j + screenTop) % ROWS;
									screenChars[rowTr][k] = ' ';
								}
							}
							rowTr = (cursorRow + screenTop) % ROWS;
							for(j = 0; j < cursorCol; ++j)
							{
								screenChars[cursorRow][j] = ' ';
							}
						}
						else if(csi_nums[csi_nums_rec-1] == 2 || csi_nums[csi_nums_rec-1] == 3)
						{ // erase whole display
							for(j = 0; j < ROWS; ++j)
							{
								for (k = 0; k < COLS; ++k)
								{
									screenChars[j][k] = ' ';
								}
							}
						}
						break;
					case 'K':
						// Erase line
						rowTr = (cursorRow + screenTop) % ROWS;
						if (csi_nums[csi_nums_rec-1] == 1)
						{ // erase from start to cursor
							for(j = 0; j < cursorCol; ++j)
							{
								screenChars[rowTr][j] = ' ';
							}
						}
						if (csi_nums[csi_nums_rec-1] == 0)
						{ // erase from cursor to end
							for(j = cursorCol; j < COLS; ++j)
							{
								screenChars[rowTr][j] = ' ';
							}
						}
						else if(csi_nums[csi_nums_rec-1] == 2 || csi_nums[csi_nums_rec-1] == 3)
						{ // erase whole line
							for(j = 0; j < COLS; ++j)
							{
								screenChars[rowTr][j] = ' ';
							}
						}
						break;
					case 's': // push cursor position
						CSIpushCursor();
						break;
					case 'u': // pop cursor position
						CSIpopCursor();
						break;
					case 'n': // if we get 6, it reports cursor position
						if (csi_nums[0] == 6)
						{
							reportCursorToHost();
						}
						break;
					case 'l': // sometimes hides the cursor (ESC[?25l)
						if (csi_nums[0] == 25 && (escStat & CSI_QUESTION_ON))
						{
							drawCursor = 0;
						}
						break;
					case 'h': // sometimes shows the cursor (ESC[?25h)
						if (csi_nums[0] == 25 && (escStat & CSI_QUESTION_ON))
						{
							drawCursor = 1;
						}
						break;
					default:
						break;
				}
				escStat &= ~(CSI_ON | CSI_QUESTION_ON);
				csi_nums_rec = 0;
			}
		} //////////////// END HANDLING CSI CODES
		else if(escStat & ESC_ON)
		{
			// handle escape codes
/*			
			The following is an exerpt from the "console_codes" man page.
			We're not implementing all of them (Operating system command?  Obviously that's Linux only.)
			
			
			ESC- but not CSI-sequences

       ESC c     RIS      Reset.
       ESC D     IND      Linefeed.
       ESC E     NEL      Newline.
       ESC H     HTS      Set tab stop at current column.
       ESC M     RI       Reverse linefeed.
       ESC Z     DECID    DEC private identification. The kernel returns the
                          string  ESC [ ? 6 c, claiming that it is a VT102.
       ESC 7     DECSC    Save   current    state    (cursor    coordinates,
                          attributes, character sets pointed at by G0, G1).
       ESC 8     DECRC    Restore state most recently saved by ESC 7.
       ESC [     CSI      Control sequence introducer
       ESC %              Start sequence selecting character set
       ESC % @               Select default (ISO 646 / ISO 8859-1)
       ESC % G               Select UTF-8
       ESC % 8               Select UTF-8 (obsolete)
       ESC # 8   DECALN   DEC screen alignment test - fill screen with E's.
       ESC (              Start sequence defining G0 character set
       ESC ( B               Select default (ISO 8859-1 mapping)
       ESC ( 0               Select VT100 graphics mapping
       ESC ( U               Select null mapping - straight to character ROM
       ESC ( K               Select user mapping - the map that is loaded by
                             the utility mapscrn(8).
       ESC )              Start sequence defining G1
                          (followed by one of B, 0, U, K, as above).
       ESC >     DECPNM   Set numeric keypad mode
       ESC =     DECPAM   Set application keypad mode
       ESC ]     OSC      (Should  be:  Operating  system  command)  ESC ] P
                          nrrggbb: set palette, with parameter  given  in  7
                          hexadecimal  digits after the final P :-(.  Here n
                          is the color  (0-15),  and  rrggbb  indicates  the
                          red/green/blue  values  (0-255).   ESC  ] R: reset
                          palette
*/
			switch(buf[i])
			{
				case '[':
					escStat |= CSI_ON;
					break;
				case 'c':
					// reset
					bufClear();
					break;
				case '7':
					ESCpushCursor();
					break;
				case '8':
					ESCpopCursor();
					break;
				case 'D':
				case 'E':
					handle_LF();
					break;
				case 'M':
					// reverse line feed...
					moveCursor(1,UP);
					break;
				case 'H':
					// set tab stop at current column...
					break;
				case 'Z':
					// report DECID
					break;
				default:
					break;
			}
			
			escStat &= ~ESC_ON;
		}
		else
		{
			switch(buf[i])
			{
				// Handle control characters
				//00 (NUL), 07 (BEL), 08 (BS), 09 (HT), 0a (LF), 0b (VT), 0c (FF),
				//   0d (CR), 0e (SO), 0f (SI), 18 (CAN), 1a (SUB), 1b (ESC), 7f (DEL)
				//  0x9B (CSI)
				case 0x00: // NUL
					break; // it's ignored.
				case 0x07: // BEL
					DAC_beep();
					break;
				case 0x08: // BS
					// This may be tricky... we may need so implement some sort of line discipline stuff to know how many characters the user's input since the last <return>
					// It works properly if user input doesn't make it go on to a second line... the same problem that plagues many
					// X terminal emulators...
					if (cursorCol > 0)
						--cursorCol;
					keepCursorInBounds();
					break;
				case 0x09: // HT - goes to the next tab stop or to the end of the line if there is no earlier tab stop
					break;
				case 0x0A: // LF - down a line.
				case 0x0B: // VT, same as LF
				case 0x0C: // FF, same as VT and LF
					handle_LF();
					break;
				case 0x0D: // CR - push the carriage to the left! On old typewriters.
					handle_CR();
					break;
				case 0x0E: // SO - activate G1 character set
					break;
				case 0x0F: // SI - activate G0 character set
					break;
				case 0x18: // CAN - interrupt escape sequence (not sure what it does)
					break;
				case 0x1A: // SUB - interrupt escape sequence (not sure what it does)
					break;
				case 0x1B: // ESC - start escape sequence
					escStat |= ESC_ON;
					break;
				case 0x7F: // DEL - ignored
					break;
				case 0x9B: // CSI - start CSI sequence
					escStat |= CSI_ON;
					break;
				// Normal character handling!
				default:
					handle_normal(buf[i]);
					break;
			}
		}
	}
}

