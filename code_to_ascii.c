//code_to_ascii.c

#include "code_to_ascii.h"

unsigned char map[0x100];
unsigned char shift_map[0x100];
unsigned char ctl_map[0x100];

void scan_code_init(void)
{
	int i = 0;
	for (i=0; i<0x100; i++)
	{
		map[i] = 0;
		shift_map[i] = 0;
		ctl_map[i] = 0;
	}
	
	map[0x1C]='a';			
	map[0x32]='b';			
	map[0x21]='c';			
	map[0x23]='d';			
	map[0x24]='e';			
	map[0x2B]='f';			
	map[0x34]='g';			
	map[0x33]='h';			
	map[0x43]='i';			
	map[0x3B]='j';			
	map[0x42]='k';			
	map[0x4B]='l';			
	map[0x3A]='m';			
	map[0x31]='n';			
	map[0x44]='o';			
	map[0x4D]='p';			
	map[0x15]='q';						
	map[0x2D]='r';						
	map[0x1B]='s';							
	map[0x2C]='t';							
	map[0x3C]='u';									
	map[0x2A]='v';									
	map[0x1D]='w';									
	map[0x22]='x';								
	map[0x35]='y';									
	map[0x1A]='z';								
	map[0x45]='0';								
	map[0x16]='1';							
	map[0x1E]='2';								
	map[0x26]='3';									
	map[0x25]='4';								
	map[0x2E]='5';									
	map[0x36]='6';						
	map[0x3D]='7';						
	map[0x3E]='8';								
	map[0x46]='9';								
									
	map[0x54]='[';			
	map[0x0E]='`';
	map[0x4E]='-';
	map[0x55]='=';
	map[0x5D]='\\';
	map[0x52]='\'';
	map[0x49]='.';
	map[0x41]=',';
	map[0x4C]=';';
	map[0x5B]=']';
	map[0x4A]='/';
	
	map[0x29]=0x20;//SPACE
	map[0x5A]=0x0D;//ENTER
	map[0x76]=0x1B;//ESC
	map[0x66]=0x08;//BKSP
	map[0x0D]=0x09;//TAB
	
	map[0x7C]='*';//'KP *';
	map[0x79]='+';//'KP +';
	map[0x7B]='-';//'KP -';
	map[0x71]='.';//'KP .';
	map[0x70]='0';//'KP 0';
	map[0x69]='1';//'KP 1';
	map[0x72]='2';//'KP 2';
	map[0x7A]='3';//'KP 3';
	map[0x6B]='4';//'KP 4';
	map[0x73]='5';//'KP 5';
	map[0x74]='6';//'KP 6';
	map[0x6C]='7';//'KP 7';
	map[0x75]='8';//'KP 8';
	map[0x7D]='9';//'KP 9';


	shift_map[0x1C]='A';			
	shift_map[0x32]='B';			
	shift_map[0x21]='C';			
	shift_map[0x23]='D';			
	shift_map[0x24]='E';			
	shift_map[0x2B]='F';			
	shift_map[0x34]='G';			
	shift_map[0x33]='H';			
	shift_map[0x43]='I';			
	shift_map[0x3B]='J';			
	shift_map[0x42]='K';			
	shift_map[0x4B]='L';			
	shift_map[0x3A]='M';			
	shift_map[0x31]='N';			
	shift_map[0x44]='O';			
	shift_map[0x4D]='P';			
	shift_map[0x15]='Q';						
	shift_map[0x2D]='R';						
	shift_map[0x1B]='S';							
	shift_map[0x2C]='T';							
	shift_map[0x3C]='U';									
	shift_map[0x2A]='V';									
	shift_map[0x1D]='W';									
	shift_map[0x22]='X';								
	shift_map[0x35]='Y';									
	shift_map[0x1A]='Z';								
	shift_map[0x45]=')';								
	shift_map[0x16]='!';							
	shift_map[0x1E]='@';								
	shift_map[0x26]='#';									
	shift_map[0x25]='$';								
	shift_map[0x2E]='%';									
	shift_map[0x36]='^';						
	shift_map[0x3D]='&';						
	shift_map[0x3E]='*';								
	shift_map[0x46]='(';								
									
	shift_map[0x54]='{';			
	shift_map[0x0E]='~';
	shift_map[0x4E]='_';
	shift_map[0x55]='+';
	shift_map[0x5D]='|';
	shift_map[0x52]='"';
	shift_map[0x49]='>';
	shift_map[0x41]='<';
	shift_map[0x4C]=':';
	shift_map[0x5B]='}';
	shift_map[0x4A]='?';
	
	shift_map[0x29]=0x20;//SPACE
	shift_map[0x5A]='\n';//ENTER - set to newline for debugging purposes
	shift_map[0x76]=0x1B;//ESC
	shift_map[0x66]=0x08;//BKSP
	shift_map[0x0D]=0x09;//TAB

	shift_map[0x7C]='*';//'KP *';
	shift_map[0x79]='+';//'KP +';
	shift_map[0x7B]='-';//'KP -';
	shift_map[0x71]='.';//'KP .';
	shift_map[0x70]='0';//'KP 0';
	shift_map[0x69]='1';//'KP 1';
	shift_map[0x72]='2';//'KP 2';
	shift_map[0x7A]='3';//'KP 3';
	shift_map[0x6B]='4';//'KP 4';
	shift_map[0x73]='5';//'KP 5';
	shift_map[0x74]='6';//'KP 6';
	shift_map[0x6C]='7';//'KP 7';
	shift_map[0x75]='8';//'KP 8';
	shift_map[0x7D]='9';//'KP 9';
	
	ctl_map[0x1e] = 0x00; //^@
	ctl_map[0x36] = 0x1e; //^^
	ctl_map[0x3e] = 0x7f; //^?
	ctl_map[0x4e] = 0x1f; //^_
	ctl_map[0x15] = 0x11; //^Q
	ctl_map[0x1d] = 0x17; //^W
	ctl_map[0x24] = 0x05; //^E
	ctl_map[0x2d] = 0x12; //^R
	ctl_map[0x2c] = 0x14; //^T
	ctl_map[0x35] = 0x19; //^Y
	ctl_map[0x3c] = 0x15; //^U
	ctl_map[0x44] = 0x0f; //^O
	ctl_map[0x4d] = 0x10; //^P
	ctl_map[0x54] = 0x1b; //^[
	ctl_map[0x5b] = 0x1d; //^]
	ctl_map[0x5d] = 0x1c; //^\\ ...
	ctl_map[0x1c] = 0x01; //^A
	ctl_map[0x1b] = 0x13; //^S
	ctl_map[0x2b] = 0x06; //^F
	ctl_map[0x34] = 0x07; //^G
	ctl_map[0x33] = 0x08; //^H
	ctl_map[0x3b] = 0x0a; //^J
	ctl_map[0x42] = 0x0b; //^K
	ctl_map[0x4b] = 0x0c; //^L
	ctl_map[0x1a] = 0x1a; //^Z
	ctl_map[0x22] = 0x18; //^X
	ctl_map[0x21] = 0x03; //^C
	ctl_map[0x2a] = 0x16; //^V
	ctl_map[0x32] = 0x02; //^B
	ctl_map[0x31] = 0x0e; //^N
	ctl_map[0x3a] = 0x0d; //^M
	ctl_map[0x23] = 0x04; //^D
	ctl_map[0x43] = 0x09; //^I

}
