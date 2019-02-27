// Nu_LB-002: L04_00_ADC7_PWM0.c

// ADC : GPA7
// PWM0 : GPA12 (Blue)

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.              */
/* edited: june 2014: dejwoot.kha@mail.kmutt.ac.th                          */
/*--------------------------------------------------------------------------*/

#include <stdio.h>																											 
#include "NUC1xx.h"
#include "LCD_Driver.h"
#include "Driver\DrvSYS.h"

#define DELAY300ms	300000 // The maximal delay time is 335000 us.
//----------------------------------------------------------------------ADC
void InitADC(void) {
	/* Step 1. GPIO initial */ 
	GPIOA->OFFD |= 0x00800000; 			// Disable digital input path
	SYS->GPAMFP.ADC7_SS21_AD6 = 1;	// Set ADC function 
				
	/* Step 2. Enable and Select ADC clock source, and then enable ADC module */          
	SYSCLK->CLKSEL1.ADC_S = 3;		// Select 22Mhz for ADC
	// 0:12MHz, 1:PLL FOUT, 2:HCLK, 3:22.1184 MHz 
	SYSCLK->CLKDIV.ADC_N = 1;			// ADC clock source = 22Mhz/2 =11Mhz;
	SYSCLK->APBCLK.ADC_EN = 1;		// Enable clock source
	ADC->ADCR.ADEN = 1;						// Enable ADC module

	/* Step 3. Select Operation mode */
	ADC->ADCR.DIFFEN = 0;     	// single end input
	ADC->ADCR.ADMD = 0;     		// single mode
		
	/* Step 4. Select ADC channel */
	ADC->ADCHER.CHEN = 0x80;		// 8-bits -> ch7
	
	/* Step 5. Enable ADC interrupt */
	ADC->ADSR.ADF = 1;					// clear the A/D interrupt flags
	ADC->ADCR.ADIE = 1;
	//	NVIC_EnableIRQ(ADC_IRQn);
	
	/* Step 6. A/D Conversion Start */
	ADC->ADCR.ADST = 1;
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

//----------------------------------------------------------------------MAIN
int32_t main (void) {
	// Enable 12Mhz and set HCLK->12Mhz
	char adc_value[15] = "ADC Value:";	
	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1;
	SYSCLK->CLKSEL0.HCLK_S = 0;
	LOCKREG();

	InitPWM();
	InitADC();

	Initial_pannel();  // call initial pannel function
	clr_all_pannal();
					 
	while(1) {
		while(ADC->ADSR.ADF == 0);		// A/D Conversion End Flag
		// A status flag that indicates the end of A/D conversion.
		
		ADC->ADSR.ADF = 1;						// This flag can be cleared by writing 1 to self
		PWMA->CMR0=ADC->ADDR[7].RSLT << 4;
		Show_Word(0,11,' ');
		Show_Word(0,12,' ');
		Show_Word(0,13,' ');
		sprintf(adc_value+10, "%d", ADC->ADDR[7].RSLT);
		print_lcd(0, adc_value);
		DrvSYS_Delay(DELAY300ms);	   	// delay (The maximal delay time is 335000 us.)
		ADC->ADCR.ADST = 1;						// 1 = Conversion start 
		}
	}
