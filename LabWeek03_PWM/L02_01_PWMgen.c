// Nu_LB-002:	L02_01_PWMgen.c

//	PWM1 -> GPA13 High level: 262 msec, Low level: 786 msec
//	PWM2 -> GPA14 High level: 682 usec, Low level: 682 usec(microsecond)

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.              */
/* edited: june 2014: dejwoot.kha@mail.kmutt.ac.th                          */
/*--------------------------------------------------------------------------*/

#include <stdio.h>																											 
#include "NUC1xx.h"
#include "LCD_Driver.h"
#include "Driver\DrvSYS.h"

void InitPWM1(void) {
 	/* Step 1. GPIO initial */ 
	SYS->GPAMFP.PWM1_AD14 = 1;		// System Manager Control Registers
	
	/* Step 2. Enable and Select PWM clock source*/		
	SYSCLK->APBCLK.PWM01_EN = 1;	// Enable PWM clock
	SYSCLK->CLKSEL1.PWM01_S = 0;	// Select 12MHz for PWM clock source
	// 0:12MHz, 1:32.768 kHz, 2:HCLK, 3:22.1184 MHz 
	PWMA->PPR.CP01 = 11;	// Prescaler 0~255, Setting 0 to stop output clock
	PWMA->CSR.CSR1 = 3;		// clock divider ->	0:/2, 1:/4, 2:/8, 3:/16, 4:/1
	// PWM frequency = PWMxy_CLK/[(prescale+1)*(clock divider)*(CNR+1)]
	// Ex:= 12M/[(11+1)*(16)*(2^16)] = 0.95367 Hz -> T = 1.048576 ~ 262+786

	/* Step 3. Select PWM Operation mode */
	PWMA->PCR.CH1MOD = 1;		// 0:One-shot mode, 1:Auto-load mode
	// CNR and CMR will be auto-cleared after setting CH0MOD form 0 to 1.
	PWMA->CNR1 = 0xFFFF;
	PWMA->CMR1 = 0x3FFF;
	// CMR < CNR:	PWM low width = (CNR-CMR) unit [one PWM clock cycle]
	//						PWM high width = (CMR+1) unit 

	PWMA->PCR.CH1INV = 0;	// Inverter -> 0:off, 1:on
	PWMA->PCR.CH1EN = 1;	// PWM function -> 0:Disable, 1:Enable
 	PWMA->POE.PWM1 = 1;		// Output to pin -> 0:Diasble, 1:Enable
	}

void InitPWM2(void) {
 	/* Step 1. GPIO initial */ 
	SYS->GPAMFP.PWM2_AD15 = 1;		// System Manager Control Registers
	
	/* Step 2. Enable and Select PWM clock source*/		
	SYSCLK->APBCLK.PWM23_EN = 1;	// Enable PWM clock
	SYSCLK->CLKSEL1.PWM23_S = 0;	// Select 12Mhz for PWM clock source
	// 0:12MHz, 1:32.768 kHz, 2:HCLK, 3:22.1184 MHz 
	PWMA->PPR.CP23 = 1;		// Prescaler 0~255, Setting 0 to stop output clock
	PWMA->CSR.CSR2 = 4;		// clock divider ->	0:/2, 1:/4, 2:/8, 3:/16, 4:/1
	// PWM frequency = PWMxy_CLK/[(prescale+1)*(clock divider)*(CNR+1)]
	// Ex:= 12M/[(1+1)*(1)*(8191)] = 732.5 Hz -> T = 1365 micros. ~ 682+682
	
	/* Step 3. Select PWM Operation mode */
	PWMA->PCR.CH2MOD = 1;	// 0:One-shot mode, 1:Auto-load mode
	// CNR and CMR will be auto-cleared after setting CH0MOD form 0 to 1.
	PWMA->CNR2 = 0x1FFF;	// 0x1FFF = 8191
	PWMA->CMR2 = 0x0FFF;

	PWMA->PCR.CH2INV = 0;	// Inverter -> 0:off, 1:on
	PWMA->PCR.CH2EN = 1;	// PWM function -> 0:Disable, 1:Enable
 	PWMA->POE.PWM2 = 1;		// Output to pin -> 0:Diasble, 1:Enable
	}

//----------------------------------------------------------------------MAIN
int32_t main (void) {
	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1;	
	SYSCLK->CLKSEL0.HCLK_S = 0;
	// 0:Clock source from external 4~24 MHz(12MHz) high speed crystal clock 
	LOCKREG();

	InitPWM1();
	InitPWM2();
	 
	while(1) {}				 
	}
