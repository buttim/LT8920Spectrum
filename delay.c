//!sdcc -c delay.c
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "N76E003.h"
#include "delay.h"


void Timer3_Delay100ms(uint32_t u32CNT) {
    T3CON = 0x07;                           		//Timer3 Clock = Fsys/128
    set_TR3;                                		//Trigger Timer3
    while (u32CNT != 0)  {
        RL3 = LOBYTE(TIMER_DIV128_VALUE_100ms); //Find  define in "Function_define.h" "TIMER VALUE"
        RH3 = HIBYTE(TIMER_DIV128_VALUE_100ms);
        while ((T3CON&SET_BIT4) != SET_BIT4);		//Check Timer3 Time-Out Flag
        clr_TF3;
        u32CNT --;
    }
    clr_TR3;                                		//Stop Timer3
}

void Timer2_Delay500us(uint32_t u32CNT) {
    clr_T2DIV2;																	//Timer2 Clock = Fsys/4 
    clr_T2DIV1;
    set_T2DIV0;
    set_TR2;                                		//Start Timer2
    while (u32CNT != 0)
    {
        TL2 = LOBYTE(TIMER_DIV4_VALUE_500us);		//Find  define in "Function_define.h" "TIMER VALUE"
        TH2 = HIBYTE(TIMER_DIV4_VALUE_500us);
        while (TF2 != 1);                   		//Check Timer2 Time-Out Flag
        clr_TF2;
        u32CNT --;
    }
    clr_TR2;                                		//Stop Timer2
}

void Timer3_Delay10us(uint32_t u32CNT) {
    T3CON = 0x07;                           		//Timer3 Clock = Fsys/128
    set_TR3;                                		//Trigger Timer3
    while (u32CNT != 0)
    {
	RL3 = LOBYTE(TIMER_DIV4_VALUE_10us); //Find  define in "Function_define.h" "TIMER VALUE"
        RH3 = HIBYTE(TIMER_DIV4_VALUE_10us);
        while ((T3CON&SET_BIT4) != SET_BIT4);		//Check Timer3 Time-Out Flag
        clr_TF3;
        u32CNT --;
    }
    clr_TR3;                                		//Stop Timer3
}