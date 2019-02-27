// Nu_LB-002: L04_02_ADC7_PWM0_compare.c
/*

	ADC : GPA7 -> change PWMA->CMR0 = ADC->ADDR[7].RSLT<<4;
	PWM0 : GPA12 (Blue)
	Timer0 : do ADC conversion every 300ms.

*/

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.              */
/* edited: june 2014: dejwoot.kha@mail.kmutt.ac.th                          */
/*--------------------------------------------------------------------------*/

#include <stdio.h>																											 
#include "NUC1xx.h"
#include "LCD_Driver.h"
#include "Driver\DrvSYS.h"
//---------------------------------------------------------------ADC7compare
void InitADC(void) {
	/* Step 1. GPIO initial */ 
	GPIOA->OFFD |= 0x00800000;		// Disable digital input path 
																//(when input is analog signal)
	SYS->GPAMFP.ADC7_SS21_AD6 = 1;// Set ADC function 
				
	/* Step 2. Enable and Select ADC clock source, and then enable ADC module */          
	SYSCLK->CLKSEL1.ADC_S = 3;		// Select 22Mhz for ADC
	// 0:12MHz, 1:PLL FOUT, 2:HCLK, 3:22.1184 MHz 
	SYSCLK->CLKDIV.ADC_N = 1;			// ADC clock source = 22Mhz/2 =11Mhz;
	// The ADC clock frequency = (ADC clock source frequency)/(ADC_N+1)	;8-bits
	SYSCLK->APBCLK.ADC_EN = 1;		// Enable clock source
	ADC->ADCR.ADEN = 1;						// Enable ADC module

	/* Step 3. Select Operation mode */
	ADC->ADCR.DIFFEN = 0;					// Single-end analog input mode
	ADC->ADCR.ADMD   = 0;					// A/D Converter Operation Mode
	// 0:Single conversion, 2:Single-cycle scan, 3:Continuous scan 
		
	/* Step 4. Select ADC channel */
	ADC->ADCHER.CHEN = 0x0080;		// 8-bits -> ch7
	
	/* Step 5. Enable ADC interrupt */
	//ADC->ADSR.ADF = 1;						// clear the A/D interrupt flags
	//ADC->ADCR.ADIE = 1;
	//NVIC_EnableIRQ(ADC_IRQn);

	/* Step x. compare setup */
	ADC->ADCMPR[0].CMPD = 0x7FF;	// Comparison Data
	ADC->ADCMPR[0].CMPCH = 7;			// Compare Channel Selection
	ADC->ADCMPR[0].CMPCOND = 1;		// Compare Condition
	// 1: greater or equal, 0: less than
	ADC->ADCMPR[0].CMPIE = 1;			// Compare Interrupt Enable
	ADC->ADCMPR[0].CMPEN = 1;			// Compare Enable
	NVIC_EnableIRQ(ADC_IRQn);
	
	/* Step 6. A/D Conversion Start */
	ADC->ADCR.ADST=1;						
	// ADST will be cleared to 0 by hardware automatically 
	// at the ends of single mode and single cycle scan mode.
	}

void ADC_IRQHandler(void) {
	print_lcd(3, "ADC interrupt");

	ADC->ADCMPR[0].CMPIE = 0;			// Disable compare interrupt
	}
//----------------------------------------------------------------------PWM0
void InitPWM(void) {
 	/* Step 1. GPIO initial */ 
	SYS->GPAMFP.PWM0_AD13 = 1;
				
	/* Step 2. Enable and Select PWM clock source*/		
	SYSCLK->APBCLK.PWM01_EN = 1;	// Enable PWM clock
	SYSCLK->CLKSEL1.PWM01_S = 3;	// Select 22.1184Mhz for PWM clock source
	// 0:12MHz, 1:32.768 kHz, 2:HCLK, 3:22.1184 MHz 
	PWMA->PPR.CP01 = 1;			// Prescaler 0~255, Setting 0 to stop output clock
	PWMA->CSR.CSR0 = 0;			// clock divider ->	0:/2, 1:/4, 2:/8, 3:/16, 4:/1
	// PWM frequency = PWMxy_CLK/[(prescale+1)*(clock divider)*(CNR+1)]
	// Ex:= 22.1184M/[(1+1)*(2)*(2^16)] = 84.375 Hz -> T = 11.85 ms.
	
	/* Step 3. Select PWM Operation mode */
	PWMA->PCR.CH0MOD = 1;		// 0:One-shot mode, 1:Auto-load mode
	//CNR and CMR will be auto-cleared after setting CH0MOD form 0 to 1.
	PWMA->CNR0 = 0xFFFF;		// CMR >= CNR: PWM output is always high 
	PWMA->CMR0 = 0xFFFF;		
	// CMR = 0: PWM low width = (CNR) unit; PWM high width = 1 unit 

	PWMA->PCR.CH0INV = 0;			// Inverter -> 0:off, 1:on
	PWMA->PCR.CH0EN = 1;			// PWM function -> 0:Disable, 1:Enable
 	PWMA->POE.PWM0 = 1;				// Output to pin -> 0:Diasble, 1:Enable
	}
//--------------------------------------------------------------------Timer0
void InitTIMER0(void) {
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
	TIMER0->TCMPR = 300000;		// Set TCMPR [0~16777215]
	// (1/12000000)*(11+1)*(300000)= 0.3 sec

	/* Step 4. Enable interrupt */
	TIMER0->TCSR.IE = 1;
	TIMER0->TISR.TIF = 1;				// Write 1 to clear the interrupt flag 		
	NVIC_EnableIRQ(TMR0_IRQn);	// Enable Timer0 Interrupt

	/* Step 5. Enable Timer module */
	TIMER0->TCSR.CRST = 1;			// Reset up counter
	TIMER0->TCSR.CEN = 1;				// Enable Timer0
	}

void TMR0_IRQHandler(void) {	// Timer0 interrupt subroutine
	char adc_value[15] = "ADC Value:";
	while(ADC->ADSR.ADF == 0);		// A/D Conversion End Flag
	// A status flag that indicates the end of A/D conversion.
		
	ADC->ADSR.ADF = 1;					// This flag can be cleared by writing 1 to self
	PWMA->CMR0 = ADC->ADDR[7].RSLT << 4;
	Show_Word(0,11,' ');
	Show_Word(0,12,' ');
	Show_Word(0,13,' ');
	sprintf(adc_value+10, "%d", ADC->ADDR[7].RSLT);
	print_lcd(0, adc_value);
	ADC->ADCR.ADST = 1;					// 1 = Conversion start 
	
 	TIMER0->TISR.TIF = 1;				// Write 1 to clear the interrupt flag 
	}

//----------------------------------------------------------------------MAIN
int32_t main (void) {
	// Enable 12Mhz and set HCLK->12Mhz
	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1;
	SYSCLK->CLKSEL0.HCLK_S = 0;
	LOCKREG();

	InitPWM();
	InitADC();
	InitTIMER0();

	Initial_pannel();  // call initial pannel function
	clr_all_pannal();
							 	
	while(1) {
		__NOP();
		}
	}
