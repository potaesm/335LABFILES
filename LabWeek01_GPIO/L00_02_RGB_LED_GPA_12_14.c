// Nu_LB-002: L00_02_RGB_LED_GPA_12_14.c
//
// RGB-LED GPA_12to14

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.              */
/* edited: june 2014: dejwoot.kha@mail.kmutt.ac.th                          */
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include "NUC1xx.h"
#include "Driver\DrvSYS.h"
#include "Driver\DrvGPIO.h"

#define DELAY300ms	300000 // The maximal delay time is 335000 us.

// Initial GPIO pins (GPA 12,13,14) to Output mode  
void Init_RGB_LED() {
	// initialize GPIO pins
	DrvGPIO_Open(E_GPA, 12, E_IO_OUTPUT); // GPA12 pin set to output mode
	DrvGPIO_Open(E_GPA, 13, E_IO_OUTPUT); // GPA13 pin set to output mode
	DrvGPIO_Open(E_GPA, 14, E_IO_OUTPUT); // GPA14 pin set to output mode
	// set GPIO pins output Hi to disable LEDs
	DrvGPIO_SetBit(E_GPA, 12); // GPA12 pin output Hi to turn off Blue  LED
	DrvGPIO_SetBit(E_GPA, 13); // GPA13 pin output Hi to turn off Green LED
	DrvGPIO_SetBit(E_GPA, 14); // GPA14 pin output Hi to turn off Red   LED
} 

int main(void) {
	Init_RGB_LED();		

	// GPA12 = Blue,  0 : on, 1 : off
	// GPA13 = Green, 0 : on, 1 : off
	// GPA14 = Red,   0 : on, 1 : off	
	while(1) {
		// set RGBled to Blue
    DrvGPIO_ClrBit(E_GPA,12); // GPA12 = Blue,  0 : on, 1 : off
    DrvGPIO_SetBit(E_GPA,13); 
    DrvGPIO_SetBit(E_GPA,14); 
		DrvSYS_Delay(DELAY300ms);	   	// delay
		
		// set RGBled to Green
    DrvGPIO_SetBit(E_GPA,12); 
    DrvGPIO_ClrBit(E_GPA,13); // GPA13 = Green, 0 : on, 1 : off
    DrvGPIO_SetBit(E_GPA,14); 
		DrvSYS_Delay(DELAY300ms);	   	// delay
		
		// set RGBled to Red
    DrvGPIO_SetBit(E_GPA,12); 
    DrvGPIO_SetBit(E_GPA,13); 
    DrvGPIO_ClrBit(E_GPA,14); // GPA14 = Red,   0 : on, 1 : off
		DrvSYS_Delay(DELAY300ms);	   	// delay
		
		// set RGBled to off	 
    DrvGPIO_SetBit(E_GPA,12); // GPA12 = Blue,  0 : on, 1 : off
    DrvGPIO_SetBit(E_GPA,13); // GPA13 = Green, 0 : on, 1 : off
    DrvGPIO_SetBit(E_GPA,14); // GPA14 = Red,   0 : on, 1 : off		
		DrvSYS_Delay(DELAY300ms);	   	// delay
		}	  		
	}
