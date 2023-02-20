//!sdcc -c  spi.c
#include "N76E003.h"
#include "spi.h"

void SPI_Initial(void) {      
	P12_Quasi_Mode;														// P12 (SS) Quasi mode
	P10_Quasi_Mode;														// P10(SPCLK) Quasi mode
	P00_Quasi_Mode;														// P00 (MOSI) Quasi mode
	P01_Quasi_Mode;														// P01 (MISO) Quasi mode

	set_DISMODF;                                // SS General purpose I/O ( No Mode Fault ) 
	clr_SSOE;

	clr_LSBFE;                                  // MSB first         

	clr_CPOL;                                   // The SPI clock is low in idle mode
	set_CPHA;                                   // The data is sample on the second edge of SPI clock 

	set_MSTR;                                   // SPI in Master mode 
	SPICLK_DIV16;                        				// Select SPI clock 
	set_SPIEN;                                  // Enable SPI function 
	clr_SPIF;
}