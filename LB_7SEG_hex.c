/*	LB_7SEG_hex.c

-> display 4-7SEG

Timer0 update counter every 1 s.

*/

/*--------------------------------------------------------------*/
/*                                                              */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.  */
/*                                                              */
/*--------------------------------------------------------------*/

#include <stdio.h>
#include "NUC1xx.h"
#include "Driver\DrvSYS.h"
#include "Driver\DrvGPIO.h"
#include "LCD_Driver.h"
#include "Driver\DrvI2C.h"
#include "Seven_Segment.h"

#define scanDelay 4000

#define SEG_N0   0x82 
#define SEG_N1   0xEE 
#define SEG_N2   0x07 
#define SEG_N3   0x46 
#define SEG_N4   0x6A  
#define SEG_N5   0x52 
#define SEG_N6   0x12 
#define SEG_N7   0xE6 
#define SEG_N8   0x02 
#define SEG_N9   0x62
#define SEG_Na   0x22
#define SEG_Nb   0x1A
#define SEG_Nc   0x93
#define SEG_Nd   0x0E
#define SEG_Ne   0x13
#define SEG_Nf   0x33

static uint16_t Timer0Counter = 0;

unsigned char SEG_BUF_HEX[16]={SEG_N0, SEG_N1, SEG_N2, SEG_N3, SEG_N4, SEG_N5, SEG_N6, SEG_N7, SEG_N8, SEG_N9, SEG_Na, SEG_Nb, SEG_Nc, SEG_Nd, SEG_Ne, SEG_Nf}; 

void show_seven_segment_Hex(unsigned char no, unsigned char number)
{
    unsigned char temp,i;
	temp=SEG_BUF_HEX[number];
	
	for(i=0;i<8;i++)
	    {
		if((temp&0x01)==0x01)		   		   
		   DrvGPIO_SetBit(E_GPE,i);
		   else
		   DrvGPIO_ClrBit(E_GPE,i);		  
		   temp=temp>>1;
		}
		DrvGPIO_SetBit(E_GPC,4+no);	

}

void seg_display(int16_t value)
{
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

		value = value - digit * 100;
		digit = value / 10;
		close_seven_segment();
		show_seven_segment(1,digit);
		DrvSYS_Delay(scanDelay);

		value = value - digit * 10;
		digit = value;
		close_seven_segment();
		show_seven_segment(0,digit);
		DrvSYS_Delay(scanDelay);
}

void seg_display_Hex(int16_t value)
{
  int8_t digit;
		digit = value / 256;
		close_seven_segment();
		show_seven_segment_Hex(3,digit);
		DrvSYS_Delay(scanDelay);
			
		value = value - digit * 256;
		digit = value / 16;
		close_seven_segment();
		show_seven_segment_Hex(2,digit);
		DrvSYS_Delay(scanDelay);

		value = value - digit * 16;
		digit = value;
		close_seven_segment();
		show_seven_segment_Hex(1,digit);
		DrvSYS_Delay(scanDelay);

		//value = value - digit * 10;
		//digit = value;
		//close_seven_segment();
		//show_seven_segment(0,digit);
		//DrvSYS_Delay(scanDelay);
}

void InitTIMER0(void)
{
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR0_S = 0;	// Select 12Mhz for Timer0 clock source
	// 0 = 12 MHz, 1 = 32 kHz, 2 = HCLK, 7 = 22.1184 MHz
  SYSCLK->APBCLK.TMR0_EN = 1;	// Enable Timer0 clock source

	/* Step 2. Select Operation mode */	
	TIMER0->TCSR.MODE = 1;				// 1 -> Select periodic mode
	// 0 = One shot, 1 = Periodic, 2 = Toggle, 3 = continuous counting mode
	
	/* Step 3. Select Time out period 
	= (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER0->TCSR.PRESCALE = 11;	// Set Prescale [0~255]
	TIMER0->TCMPR = 1000000;		// Set TCMPR [0~16777215]
	// (1/12000000)*(11+1)*(1000000)= 1 sec

	/* Step 4. Enable interrupt */
	TIMER0->TCSR.IE = 1;
	TIMER0->TISR.TIF = 1;				// Write 1 to clear the interrupt flag 		
	NVIC_EnableIRQ(TMR0_IRQn);	// Enable Timer0 Interrupt

	/* Step 5. Enable Timer module */
	TIMER0->TCSR.CRST = 1;			// Reset up counter
	TIMER0->TCSR.CEN = 1;				// Enable Timer0
}

void TMR0_IRQHandler(void) 		// Timer0 interrupt subroutine 
{
	char lcd2_buffer[18]="Timer0:";

	Timer0Counter+=1;
	sprintf(lcd2_buffer+7," %d",Timer0Counter);
	print_lcd(2, lcd2_buffer);	
	
	TIMER0->TISR.TIF = 1;				// Write 1 to clear the interrupt flag 
}

int main(void)
{
	UNLOCKREG();
    DrvSYS_Open(48000000);
	LOCKREG();	
	
	Initial_pannel();	// call initial panel function
	clr_all_pannal();
	
	InitTIMER0();	
	
	while (1) {
		seg_display_Hex(Timer0Counter);
	}	
}
