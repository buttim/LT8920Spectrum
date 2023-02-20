#ifndef __TIMER_H
#define __TIMER_H

//16MHz 
#define TIMER_DIV4_VALUE_10us				65536-40		//40*4/16000000 = 10 uS,    		// Timer divider = 4	for TM2/TM3
#define TIMER_DIV4_VALUE_500us			65536-2000	//2000*4/16000000 = 500 us			// Timer divider = 4
#define	TIMER_DIV128_VALUE_100ms		65536-12500	//12500*128/16000000 = 100 ms		// Timer divider = 128

void Timer3_Delay100ms(uint32_t u32CNT);
void Timer2_Delay500us(uint32_t u32CNT);
void Timer3_Delay10us(uint32_t u32CNT);
#endif