// Nu_LB-002:	L02_02_PWMgenPair.c

//	PWM2 -> GPA14 High level: 682 usec, Low level: 682 usec(microsecond)
//	PWM3 -> GPA15 - complementary PWM 

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.              */
/* edited: june 2014: dejwoot.kha@mail.kmutt.ac.th                          */
/*--------------------------------------------------------------------------*/

#include <stdio.h>																											 
#include "NUC1xx.h"
#include "LCD_Driver.h"

void InitPWM23(void) {
 	/* Step 1. GPIO initial */ 
	SYS->GPAMFP.PWM2_AD15 = 1;		// System Manager Control Registers
	SYS->GPAMFP.PWM3_I2SMCLK = 1;	// System Manager Control Registers
	
	/* Step 2. Enable and Select PWM clock source*/		
	SYSCLK->APBCLK.PWM23_EN = 1;	// Enable PWM clock
	SYSCLK->CLKSEL1.PWM23_S = 0;	// Select 12Mhz for PWM clock source
	// 0:12MHz, 1:32.768 kHz, 2:HCLK, 3:22.1184 MHz 
	PWMA->PPR.CP23 = 1;		// Prescaler 0~255, Setting 0 to stop output clock
	PWMA->CSR.CSR2 = 4;		// clock divider -> 0:/2, 1:/4, 2:/8, 3:/16, 4:/1
	// PWM frequency = PWMxy_CLK/[(prescale+1)*(clock divider)*(CNR+1)]
	// Ex:= 12M/[(1+1)*(1)*(8191)] = 732.5 Hz -> T = 1365 micros. ~ 682+682
	
	/* Step 3. Select PWM Operation mode */
	PWMA->PCR.CH2MOD = 1;	// 0:One-shot mode, 1:Auto-load mode
	//PWMA->PCR.CH3MOD=1;	// 0:One-shot mode, 1:Auto-load mode	
	//CNR and CMR will be auto-cleared after setting CH0MOD form 0 to 1.
	PWMA->CNR2 = 0x1FFF;	// 0x1FFF = 8191
	PWMA->CMR2 = 0x0FFF;
	
	PWMA->PPR.DZI23 = 127;	// These 8-bit determine dead zone length.
	PWMA->PCR.DZEN23 = 1;		// Dead-Zone 2 Generator Enable
	
	PWMA->PCR.CH2INV = 0;		// Inverter -> 0:off, 1:on
	PWMA->PCR.CH2EN = 1;		// PWM function -> 0:Disable, 1:Enable
 	PWMA->POE.PWM2 = 1;			// Output to pin -> 0:Diasble, 1:Enable
	
 	PWMA->POE.PWM3 = 1;		// Output to pin -> 0:Diasble, 1:Enable	
	}

//----------------------------------------------------------------------MAIN
int32_t main (void) {
	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1;
	SYSCLK->CLKSEL0.HCLK_S = 0;
	// 0:Clock source from external 4~24 MHz(12MHz) high speed crystal clock 
	LOCKREG();

	InitPWM23();

	while(1) {}			 
	}
