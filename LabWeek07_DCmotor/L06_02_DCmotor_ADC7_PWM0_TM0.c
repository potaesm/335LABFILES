// Nu_LB-002: L06_02_DCmotor_ADC7_PWM0_TM0.c
/* 
	Timer0 (GPB8) -> event counting

	PWM0 -> GPA12 to vary the DCmotor speed
	
	Timer1: read ADC7 every 1 s. 
		- change CMR0 (duty ratio), adjust speed
				PWMA->CMR0 = ADC->ADDR[7].RSLT << 4;
		- show speed

	DCmotor:
	- IN1:	PWM0 GPA12
	- EN:		E_IO_OUTPUT GPA13
	- IN2:	E_IO_OUTPUT GPA14
 */

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

static uint16_t Timer1Counter = 0;

//--------------------------------------------------------------------Timer0
void InitTIMER0(void) {	// event counting
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR0_S = 2;	// Select HCLK for event counting
	// 0 = 12 MHz, 1 = 32 kHz, 2 = HCLK, 7 = 22.1184 MHz
	SYSCLK->APBCLK.TMR0_EN = 1;	// Enable Timer clock source
	
	SYS->GPBMFP.TM0 = 1;	// Multiple Function Pin GPIOB Control Register
	//SYS->ALTMFP.PB9_S11 = 0;	// Alternative Multiple Function Pin Control Register
	
	TIMER0->TEXCON.TX_PHASE = 1;// A rising edge of external co in will be counted.
	TIMER0->TEXCON.TCDB = 1;		// Enable De-bounce
	 
	TIMER0->TCSR.CTB = 1; 			//  Enable counter mode

	/* Step 2. Select Operation mode */	
	// TIMER1->TCSR.MODE = 1;	// 1 -> Select periodic mode
	// 0 = One shot, 1 = Periodic, 2 = Toggle, 3 = continuous counting mode
	
	/* Step 3. Select Time out period 
	= (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER0->TCSR.PRESCALE = 0;	// Set Prescale [0~255]
	// TIMER1->TCMPR = 1000000;		// Set TCMPR [0~16777215]
	// (1/12000000)*(11+1)*(1000000)= 1 sec or 1 Hz	
	
	/* Step 5. Enable Timer module */
	TIMER0->TCSR.CRST = 1;			// Reset up counter
	TIMER0->TCSR.CEN = 1;				// Enable Timer
	
	TIMER0->TCSR.TDR_EN = 1;		// Enable TDR function	
	}
//----------------------------------------------------------------------PWM0
void InitPWM0(void) {
 	/* Step 1. GPIO initial */ 
	SYS->GPAMFP.PWM0_AD13 = 1;
				
	/* Step 2. Enable and Select PWM clock source*/		
	SYSCLK->APBCLK.PWM01_EN = 1;// Enable PWM clock
	SYSCLK->CLKSEL1.PWM01_S = 3;// Select 22.1184Mhz for PWM clock source

	PWMA->PPR.CP01 = 1;			// Prescaler 0~255, Setting 0 to stop output clock
	PWMA->CSR.CSR0 = 0;			// PWM clock = clock source/(Prescaler + 1)/divider
				         
	/* Step 3. Select PWM Operation mode */
	PWMA->PCR.CH0MOD = 1;		// 0:One-shot mode, 1:Auto-load mode
	// CNR and CMR will be auto-cleared after setting CH0MOD form 0 to 1.
	PWMA->CNR0 = 0xFFFF;
	PWMA->CMR0 = 0xFFFF;

	PWMA->PCR.CH0INV = 0;		// Inverter->0:off, 1:on
	PWMA->PCR.CH0EN = 1;		// PWM function->0:Disable, 1:Enable
 	PWMA->POE.PWM0 = 1;			// Output to pin->0:Diasble, 1:Enable
	}
//----------------------------------------------------------------------ADC7
void InitADC7(void) {
	/* Step 1. GPIO initial */ 
	GPIOA->OFFD |= 0x00800000; 	//Disable digital input path
	SYS->GPAMFP.ADC7_SS21_AD6 = 1; 		//Set ADC function 
				
	/* Step 2. Enable and Select ADC clock source, and then enable ADC module */          
	SYSCLK->CLKSEL1.ADC_S = 2;	//Select 22Mhz for ADC
	SYSCLK->CLKDIV.ADC_N = 1;	//ADC clock source = 22Mhz/2 =11Mhz;
	SYSCLK->APBCLK.ADC_EN = 1;	//Enable clock source
	ADC->ADCR.ADEN = 1;			//Enable ADC module

	/* Step 3. Select Operation mode */
	ADC->ADCR.DIFFEN = 0;     	//single end input
	ADC->ADCR.ADMD   = 0;     	//single mode
		
	/* Step 4. Select ADC channel */
	ADC->ADCHER.CHEN = 0x80;
	
	/* Step 5. Enable ADC interrupt */
	//ADC->ADSR.ADF = 1;     		//clear the A/D interrupt flags for safe 
	//ADC->ADCR.ADIE = 1;
	//NVIC_EnableIRQ(ADC_IRQn);
	
	/* Step 6. Enable WDT module */
	ADC->ADCR.ADST = 1;
	}	
