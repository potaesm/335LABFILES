// Nu_LB-002: L05_01_I2C1_8591_A2D_D2A_ADC0_8574disp.c
/*

	D/A from PCF8591 (read A/D via VR) input to ADC0
	
	ADC0: GPA0
	
	i2c1:	GPA10 - SDA
				GPA11 - SCL	
	
	PCF8591: 	addr. 90h write
						addr. 91h read
	
	PCF8574:	addr 0x70 the right 7SEG
						addr 0x72	the left 7SEG
						
	can't use with KeyPad
	Timer1: read A2D_8591(8bits) -> out D2A_8591 every 1s.
	Timer0: read ADC0_LB (12bits)
*/

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.              */
/* edited: june 2014: dejwoot.kha@mail.kmutt.ac.th                          */
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include "NUC1xx.h"
#include "Driver\DrvSYS.h"
#include "Driver\DrvGPIO.h"
#include "LCD_Driver.h"
#include "Driver\DrvI2C.h"

static uint16_t TimerCounter1 = 0;

uint8_t HEX2Disp(uint8_t hexNum) {
	static const uint8_t lookUp[16] = {
		0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8,
		0x80, 0x90, 0x88, 0x83, 0xC6, 0xA1, 0x86, 0x8E
		};
	uint8_t hexDisp = lookUp[hexNum];
		
	return hexDisp;
	}
//-------------------------------------------------------------------PCF8574
void Write_to_any8574(uint8_t i2c_addr, uint8_t data) {
	SystemCoreClock = DrvSYS_GetHCLKFreq();
	//Open I2C1 and set clock = 50Kbps
	DrvI2C_Open(I2C_PORT1, 50000);
	
	//send i2c start
	DrvI2C_Ctrl(I2C_PORT1, 1, 0, 0, 0);	// set start
	while (I2C1->I2CON.SI == 0);				// poll si flag
	
	//send writer command
	I2C1->I2CDAT = i2c_addr;						// send writer command to 8574
	DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 0); // clr si flag
	while (I2C1->I2CON.SI == 0);				// poll si flag

	//send data
	I2C1->I2CDAT = data;								// write data to 
	DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 1); // clr si and set ack	
	while (I2C1->I2CON.SI == 0);				// poll si flag
   	
	//send i2c stop
	DrvI2C_Ctrl(I2C_PORT1, 0, 1, 1, 0); // send stop	
	while (I2C1->I2CON.STO);						/* if a STOP condition is detected 
										this flag will be cleared by hardware automatically. */
	//while (I2C1->I2CON.SI == 0);			// poll si flag
	
	DrvI2C_Close(I2C_PORT1);
}

void disp2Digit8574(uint8_t data) {
	Write_to_any8574(0x72,HEX2Disp(data >> 4));	
	Write_to_any8574(0x70,HEX2Disp(data &= 0x0F));
	}
//-------------------------------------------------------------------D2A8591
void out_D2A_8591(uint8_t data) {
  // Open I2C1 and set clock = 50Kbps
	SystemCoreClock = DrvSYS_GetHCLKFreq(); 
	DrvI2C_Open(I2C_PORT1, 50000);
	
	// send i2c start
	DrvI2C_Ctrl(I2C_PORT1, 1, 0, 1, 0);	// set start
	while (I2C1->I2CON.SI == 0);				// poll si flag
	 
	// send writer command
	I2C1->I2CDAT = 0x90;								// 8591
	DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 0);	// clr si
	while (I2C1->I2CON.SI == 0);				// poll si flag
	
	I2C1->I2CDAT = 0x40;								// D2A out
	DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 0);	// clr si
	while (I2C1->I2CON.SI == 0);				// poll si flag	
	
	I2C1->I2CDAT = data;								// 8591
	DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 0);	// clr si
	while (I2C1->I2CON.SI == 0);				// poll si flag
	
	// send i2c stop
	DrvI2C_Ctrl(I2C_PORT1, 0, 1, 1, 0);	// clr si and set stop
	while (I2C1->I2CON.STO);
	
	DrvI2C_Close(I2C_PORT1);
	}
//---------------------------------------------------------------A2D8591_ch0
uint8_t Read_A2D_8591(void)	{	// Read Ch0
	uint8_t a2d_value;
	// Open I2C1 and set clock = 50Kbps
	SystemCoreClock = DrvSYS_GetHCLKFreq(); 
	DrvI2C_Open(I2C_PORT1, 50000);
	// send i2c start
	DrvI2C_Ctrl(I2C_PORT1, 1, 0, 1, 0);	// set start
	while (I2C1->I2CON.SI == 0);				// poll si flag
	 
	// send writer command
	I2C1->I2CDAT = 0x90;								// 8591 write address
	DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 0);	// clr si
	while (I2C1->I2CON.SI == 0);				// poll si flag
	
	I2C1->I2CDAT = 0;										// control byte -> ch0
	DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 0);	// clr si
	while (I2C1->I2CON.SI == 0);				// poll si flag	
	
	// send start flag
	DrvI2C_Ctrl(I2C_PORT1, 1, 0, 1, 0);	// clr si and send start	
	while (I2C1->I2CON.SI == 0);				// poll si flag
	
	I2C1->I2CDAT = 0x91;								// read 8591
	DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 0);	// clr si
	while (I2C1->I2CON.SI == 0);				// poll si flag
	
	// receive data
	//I2C0->I2CDAT = 0XFF;
	DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 0);	// clr si	
	while (I2C1->I2CON.SI == 0);				// poll si flag
	a2d_value = I2C1->I2CDAT;
	
	// send i2c stop
	DrvI2C_Ctrl(I2C_PORT1, 0, 1, 1, 0);	// clr si and set stop
	while (I2C1->I2CON.STO);						/* if a STOP condition is detected 
										this flag will be cleared by hardware automatically. */
	DrvI2C_Close(I2C_PORT1);

	return a2d_value; 
	}
