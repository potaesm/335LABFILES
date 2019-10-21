// LAB335 WEEK 2 BY SUTHINAN & PONGPITH
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.              */
/* edited: JAN 2019: suthinan.musitmani@gmail.com                       	  */
/*--------------------------------------------------------------------------*/

#include <stdio.h>																											 
#include "NUC1xx.h"
#include "DrvSYS.h"
#include "Seven_Segment.h"
#include "scankey.h"
#include "Driver\DrvSYS.h"
#include "Driver\DrvGPIO.h"
#include "LCD_Driver.h"
#include "Driver\DrvUART.h"

#define DELAY300ms	300000
#define scanDelay 4000

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

void seg_display(int16_t value) {
	int8_t digit;
	digit = value / 1000;
	close_seven_segment();
	show_seven_segment(3,digit);
	DrvSYS_Delay(scanDelay);
		
	value = value - digit * 1000;
	digit = value / 100;
	close_seven_segment();
	show_seven_segment(2,digit);
	DrvSYS_Delay(scanDelay);
}

int32_t main (void) {
	
	int8_t number;
	int32_t i = 0;
	Init_RGB_LED();
	OpenKeyPad();					 	
	 
	while(1) {
	  number = Scankey();	// scan keypad to get a number (1~9)
		
		if (number == 1) {
				DrvGPIO_ClrBit(E_GPA,12); // GPA12 = Blue,  0 : on, 1 : off
				DrvGPIO_SetBit(E_GPA,13); 
				DrvGPIO_SetBit(E_GPA,14);
			}
			
		if (number == 2) {
				_DRVGPIO_DOUT (E_GPC, 14) = 0;
				_DRVGPIO_DOUT (E_GPC, 15) = 0;
			}
			
		if (number == 3) {
				GPIOA->DOUT |= 0x1000;
				GPIOC->DOUT |= 0xC000;
			}
			
		if (number == 4) {
				GPIOA->DOUT ^= 0x1000;
				DrvSYS_Delay(DELAY300ms);
			}
			
		if (number == 5) {
			GPIOC->DOUT ^= 0x1000;
			DrvSYS_Delay(DELAY300ms);
			}
			
		if (number == 6) {
			GPIOC->DOUT ^= 0x6000;
			DrvSYS_Delay(DELAY300ms);
			}
			
		if (number == 7) {
			while(1) {
				seg_display(i/10);	// display i on 7-segment display
				i = i + 100;								// increment i
				if (i == 100000) i = 0;
				if (Scankey() == 8 || Scankey() == 6 || Scankey() == 5 || Scankey() == 4 || Scankey() == 3 || Scankey() == 2 || Scankey() == 1) {
					break;
					}
				}
			}
			
		if (number == 8) {
			char lcd3_buffer[18];
			uint32_t lcdDemoCounter = 0;
			Initial_pannel();  //call initial pannel function
			clr_all_pannal();

			print_lcd(0, "LCD demo");
			print_lcd(1, "12345678901234567890");
			sprintf(lcd3_buffer,"count up = ");
	
			while (1) {
				sprintf(lcd3_buffer+11,"%d",lcdDemoCounter);
				print_lcd(2, lcd3_buffer);
				DrvSYS_Delay(DELAY300ms);
				lcdDemoCounter++;
				if (lcdDemoCounter == 100) {
					break;
					}
				if (Scankey() == 7 || Scankey() == 6 || Scankey() == 5 || Scankey() == 4 || Scankey() == 3 || Scankey() == 2 || Scankey() == 1) {
					break;
					}
				}
			}
			
		show_seven_segment(0,number); // display number on 7-segment LEDs
		DrvSYS_Delay(5000);           // delay time for keeping 7-segment display 
		close_seven_segment();	      // turn off 7-segment LEDs								 
		}
	}