//--------------------------------------------------------------------Timer1
void InitTIMER1(void) {
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR1_S = 0;	// Select 12Mhz for Timer0 clock source
	// 0 = 12 MHz, 1 = 32 kHz, 2 = HCLK, 7 = 22.1184 MHz
	SYSCLK->APBCLK.TMR1_EN = 1;	// Enable Timer0 clock source

	/* Step 2. Select Operation mode */	
	TIMER1->TCSR.MODE = 1;			// 1 -> Select periodic mode
	// 0 = One shot, 1 = Periodic, 2 = Toggle, 3 = continuous counting mode
	
	/* Step 3. Select Time out period 
	= (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER1->TCSR.PRESCALE = 11;	// Set Prescale [0~255]
	TIMER1->TCMPR = 1000000;		// Set TCMPR [0~16777215]
	// (1/12000000)*(11+1)*(1000000)= 1 sec

	/* Step 4. Enable interrupt */
	TIMER1->TCSR.IE = 1;
	TIMER1->TISR.TIF = 1;				// Write 1 to clear the interrupt flag 		
	NVIC_EnableIRQ(TMR1_IRQn);	// Enable Timer0 Interrupt

	/* Step 5. Enable Timer module */
	TIMER1->TCSR.CRST = 1;			// Reset up counter
	TIMER1->TCSR.CEN = 1;				// Enable Timer0
	}
//----------------------------------------------------------------Timer1_IRQ
void TMR1_IRQHandler(void) {	// Timer0 interrupt subroutine
	char adc_value[15] = "ADC7 Value:";
	char lcd2_buffer[18] = "Timer1:";
	char lcd3_buffer[18] = "T0_TDR:";

	while (ADC->ADSR.ADF == 0);	// A/D Conversion End Flag
	// A status flag that indicates the end of A/D conversion.
		
	ADC->ADSR.ADF = 1;					// This flag can be cleared by writing 1 to self
	PWMA->CMR0 = ADC->ADDR[7].RSLT << 4;
	sprintf(adc_value+11,"%d   ", ADC->ADDR[7].RSLT);
	print_lcd(0, adc_value);
	ADC->ADCR.ADST = 1;					// 1 = Conversion start 

	Timer1Counter+=1;
	sprintf(lcd2_buffer+7, " %d", Timer1Counter);
	print_lcd(2, lcd2_buffer);	
	
	sprintf(lcd3_buffer+7, " %d  ", TIMER0->TDR);
	print_lcd(3, lcd3_buffer);
	TIMER0->TCSR.CRST = 1;			// Reset up counter
	TIMER0->TCSR.CEN = 1;				// Enable Timer
	
 	TIMER1->TISR.TIF = 1;				// Write 1 to clear the interrupt flag 
	}
//----------------------------------------------------------------------GPIO
void InitGPIO() {
//	DrvGPIO_Open(E_GPA,12,E_IO_OUTPUT);	// IN1
	DrvGPIO_Open(E_GPA,13,E_IO_OUTPUT);	// EN
	DrvGPIO_Open(E_GPA,14,E_IO_OUTPUT);	// IN2
//	DrvGPIO_ClrBit(E_GPA,12);
	DrvGPIO_ClrBit(E_GPA,13);
	DrvGPIO_ClrBit(E_GPA,14);
	}
	
//----------------------------------------------------------------------MAIN
int32_t main (void) {
	// Enable 12Mhz and set HCLK->12Mhz
	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1;
	SYSCLK->CLKSEL0.HCLK_S = 0;
	LOCKREG();

	InitGPIO();
	// right turn
	DrvGPIO_SetBit(E_GPA,13);	// EN
	DrvGPIO_ClrBit(E_GPA,14);	// IN2
	
	InitPWM0();				// IN1
	InitADC7();				// to vary the DCmotor speed
	
	InitTIMER0();			// event counting
	InitTIMER1();			// read ADC7
	
	Initial_pannel();	// call initial panel function
	clr_all_pannal();
					 
	while (1) {
		__NOP();
		}
	}
