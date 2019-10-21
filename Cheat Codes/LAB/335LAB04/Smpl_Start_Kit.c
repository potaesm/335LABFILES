// LAB335 WEEK 5 BY SUTHINAN & PONGPITH
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.              */
/* edited: FEB 2019: suthinan.musitmani@mail.kmutt.ac.th                   */
/*--------------------------------------------------------------------------*/

#include <stdio.h>																											 
#include "NUC1xx.h"
#include "LCD_Driver.h"
#include "Driver\DrvSYS.h"
#include "Driver\DrvGPIO.h"
#include "Seven_Segment.h"
#define scanDelay 4000
#define DELAY300ms	300000
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
unsigned char SEG_BUF_HEX[16]={SEG_N0, SEG_N1, SEG_N2, SEG_N3, SEG_N4, SEG_N5, SEG_N6, SEG_N7, SEG_N8, SEG_N9, SEG_Na, SEG_Nb, SEG_Nc, SEG_Nd, SEG_Ne, SEG_Nf};

// 7Segment HEX
void show_seven_segment_Hex(unsigned char no, unsigned char number) {
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

void seg_display_Hex(int16_t value)
{
  int8_t digit;
		digit = value / 256;
		close_seven_segment();
		show_seven_segment_Hex(1,digit);
		DrvSYS_Delay(scanDelay);
			
		value = value - digit * 256;
		digit = value / 16;
		close_seven_segment();
		show_seven_segment_Hex(0,digit);
		DrvSYS_Delay(scanDelay);
	}
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
	
	/* Step 6. A/D Conversion Start */
	ADC->ADCR.ADST=1;						
	// ADST will be cleared to 0 by hardware automatically 
	// at the ends of single mode and single cycle scan mode.
	}

void InitADC_LESSTHAN_700(void) {
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
	ADC->ADCMPR[0].CMPD = 0x700;	// Comparison Data 0x700
	ADC->ADCMPR[0].CMPCH = 7;			// Compare Channel Selection
	ADC->ADCMPR[0].CMPCOND = 0;		// Compare Condition
	// 1: greater or equal, 0: less than
	ADC->ADCMPR[0].CMPIE = 1;			// Compare Interrupt Enable
	ADC->ADCMPR[0].CMPEN = 1;			// Compare Enable
	NVIC_EnableIRQ(ADC_IRQn);
	
	/* Step 6. A/D Conversion Start */
	ADC->ADCR.ADST=1;						
	// ADST will be cleared to 0 by hardware automatically 
	// at the ends of single mode and single cycle scan mode.
	}

void InitADC_LESSTHAN_100(void) {
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
	ADC->ADCMPR[0].CMPD = 0x100;	// Comparison Data 0x100
	ADC->ADCMPR[0].CMPCH = 7;			// Compare Channel Selection
	ADC->ADCMPR[0].CMPCOND = 0;		// Compare Condition
	// 1: greater or equal, 0: less than
	ADC->ADCMPR[0].CMPIE = 1;			// Compare Interrupt Enable
	ADC->ADCMPR[0].CMPEN = 1;			// Compare Enable
	NVIC_EnableIRQ(ADC_IRQn);
	
	/* Step 6. A/D Conversion Start */
	ADC->ADCR.ADST=1;						
	// ADST will be cleared to 0 by hardware automatically 
	// at the ends of single mode and single cycle scan mode.
	}
	
void InitADC_MORETHAN_D00(void) {
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
	ADC->ADCMPR[0].CMPD = 0xD00;	// Comparison Data 0x100
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
	//For ADC_LESSTHAN_100
	if (ADC->ADDR[7].RSLT < 0x100) {
		GPC_12 = 0;
		}
	//For ADC_MORETHAN_D00
	if (ADC->ADDR[7].RSLT > 0xD00) {
		GPC_15 = 0;
		}
	//For ADC_LESSTHAN_700
	//GPC_12 = 0;										// Macro -> Turn 'On' LED5
	//GPC_13 = 0;										// Macro -> Turn 'On' LED6
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
	// char lcd1_buffer[18] = "A7 0xABC ZZZZd";
	char adc_value[15] = "A7 0xABC ZZZZd";
	while(ADC->ADSR.ADF == 0);		// A/D Conversion End Flag
	// A status flag that indicates the end of A/D conversion.
		
	ADC->ADSR.ADF = 1;					// This flag can be cleared by writing 1 to self
	PWMA->CMR0 = ADC->ADDR[7].RSLT << 4;
	Show_Word(0,11,' ');
	Show_Word(0,12,' ');
	Show_Word(0,13,' ');
	sprintf(adc_value+5, "%X %dd    ", ADC->ADDR[7].RSLT, ADC->ADDR[7].RSLT);
	print_lcd(0, adc_value);
	ADC->ADCR.ADST = 1;					// 1 = Conversion start
	if (ADC->ADDR[7].RSLT < 0x500) {
			GPC_13 = 1;
			GPC_14 = 1;
		}
	if (ADC->ADDR[7].RSLT > 0x100 && ADC->ADDR[7].RSLT < 0x500) {
			GPC_12 = 1;
			print_lcd(3, "              ");
			ADC->ADCMPR[0].CMPIE = 1;			// Compare Interrupt Enable
			ADC->ADCMPR[0].CMPEN = 1;			// Compare Enable
			NVIC_EnableIRQ(ADC_IRQn);
		}
	if (ADC->ADDR[7].RSLT > 0x500 && ADC->ADDR[7].RSLT < 0x700) {
			GPC_13 = 0;
			GPC_14 = 0;
		}
	if (ADC->ADDR[7].RSLT > 0x700 && ADC->ADDR[7].RSLT < 0xD00) {
			GPC_13 = 1;
			GPC_14 = 1;
			GPC_15 = 1;
			print_lcd(3, "              ");
			ADC->ADCMPR[0].CMPIE = 1;			// Compare Interrupt Enable
			ADC->ADCMPR[0].CMPEN = 1;			// Compare Enable
			NVIC_EnableIRQ(ADC_IRQn);
		}
 	TIMER0->TISR.TIF = 1;				// Write 1 to clear the interrupt flag 
	}

//----------------------------------------------------------------------MAIN
int32_t main (void) {
	int8_t state = 0;
	// Enable 12Mhz and set HCLK->12Mhz
	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1;
	SYSCLK->CLKSEL0.HCLK_S = 0;
	LOCKREG();

	//InitPWM();
	//InitADC();
	InitTIMER0();

	Initial_pannel();  // call initial pannel function
	clr_all_pannal();
							 	
	while(1) {
			if (state == 0) {
					InitADC_LESSTHAN_100();
					state = 1;
				}
			else {
					InitADC_MORETHAN_D00();
					state = 0;
				}
			seg_display_Hex(ADC->ADDR[7].RSLT);
		}
	}
