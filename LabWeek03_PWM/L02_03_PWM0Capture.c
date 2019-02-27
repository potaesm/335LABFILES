// Nu_LB-002:	L02_03_PWM0Capture.c

//	PWM0 -> GPA12 - capture input
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

#define	PWM_CNR	0xFFFF

static uint16_t Timer3Counter=0;
uint16_t	CaptureCounter = 0;
uint32_t	CaptureValue[2];
//------------------------------------------------------------------Capture0
void InitCapture0(void) {
 	/* Step 1. GPIO initial */ 
	SYS->GPAMFP.PWM0_AD13 = 1;
	
	/* Step 2. Enable and Select PWM clock source*/		
	SYSCLK->APBCLK.PWM01_EN = 1; // Enable PWM clock
	SYSCLK->CLKSEL1.PWM01_S = 0; // Select 12Mhz for PWM clock source
	// 0:12MHz, 1:32.768 kHz, 2:HCLK, 3:22.1184 MHz
	
	PWMA->PPR.CP01 = 11;		// Prescaler 0~255, Setting 0 to stop output clock
	PWMA->CSR.CSR0 = 4;			// clock divider -> 0:/2, 1:/4, 2:/8, 3:/16, 4:/1
									         
	/* Step 3. Select PWM Operation mode */
	PWMA->PCR.CH0MOD = 1;		// 0:One-shot mode, 1:Auto-load mode
	//CNR and CMR will be auto-cleared after setting CH0MOD from 0 to 1.
	PWMA->CNR0 = PWM_CNR;			// Set Reload register
	PWMA->CAPENR = 1;				// Enable Capture function pin
	PWMA->CCR0.CAPCH0EN = 1;// Enable Capture function

	/* Step 4. Set PWM Interrupt */
	PWMA->CCR0.CRL_IE0 = 1;	// Enable Capture rising edge interrupt
	PWMA->CCR0.CFL_IE0 = 1;	// Enable Capture falling edge interrupt
	PWMA->PIER.PWMIE0 = 1;	// Enable PWM interrupt for down-counter equal zero.
	NVIC_EnableIRQ(PWMA_IRQn);  // Enable PWM inturrupt

	/* Step 5. Enable PWM down counter*/
	PWMA->PCR.CH0EN = 1;		// Enable PWM down counter
	}
//----------------------------------------------------------------------PWM1
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
	//CNR and CMR will be auto-cleared after setting CH0MOD form 0 to 1.
	PWMA->CNR1 = 0xFFFF;
	PWMA->CMR1 = 0x3FFF;
	// CMR < CNR:	PWM low width = (CNR-CMR) unit [one PWM clock cycle]
	//						PWM high width = (CMR+1) unit 

	PWMA->PCR.CH1INV = 0;	// Inverter -> 0:off, 1:on
	PWMA->PCR.CH1EN = 1;	// PWM function -> 0:Disable, 1:Enable
 	PWMA->POE.PWM1 = 1;		// Output to pin -> 0:Diasble, 1:Enable
	}
//----------------------------------------------------------------------PWM2
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
	//CNR and CMR will be auto-cleared after setting CH0MOD form 0 to 1.
	PWMA->CNR2 = 0x1FFF;	// 0x1FFF = 8191
	PWMA->CMR2 = 0x0FFF;

	PWMA->PCR.CH2INV = 0;	//Inverter->0:off, 1:on
	PWMA->PCR.CH2EN = 1;	//PWM function->0:Disable, 1:Enable
 	PWMA->POE.PWM2 = 1;		//Output to pin->0:Diasble, 1:Enable
	}
