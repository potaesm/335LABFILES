// Nu_LB-002: L00_05_LCD_text.c
//

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.              */
/* edited: june 2014: dejwoot.kha@mail.kmutt.ac.th                          */
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include "NUC1xx.h"
#include "LCD_Driver.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvUART.h"
#include "Driver\DrvSYS.h"

#define DELAY300ms	300000 // The maximal delay time is 335000 us.

int main (void)	{
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
		DrvSYS_Delay(DELAY300ms);	   // delay
		lcdDemoCounter++;
		}
	}
