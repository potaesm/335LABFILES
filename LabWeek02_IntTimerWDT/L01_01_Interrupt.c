// Nu_LB-002: L01_01_Interrupt.c
// SW GPB_15

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.              */
/* edited: june 2014: dejwoot.kha@mail.kmutt.ac.th                          */
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include "NUC1xx.h"
#include "LCD_Driver.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvSYS.h"

#define DELAY300ms	300000 		// The maximal delay time is 335000 us.

void EINT1Callback(void) {
	clr_all_pannal();
	print_lcd(0, "Int1 !!!!");
	DrvSYS_Delay(DELAY300ms);		// delay
	}

void Delay(uint32_t counter) {
	while(counter--);
	}

int main (void) {
	Initial_pannel();  //call initial pannel function
	clr_all_pannal();	
  
	/* Configure general GPIO interrupt */
	DrvGPIO_Open(E_GPB, 15, E_IO_INPUT);

	/* Configure external interrupt */
	DrvGPIO_EnableEINT1(E_IO_BOTH_EDGE, E_MODE_EDGE, EINT1Callback);

	/* Waiting for interrupts */
	while(1) {
		print_lcd(0, "Deep Sleep");
		DrvSYS_Delay(DELAY300ms);	   // delay

		UNLOCKREG();						  
		SCB->SCR = 4;			// System Control Register (5.2.8: Tech.Ref.)
		SYSCLK->PWRCON.PD_WU_INT_EN = 0;	
		SYSCLK->PWRCON.PD_WAIT_CPU = 1;
		SYSCLK->PWRCON.PWR_DOWN_EN = 1;
		LOCKREG();
		__WFI();

		// check if SW_INT is pressed         
		while((GPIOB->PIN &= 0x8000) == 0) {
			GPIOA->DOUT &= 0x8FFF;	// turn on only RGB_LED GPA_12,13, and 14
			Delay(DELAY300ms);
			GPIOA->DOUT |= 0x7000;	// turn off only RGB_LED GPA_12,13, and 14
			Delay(DELAY300ms);
			}
		}
	}
