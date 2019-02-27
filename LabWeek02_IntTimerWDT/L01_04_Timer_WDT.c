// Nu_LB-002: L01_04_Timer_WDT.c
// timer up every 1 second

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.              */
/* edited: june 2014: dejwoot.kha@mail.kmutt.ac.th                          */
/*--------------------------------------------------------------------------*/

#include <stdio.h>	
#include <string.h>																										 
#include "NUC1xx.h"
#include "LCD_Driver.h"

static uint16_t Timer0Counter = 0;

//-----------------------------------------------------------------------WDT
void InitWDT(void) {
	UNLOCKREG();		
	/* Step 1. Enable and Select WDT clock source */         
	SYSCLK->CLKSEL1.WDT_S = 3;	// Select 10kHz for WDT clock source  	
	SYSCLK->APBCLK.WDT_EN = 1;	// Enable WDT clock source	

	/* Step 2. Select Timeout Interval */
	WDT->WTCR.WTIS = 6;					// 2^16 * (1/10k) = 6.5536 sec.
	
	/* Step 3. Disable Watchdog Timer Reset function */
	WDT->WTCR.WTRE = 0;

	/* Step 4. Enable WDT interrupt */	
	WDT->WTCR.WTIF = 1;					// Write 1 to clear flag	
	WDT->WTCR.WTIE = 1;
	NVIC_EnableIRQ(WDT_IRQn);

	/* Step 5. Enable WDT module */
	WDT->WTCR.WTE = 1;					// Enable WDT
	WDT->WTCR.WTR = 1;					// Clear WDT counter
	LOCKREG();
	}
	
void WDT_IRQHandler(void) {
	UNLOCKREG();
	WDT->WTCR.WTIF = 1;
	WDT->WTCR.WTR = 1;
	UNLOCKREG();
	print_lcd(3, "WDT interrupt");
	}

//---------------------------------------------------------------------TIMER
void InitTIMER0(void) {
	/* Step 0. GPIO initial */
	SYS->GPBMFP.TM0 = 1;				// System Manager Control Registers
	
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR0_S = 0;	// Select 12Mhz for Timer0 clock source
	// 0 = 12 MHz, 1 = 32 kHz, 2 = HCLK, 7 = 22.1184 MHz
	SYSCLK->APBCLK.TMR0_EN = 1;	// Enable Timer0 clock source

	/* Step 2. Select Operation mode */	
	TIMER0->TCSR.MODE = 2;			// 2 -> Select Toggle mode
	// 0 = One shot, 1 = Periodic, 2 = Toggle, 3 = continuous counting mode
	
	/* Step 3. Select Time out period 
	= (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER0->TCSR.PRESCALE = 11;	// Set Prescale [0~255]
	TIMER0->TCMPR = 1000000;		// Set TCMPR [0~16777215]
	// (1/12000000)*(11+1)*(1000000)= 1 sec or 1 Hz

	/* Step 4. Enable interrupt */
	TIMER0->TCSR.IE = 1;
	TIMER0->TISR.TIF = 1;				// Write 1 to clear the interrupt flag 		
	NVIC_EnableIRQ(TMR0_IRQn);	// Enable Timer0 Interrupt

	/* Step 5. Enable Timer module */
	TIMER0->TCSR.CRST = 1;			// Reset up counter
	TIMER0->TCSR.CEN = 1;				// Enable Timer0
	}

void TMR0_IRQHandler(void) {	// Timer0 interrupt subroutine 
	char lcd2_buffer[18] = "Timer0:";
	Timer0Counter += 1;
	sprintf(lcd2_buffer+7, " %d s.", Timer0Counter);
	print_lcd(2, lcd2_buffer);
 	TIMER0->TISR.TIF = 1;    		// Write 1 to clear the interrupt flag 
	}

//----------------------------------------------------------------------MAIN
int32_t main (void) {
	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1;
	SYSCLK->CLKSEL0.HCLK_S = 0;
	LOCKREG();

	Initial_pannel();  // call initial pannel function
	clr_all_pannal();
	                        
	InitTIMER0();
	InitWDT();
	
	while(1) {
		__NOP();
		}
	}
