//!make
//
//!sdcc -c "%file%"
//!sdcc -c oled.c
//!sdcc -c delay.c
//!sdcc -c radio.c
//!sdcc -c spi.c
//!sdcc -c common.c
//!sdcc main.rel oled.rel radio.rel delay.rel spi.rel common.rel
//
//!NuLink_8051OT -w CFG0 0xFFFFFFFBFB
//!NuLink_8051OT -w APROM "%name%.ihx"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "N76E003.h"
#include "radio.h"
#include "spi.h"
#include "oled.h"
#include "delay.h"
#include "common.h"

#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))

int putchar(int c) {
	Send_Data_To_UART0(c);
	return c;
}

void barGraph(uint8_t const data[],size_t size) {
	for (int i=0;i<size;i++) {
		for (int j=0;j<8;j++) {
			OLED_Set_Pos(i,7-j);
			OLED_WR_Byte(0xFF<<(8-(max(0,min(8,data[i]-8*j)))),OLED_DATA);
		}
	}
}

#define N_CHANNELS 100
uint8_t __xdata data[N_CHANNELS];

unsigned char _sdcc_external_startup (void)  {  
    __asm  
    mov	0xC7, #0xAA  
    mov	0xC7, #0x55  
    mov	0xFD, #0x5A  
    mov	0xC7, #0xAA  
    mov	0xC7, #0x55  
    mov	0xFD, #0xA5  
    __endasm;  
    return 0;  
}  

void main() {
	TIMER1_MODE0_ENABLE;
	Set_All_GPIO_Quasi_Mode;
	InitialUART0_Timer1(115200);

	P04=0;	//LT8920 reset
	Timer3_Delay100ms(1);
	P04=1;
	
	SPI_Initial();
	Timer3_Delay100ms(1);
	
	LT8920Begin();
	LT8920SetChannel(50);
	
	OLED_Init();
	OLED_Clear();
	LT8920BeginScanRSSI(0,N_CHANNELS);
	
	puts("VIA");
	
	while (true) {
		//LT8920scanRSSI(data,0,N_CHANNELS);
		LT8920EndScanRSSI(data,N_CHANNELS);
		LT8920BeginScanRSSI(0,N_CHANNELS);
		uint8_t max[]={0,0,0,0};
		for (int i=0;i<N_CHANNELS;i++) {
			if (data[i]>=max[0]) max[0]=data[i];
			else if (data[i]>=max[1]) max[1]=data[i];
			else if (data[i]>=max[2]) max[2]=data[i];
			else if (data[i]>=max[3]) max[3]=data[i];
		}
		for (int i=0;i<4;i++)
			OLED_ShowNum(110,2*i,max[i],2,16);
		barGraph(data,N_CHANNELS);
		puts("hop!");
	}
}