//--------------------------------------------------------------------Timer1	
void InitTIMER1(void) {
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR1_S = 0;	// Select 12Mhz for Timer0 clock source
	// 0 = 12 MHz, 1 = 32 kHz, 2 = HCLK, 7 = 22.1184 MHz
	SYSCLK->APBCLK.TMR1_EN = 1;	// Enable Timer0 clock source

	/* Step 2. Select Operation mode */	
	TIMER1->TCSR.MODE = 1;				// 1 -> Select periodic mode
	// 0 = One shot, 1 = Periodic, 2 = Toggle, 3 = continuous counting mode
	
	/* Step 3. Select Time out period 
	= (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER1->TCSR.PRESCALE = 11;	// Set Prescale [0~255]
	TIMER1->TCMPR = 1000000;		// Set TCMPR [0~16777215]
	// (1/12000000)*(11+1)*(1000000)= 1 sec or 1 Hz

	/* Step 4. Enable interrupt */
	TIMER1->TCSR.IE = 1;
	TIMER1->TISR.TIF = 1;				// Write 1 to clear the interrupt flag 		
	NVIC_EnableIRQ(TMR1_IRQn);	// Enable Timer0 Interrupt

	/* Step 5. Enable Timer module */
	TIMER1->TCSR.CRST = 1;			// Reset up counter
	TIMER1->TCSR.CEN = 1;				// Enable Timer0
	}

void TMR1_IRQHandler(void) {	// Timer1 interrupt subroutine
	char lcd_buffer[18] = "Timer1:";
	uint8_t i2cdata = 0;
	char ch0A2D8591[18] = "ch0A2D8591:";
	
	TimerCounter1 += 1;
	sprintf(lcd_buffer+7, " %d s.  ", TimerCounter1);
	print_lcd(0, lcd_buffer);
	
	i2cdata = Read_A2D_8591();
	sprintf(ch0A2D8591+11, "%x ", i2cdata);
	print_lcd(1, ch0A2D8591);	
	out_D2A_8591(i2cdata);
	
	disp2Digit8574(i2cdata);
	
 	TIMER1->TISR.TIF = 1;    		// Write 1 to clear the interrupt flag 
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
	char adc_value[15] = "ADC0 Value:";
	while (ADC->ADSR.ADF == 0);	// A/D Conversion End Flag
	// A status flag that indicates the end of A/D conversion.
		
	ADC->ADSR.ADF = 1;					// This flag can be cleared by writing 1 to self
	sprintf(adc_value+11,"%d   ",ADC->ADDR[0].RSLT);
	print_lcd(2, adc_value);
	ADC->ADCR.ADST = 1;					// 1 = Conversion start 
	
 	TIMER0->TISR.TIF = 1;				// Write 1 to clear the interrupt flag 
	}
//----------------------------------------------------------------------ADC0
void InitADC0(void) {
	/* Step 1. GPIO initial */ 
	GPIOA->OFFD |= 0x00010000;		// Disable digital input path (when input is analog signal)
	SYS->GPAMFP.ADC0 = 1;					// Set ADC function 
				
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
	ADC->ADCHER.CHEN = 0x0001;		// 8-bits -> ch0
	
	/* Step 5. Enable ADC interrupt */
	//ADC->ADSR.ADF = 1;					// clear the A/D interrupt flags
	//ADC->ADCR.ADIE = 1;
	//NVIC_EnableIRQ(ADC_IRQn);
	
	/* Step 6. A/D Conversion Start */
	ADC->ADCR.ADST = 1;						
	// ADST will be cleared to 0 by hardware automatically 
	// at the ends of single mode and single cycle scan mode.
	}

//----------------------------------------------------------------------MAIN
int main(void) {
	UNLOCKREG();
    DrvSYS_Open(48000000);
	LOCKREG();	

	DrvSYS_SetClockDivider(E_SYS_HCLK_DIV, 0); 
	/* HCLK clock frequency = HCLK clock source / (HCLK_N + 1) */

	Initial_pannel();  // call initial pannel function
	clr_all_pannal();

	InitTIMER0();	
	InitTIMER1();
	InitADC0();	
	
	print_lcd(3, "no keyPad A0-A5 ");
	
	DrvGPIO_InitFunction(E_FUNC_I2C1);

	while (1) {
		__NOP();
		}	
	}
