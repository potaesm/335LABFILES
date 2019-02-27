// Nu_LB-002: L00_01_LED_GPC_12_15.c
// use function, macro and directly at Register to turn 'ON' & 'OFF' LED

// 
// Smpl_GPIO_LED : GPC12 ~ 15 to control on-board LEDs
//                 output low to enable red LEDs
//
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.              */
/* edited: june 2014: dejwoot.kha@mail.kmutt.ac.th                          */
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include "NUC1xx.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvSYS.h"

#define DELAY300ms	300000 // The maximal delay time is 335000 us.

// Initial GPIO pins (GPC 12,13,14,15) to Output mode  
void Init_LED() {
	// initialize GPIO pins
	DrvGPIO_Open(E_GPC, 12, E_IO_OUTPUT); // GPC12 pin set to output mode
	DrvGPIO_Open(E_GPC, 13, E_IO_OUTPUT); // GPC13 pin set to output mode
	DrvGPIO_Open(E_GPC, 14, E_IO_OUTPUT); // GPC14 pin set to output mode
	DrvGPIO_Open(E_GPC, 15, E_IO_OUTPUT); // GPC15 pin set to output mode
	// set GPIO pins to output Low
	DrvGPIO_SetBit(E_GPC, 12); // GPC12 pin output Hi to turn off LED
	DrvGPIO_SetBit(E_GPC, 13); // GPC13 pin output Hi to turn off LED
	DrvGPIO_SetBit(E_GPC, 14); // GPC14 pin output Hi to turn off LED
	DrvGPIO_SetBit(E_GPC, 15); // GPC15 pin output Hi to turn off LED
	}

int main (void) {

  Init_LED();	// Initialize LEDs (four on-board LEDs below LCD panel)

	while (1) { // forever loop to keep flashing four LEDs one at a time
		DrvGPIO_ClrBit(E_GPC, 12); 		// Function -> output Low to turn on LED
		DrvSYS_Delay(DELAY300ms);	   	// delay (The maximal delay time is 335000 us.)
		DrvGPIO_SetBit(E_GPC, 12); 		// output Hi to turn off LED
		DrvSYS_Delay(DELAY300ms);	   	// delay

		GPC_13 = 0;										// Macro -> Turn 'On' LED
		DrvSYS_Delay(DELAY300ms);	   	// delay
		GPC_13 = 1;										// Macro -> Turn 'OFF' LED
		DrvSYS_Delay(DELAY300ms);	   	// delay

		_DRVGPIO_DOUT (E_GPC, 14) = 0;	// Macro -> Turn 'On' LED
		DrvSYS_Delay(DELAY300ms);	   		// delay 
		_DRVGPIO_DOUT (E_GPC, 14) = 1;	// Macro -> Turn 'OFF' LED
		DrvSYS_Delay(DELAY300ms);	   		// delay

		//DrvGPIO_ClrBit(E_GPC, 15);		// output Low to turn on LED
		GPIOC->DOUT &= 0x7FFF;				// turn on only LED GPC 15
		DrvSYS_Delay(DELAY300ms);	  	// delay 
		//DrvGPIO_SetBit(E_GPC, 15);		// output Hi to turn off LED
		GPIOC->DOUT |= 0x8000;				// turn off only LED GPC 15
		DrvSYS_Delay(DELAY300ms);	  	// delay
		}
	}
