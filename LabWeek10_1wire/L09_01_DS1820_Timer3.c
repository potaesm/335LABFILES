// Nu_LB-002: L09_01_DS1820_Timer3.c
/* 
	Onewire : GPE8	
*/

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* written by dejwoot.kha@mail.kmutt.ac.th (2014/3/31)                      */
/* edited: june 2014: dejwoot.kha@mail.kmutt.ac.th                          */
/*--------------------------------------------------------------------------*/

#include <stdio.h>																											 
#include "NUC1xx.h"
#include "LCD_Driver.h"
#include "DrvSYS.h"
#include "Driver\DrvGPIO.h"
#define DELAY300ms	300000 		// The maximal delay time is 335000 us.

static uint16_t Timer3Counter=0;

//------------------------------------------------OneWireReadByteTemperature
int8_t OneWireReadByteTemperature(void) {
	int8_t i;	
	int8_t dataByte = 0xCC; // skip ROM
	
	GPIOE->DOUT &= 0xFEFF;	// Master send Reset
	DrvSYS_Delay(500);
	GPIOE->DOUT |= 0x0100;
	DrvSYS_Delay(200);			// wait for presence pulse
	
	for (i=0;i<8;i++) {			// skip ROM
		if ((dataByte&0x01 == 0x01)) {
			GPIOE->DOUT &= 0xFEFF;	// send '1'
			DrvSYS_Delay(3);				// low > 1 microsec.
			GPIOE->DOUT |= 0x0100;
			DrvSYS_Delay(60);	
			} else {
			GPIOE->DOUT &= 0xFEFF;	// send '0'
			DrvSYS_Delay(60);				// low > 60 microsec.
			GPIOE->DOUT |= 0x0100;
			DrvSYS_Delay(2);				
			}
		dataByte >>= 1;
		}
	
	dataByte = 0xBE;				// ReadScratchpad
	for (i=0;i<8;i++) {
		if ((dataByte&0x01 == 0x01)) {
			GPIOE->DOUT &= 0xFEFF;	// send '1'
			DrvSYS_Delay(3);				// low > 1 microsec.
			GPIOE->DOUT |= 0x0100;
			DrvSYS_Delay(60);	
			} else {
			GPIOE->DOUT &= 0xFEFF;	// send '0'
			DrvSYS_Delay(60);				// low > 60 microsec.
			GPIOE->DOUT |= 0x0100;
			DrvSYS_Delay(2);				
			}
		dataByte >>= 1;
		}

	// read 8 bits (byte0 scratchpad)
	DrvSYS_Delay(100);
	for (i=0;i<8;i++) {
		GPIOE->DOUT &= 0xFEFF;	// 
		DrvSYS_Delay(2);				// low > 1 microsec.
		GPIOE->DOUT |= 0x0100;
		// Read
		DrvSYS_Delay(12);
		if ((GPIOE->PIN &= 0x0100) == 0x0100) {
			dataByte >>= 1;
			dataByte |= 0x80;
		} else {
			dataByte >>= 1;
			dataByte &= 0x7F;			
		}
		DrvSYS_Delay(60);			
		}
	dataByte >>= 1;
	return dataByte;
	}
//---------------------------------------------------OneWireTxSkipROMConvert
void OneWireTxSkipROMConvert(void) {
	int8_t i;	
	uint8_t dataByte = 0xCC; // skip ROM
	
	GPIOE->DOUT &= 0xFEFF;	// Master send Reset
	DrvSYS_Delay(500);
	GPIOE->DOUT |= 0x0100;
	DrvSYS_Delay(200);		
	
	for (i=0;i<8;i++) {		// skip ROM
		if ((dataByte&0x01 == 0x01)) {
			GPIOE->DOUT &= 0xFEFF;	// send '1'
			DrvSYS_Delay(3);				// low > 1 microsec.
			GPIOE->DOUT |= 0x0100;
			DrvSYS_Delay(60);	
			} else {
			GPIOE->DOUT &= 0xFEFF;	// send '0'
			DrvSYS_Delay(60);				// low > 60 microsec.
			GPIOE->DOUT |= 0x0100;
			DrvSYS_Delay(2);				
			}
		dataByte >>= 1;
	}
	
	dataByte = 0x44;	// convert Temperature
	for (i=0;i<8;i++) {
		if ((dataByte&0x01 == 0x01)) {
			GPIOE->DOUT &= 0xFEFF;	// send '1'
			DrvSYS_Delay(3);				// low > 1 microsec.
			GPIOE->DOUT |= 0x0100;
			DrvSYS_Delay(60);	
			} else {
			GPIOE->DOUT &= 0xFEFF;	// send '0'
			DrvSYS_Delay(60);				// low > 60 microsec.
			GPIOE->DOUT |= 0x0100;
			DrvSYS_Delay(2);				
			}
		dataByte >>= 1;
	}	
}
//--------------------------------------------------------------------Timer3
void InitTIMER3(void)
{
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR3_S = 0;	// Select 12Mhz for Timer0 clock source
	// 0 = 12 MHz, 1 = 32 kHz, 2 = HCLK, 7 = 22.1184 MHz
  SYSCLK->APBCLK.TMR3_EN = 1;	// Enable Timer0 clock source

	/* Step 2. Select Operation mode */	
	TIMER3->TCSR.MODE = 1;				// 1 -> Select periodic mode
	// 0 = One shot, 1 = Periodic, 2 = Toggle, 3 = continuous counting mode
	
	/* Step 3. Select Time out period 
	= (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER3->TCSR.PRESCALE = 11;	// Set Prescale [0~255]
	TIMER3->TCMPR = 1000000;		// Set TCMPR [0~16777215]
	// (1/12000000)*(11+1)*(1000000)= 1 sec

	/* Step 4. Enable interrupt */
	TIMER3->TCSR.IE = 1;
	TIMER3->TISR.TIF = 1;				// Write 1 to clear the interrupt flag 		
	NVIC_EnableIRQ(TMR3_IRQn);	// Enable Timer0 Interrupt

	/* Step 5. Enable Timer module */
	TIMER3->TCSR.CRST = 1;			// Reset up counter
	TIMER3->TCSR.CEN = 1;				// Enable Timer0
}
//----------------------------------------------------------------Timer3_IRQ
void TMR3_IRQHandler(void) 		// Timer0 interrupt subroutine 
{
	int8_t ds1820Temp;
	//uint16_t Timer3Counter = 0;
	char lcd2_buffer[18] = "Timer3:";
	char lcd3_buffer[18] = "T =    C";

	sprintf(lcd2_buffer+7," %d",Timer3Counter);
	print_lcd(2, lcd2_buffer);
	Timer3Counter++;

	// to initiate a temperature measurement and A-to-D conversion
	OneWireTxSkipROMConvert(); 
	DrvSYS_Delay(100);
	ds1820Temp = OneWireReadByteTemperature();
	sprintf(lcd3_buffer+4,"%d C",ds1820Temp);
	print_lcd(3, lcd3_buffer);	
	
 	TIMER3->TISR.TIF = 1;				// Write 1 to clear the interrupt flag 
}

//----------------------------------------------------------------------MAIN
int32_t main (void) {
	
	UNLOCKREG();
	DrvSYS_Open(48000000);
	LOCKREG();

	Initial_pannel();  //call initial pannel function
	clr_all_pannal();
	print_lcd(0, "DS1820 Onewire");
	
	DrvGPIO_Open(E_GPE, 8, E_IO_QUASI);
	InitTIMER3();
	
	while (1) {
		__NOP();
		}
	}
