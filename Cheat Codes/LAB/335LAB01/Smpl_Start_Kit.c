// LAB335 WEEK 3 BY SUTHINAN & PONGPITH

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.              */
/* edited: JAN 2019: suthinan.musitmani@mail.kmutt.ac.th                    */
/*--------------------------------------------------------------------------*/

#include <stdio.h>	
#include <string.h>																										 
#include "NUC1xx.h"
#include "LCD_Driver.h"
#include "DrvSYS.h"
#include "Seven_Segment.h"
#include "scankey.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvSYS.h"

#define DELAY300ms	300000 // The maximal delay time is 335000 us.
#define scanDelay 4000

static uint16_t TIMER3Counter = 0;
int32_t i = 0;
int8_t state = 0;

// display an integer on four 7-segment LEDs
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
	WDT->WTCR.WTE = 1;				// Enable WDT
	WDT->WTCR.WTR = 1;				// Clear WDT counter
	LOCKREG();
	}
	
void StopWDT(void) {
	UNLOCKREG();
	WDT->WTCR.WTE = 0;				// Disable WDT
	WDT->WTCR.WTR = 1;				// Clear WDT counter
	LOCKREG();
	}
	
void ResumeWDT(void) {
	UNLOCKREG();		
	WDT->WTCR.WTE = 1;				// Enable WDT
	WDT->WTCR.WTR = 0;				// Don't Clear WDT counter
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
void InitTIMER3(void) {
	/* Step 0. GPIO initial */
	SYS->GPBMFP.TM3_PWM4 = 1;				// System Manager Control Registers
	
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR3_S = 0;	// Select 12Mhz for TIMER3 clock source
	// 0 = 12 MHz, 1 = 32 kHz, 2 = HCLK, 7 = 22.1184 MHz
	SYSCLK->APBCLK.TMR3_EN = 1;	// Enable TIMER3 clock source

	/* Step 2. Select Operation mode */	
	TIMER3->TCSR.MODE = 2;			// 2 -> Select Toggle mode
	// 0 = One shot, 1 = Periodic, 2 = Toggle, 3 = continuous counting mode
	
	/* Step 3. Select Time out period 
	= (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER3->TCSR.PRESCALE = 11;	// Set Prescale [0~255]
	TIMER3->TCMPR = 1000000;		// Set TCMPR [0~16777215]
	// (1/12000000)*(11+1)*(1000000)= 1 sec or 1 Hz

	/* Step 4. Enable interrupt */
	TIMER3->TCSR.IE = 1;
	TIMER3->TISR.TIF = 1;				// Write 1 to clear the interrupt flag 		
	NVIC_EnableIRQ(TMR3_IRQn);	// Enable TIMER3 Interrupt

	/* Step 5. Enable Timer module */
	TIMER3->TCSR.CRST = 1;			// Reset up counter
	TIMER3->TCSR.CEN = 1;				// Enable TIMER3
	}

void TMR3_IRQHandler(void) {	// TIMER3 interrupt subroutine 
	char lcd2_buffer[18] = "TIMER3:";
	TIMER3Counter += 1;
	sprintf(lcd2_buffer+7, " %d s.", TIMER3Counter);
	print_lcd(2, lcd2_buffer);
 	TIMER3->TISR.TIF = 1;    		// Write 1 to clear the interrupt flag 
	}

//---------------------------------------------------------------------COUNTER
void Delay(uint32_t counter) {
	while(counter--);
	}

void countUp() {
	i = i + 100;						// increment i
	if (i == 100000) i = 0;
	}
	
void countDown() {
	i = i - 100;						// decrement i
	if (i == 0) i = 100000;
	}
	
//---------------------------------------------------------------------EINTCallback
void EINT1Callback(void) {
	UNLOCKREG();
	WDT->WTCR.WTIS = 7;
	LOCKREG();
	state = 2;
	print_lcd(3, "               ");
	print_lcd(3, "WDT Change!");
	}

//----------------------------------------------------------------------MAIN
int32_t main (void) {
	
	int8_t number;
	OpenKeyPad();
	/* Configure general GPIO interrupt */
	DrvGPIO_Open(E_GPB, 15, E_IO_INPUT);
	/* Configure external interrupt */
	DrvGPIO_EnableEINT1(E_IO_BOTH_EDGE, E_MODE_EDGE, EINT1Callback);

		UNLOCKREG();
		SYSCLK->PWRCON.XTL12M_EN = 1;
		SYSCLK->CLKSEL0.HCLK_S = 0;
		LOCKREG();
		Initial_pannel();  // call initial pannel function
		clr_all_pannal();
		InitTIMER3();
		InitWDT();
		
		while(1) {
			
			number = Scankey();
		
		if (number == 1) {
			print_lcd(3, "               ");
			print_lcd(0, "               ");
			print_lcd(0, "Stop!");
			StopWDT();
			}
		if (number == 2) {
			print_lcd(3, "               ");
			print_lcd(0, "               ");
			print_lcd(0, "Resume!");
			ResumeWDT();
			}
		if (number == 3) {
				state = 1;
			}
		if (state == 1) {
				seg_display(i/10);	// display i on 7-segment display
				countUp();
			}         
		if ((GPIOB->PIN &= 0x8000) == 0) {
			}
		if (state == 2) {
				seg_display(i/10);	// display i on 7-segment display
				countDown();
			}
		if (number == 4) {
				state = 3;
			}
		if (state == 3) {
				seg_display(i/10);	// display i on 7-segment display
			}
			
		}
	}