//------------------------------------------------------------------PWMA_IRQ
void PWMA_IRQHandler(void) {		// PWM interrupt subroutine 
	if (PWMA->PIIR.PWMIF0) {
		CaptureCounter++;						// Delay (PWM_CNR+1) usec
		if (CaptureCounter == 0) {	// Overflow
			}
		PWMA->PIIR.PWMIF0	=	1;			// write 1 to clear this bit to zero
		}
	if (PWMA->CCR0.CAPIF0) {
		if (PWMA->CCR0.CFLRI0) {		// Calculate High Level width
			CaptureValue[0] = CaptureCounter*(PWM_CNR+1)+(PWM_CNR-PWMA->CFLR0);//usec
			CaptureCounter = 0;				// reset
			PWMA->CCR0.CFLRI0 = 0;// write 0 to clear this bit to zero if BCn bit is 0
		}
		if (PWMA->CCR0.CRLRI0) {		//Calculate Low Level width
			CaptureValue[1] = CaptureCounter*(PWM_CNR+1)+(PWM_CNR-PWMA->CRLR0);//usec
			CaptureCounter = 0;				// reset
			PWMA->CCR0.CRLRI0 = 0;// write 0 to clear this bit to zero if BCn bit is 0	
			}
		PWMA->CCR0.CAPIF0 = 1;	// write 1 to clear this bit to zero
		}
	}
//--------------------------------------------------------------------Timer3
void InitTIMER3(void) {
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR3_S = 0;	// Select 12Mhz for Timer3 clock source
	// 0 = 12 MHz, 1 = 32 kHz, 2 = HCLK, 7 = 22.1184 MHz
	SYSCLK->APBCLK.TMR3_EN = 1;	// Enable Timer0 clock source

	/* Step 2. Select Operation mode */	
	TIMER3->TCSR.MODE = 1;			// 1 -> Select periodic mode
	// 0 = One shot, 1 = Periodic, 2 = Toggle, 3 = continuous counting mode
	
	/* Step 3. Select Time out period 
	= (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER3->TCSR.PRESCALE = 11;	// Set Prescale [0~255]
	TIMER3->TCMPR = 1000000;		// Set TCMPR [0~16777215]
	// (1/12000000)*(11+1)*(1000000)= 1 sec or 1 Hz

	/* Step 4. Enable interrupt */
	TIMER3->TCSR.IE = 1;
	TIMER3->TISR.TIF = 1;				// Write 1 to clear the interrupt flag 		
	NVIC_EnableIRQ(TMR3_IRQn);	// Enable Timer0 Interrupt

	/* Step 5. Enable Timer module */
	TIMER3->TCSR.CRST = 1;			// Reset up counter
	TIMER3->TCSR.CEN = 1;				// Enable Timer0
	}
//----------------------------------------------------------------Timer3_IRQ
void TMR3_IRQHandler(void) {	// Timer0 interrupt subroutine
	char lcd1_buffer[18] = "Timer3:";
	char lcd2_buffer[18] = "High:";
	char lcd3_buffer[18] = "Low: ";
	
	Timer3Counter += 1;
	sprintf(lcd1_buffer+7, " %d s.", Timer3Counter);
	print_lcd(1, lcd1_buffer);
 	
	/* Display capture values */
	if (CaptureValue[0] >= 1000000) {
		sprintf(lcd2_buffer+5, "%dsec", CaptureValue[0]/1000000);
		} else if (CaptureValue[0] >= 1000) {
		sprintf(lcd2_buffer+5, "%dmsec", CaptureValue[0]/1000);
		} else
		sprintf(lcd2_buffer+5, "%dusec", CaptureValue[0]);
	print_lcd(2, lcd2_buffer);

	if (CaptureValue[1] >= 1000000) {
		sprintf(lcd3_buffer+5, "%dsec", CaptureValue[1]/1000000);
		} else if (CaptureValue[1] >= 1000) {
		sprintf(lcd3_buffer+5, "%dmsec", CaptureValue[1]/1000);
		} else
		sprintf(lcd3_buffer+5, "%dusec", CaptureValue[1]);
	print_lcd(3, lcd3_buffer);
	
	TIMER3->TISR.TIF = 1;    		// Write 1 to clear the interrupt flag 
	}

//----------------------------------------------------------------------MAIN
int32_t main (void) {
	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1;
	SYSCLK->CLKSEL0.HCLK_S = 0;
	LOCKREG();
	Initial_pannel();  //call initial pannel function
	clr_all_pannal();
	InitPWM1();
	InitPWM2();
	InitCapture0();
	InitTIMER3();
	print_lcd(0, "Capture demo");
	print_lcd(1, ">>GPA12");
	 
	while(1) {}
	}
