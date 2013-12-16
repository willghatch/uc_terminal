//ps2_over_gpioc.h
#ifndef PS2_OVER_GPIOC
#define PS2_OVER_GPIOC

#define PS2_DATA_SIZE 1000



void ps2_over_gpioc_init(void);
//void ps2_dump_data_over_usart2(void);
int ps2_memcpy(unsigned char * dst);
void ps2_insert_to_buffer(char *insert, int size);


#endif


