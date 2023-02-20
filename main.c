//! make
//command line tool non funziona
//! NuLink_8051OT -w APROM "build\%name%.bin" offset 0
#include "N76E003.h"
#include "common.h"
#include "delay.h"
#include "oled.h"
#include "radio.h"
#include "spi.h"

#define N_CHANNELS 100

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

uint8_t __xdata data[N_CHANNELS];

int putchar(int c) {
  Send_Data_To_UART0(c);
  return c;
}

void barGraph(uint8_t const data[], size_t size) {
  for (int i = 0; i < size; i++)
    for (int j = 0; j < 8; j++) {
      OLED_Set_Pos(i, 7 - j);
      OLED_WR_Byte(0xFF << (8 - (max(0, min(8, data[i] - 8 * j)))), OLED_DATA);
    }
}

unsigned char _sdcc_external_startup(void) {
	//disables Power On Reset
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
  int n = 0;
  TIMER1_MODE0_ENABLE;
  Set_All_GPIO_Quasi_Mode;
  InitialUART0_Timer1(115200);

  P04 = 0; // LT8920 reset
  Timer3_Delay100ms(1);
  P04 = 1;

  SPI_Initial();
  Timer3_Delay100ms(1);

  LT8920Begin();
  LT8920SetChannel(50);

  OLED_Init();
  OLED_Clear();
  LT8920BeginScanRSSI(0, N_CHANNELS);

  puts("VIA");

  while (true) {
    LT8920EndScanRSSI(data, N_CHANNELS);
    LT8920BeginScanRSSI(0, N_CHANNELS);
    uint8_t max[] = {0,0,0,0};
    for (int i = 0; i < N_CHANNELS; i++) {
      if (data[i] >= max[0])
        max[0] = data[i];
      else if (data[i] >= max[1])
        max[1] = data[i];
      else if (data[i] >= max[2])
        max[2] = data[i];
      else if (data[i] >= max[3])
        max[3] = data[i];
    }
    for (int i = 0; i < 4; i++)
      if (max[i]>0)
	OLED_ShowNum(110, 2 * i, max[i], 2, 16);
      else
	OLED_ShowString(110,2*i,"  ",16);
    barGraph(data, N_CHANNELS);
    putchar('a' + n++);
    if (n == 26) {
      n = 0;
      putchar('\n');
    }
  }
}