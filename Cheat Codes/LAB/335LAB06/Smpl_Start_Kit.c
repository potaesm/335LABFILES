/*
	PWM6 -> GPE0 to vary the DCmotor speed
	DCmotor:
	- IN1:	E_IO_OUTPUT GPA12
	- EN:		E_IO_OUTPUT GPA13
	- IN2:	PWM6 GPE0
*/

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


// LAB335 WEEK 7 BY SUTHINAN & PONGPITH
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.              */
/* edited: MAR 2019: suthinan.musitmani@mail.kmutt.ac.th                    */
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include "NUC1xx.h"
#include "LCD_Driver.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvSYS.h"
#include "Driver\DrvI2C.h"
#include "scankey.h"

static uint16_t Timer1Counter = 0;
int state = 0;

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

// display an integer on four 7-segment LEDs
void disp2Digit8574_inDec(int16_t value) {
	int8_t digit;
	digit = value / 1000;
	//close_seven_segment();
	//show_seven_segment(3,digit);
	//DrvSYS_Delay(scanDelay);
		
	value = value - digit * 1000;
	digit = value / 100;
	Write_to_any8574(0x72,HEX2Disp(digit));
	//close_seven_segment();
	//show_seven_segment(2,digit);
	//DrvSYS_Delay(scanDelay);

	value = value - digit * 100;
	digit = value / 10;
	Write_to_any8574(0x70,HEX2Disp(digit));
	//close_seven_segment();
	//show_seven_segment(1,digit);
	//DrvSYS_Delay(scanDelay);

	value = value - digit * 10;
	digit = value;
	//close_seven_segment();
	//show_seven_segment(0,digit);
	//DrvSYS_Delay(scanDelay);
	}
void disp2Digit8574_inHex(int16_t value) {
	int8_t digit;
	digit = value / 256;
	//close_seven_segment();
	//show_seven_segment(3,digit);
	//DrvSYS_Delay(scanDelay);
		
	value = value - digit * 256;
	digit = value / 16;
	Write_to_any8574(0x72,HEX2Disp(digit));
	//close_seven_segment();
	//show_seven_segment(2,digit);
	//DrvSYS_Delay(scanDelay);

	value = value - digit * 16;
	digit = value;
	Write_to_any8574(0x70,HEX2Disp(digit));
	//close_seven_segment();
	//show_seven_segment(1,digit);
	//DrvSYS_Delay(scanDelay);

	value = value - digit;
	digit = value;
	//close_seven_segment();
	//show_seven_segment(0,digit);
	//DrvSYS_Delay(scanDelay);
	}
	
//----------------------------------------------------------------------PWM6
void InitPWM6(void) {
 	/* Step 1. GPIO initial */ 
	SYS->GPEMFP.PWM6 = 1;
				
	/* Step 2. Enable and Select PWM clock source*/		
	SYSCLK->APBCLK.PWM67_EN = 1;// Enable PWM clock
	SYSCLK->CLKSEL2.PWM67_S = 3;// Select 22.1184Mhz for PWM clock source

	PWMB->PPR.CP23 = 1;			// Prescaler 0~255, Setting 0 to stop output clock
	PWMB->CSR.CSR2 = 0;			// PWM clock = clock source/(Prescaler + 1)/divider
				         
	/* Step 3. Select PWM Operation mode */
	PWMB->PCR.CH2MOD = 1;		// 0:One-shot mode, 1:Auto-load mode
	// CNR and CMR will be auto-cleared after setting CH0MOD form 0 to 1.
	PWMB->CNR2 = 0xFFFF;
	PWMB->CMR2 = 0xFFFF;

	PWMB->PCR.CH2INV = 0;		// Inverter->0:off, 1:on
	PWMB->PCR.CH2EN = 1;		// PWM function->0:Disable, 1:Enable
 	PWMB->POE.PWM2 = 1;			// Output to pin->0:Diasble, 1:Enable
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
	
//----------------------------------------------------------------Timer1_IRQ for Timer 2
void TMR1_IRQHandler(void) {	// Timer0 interrupt subroutine
	char adc_value[15] = "ADC7 Value:";
	char lcd2_buffer[18] = "Timer1:";
	char lcd3_buffer[18] = "T2:0xHH,DDD";

	while (ADC->ADSR.ADF == 0);	// A/D Conversion End Flag
	// A status flag that indicates the end of A/D conversion.
		
	ADC->ADSR.ADF = 1;					// This flag can be cleared by writing 1 to self
	PWMB->CMR2 = ADC->ADDR[7].RSLT << 4;
	sprintf(adc_value+11,"%d   ", ADC->ADDR[7].RSLT);
	print_lcd(0, adc_value);
	ADC->ADCR.ADST = 1;					// 1 = Conversion start 

	Timer1Counter+=1;
	sprintf(lcd2_buffer+7, " %d", Timer1Counter);
	print_lcd(2, lcd2_buffer);	
	
	sprintf(lcd3_buffer, "T2:0x%x,%d   ", TIMER2->TDR, TIMER2->TDR);
	print_lcd(3, lcd3_buffer);
	
	if (state == 1) {
			disp2Digit8574_inHex(TIMER2->TDR);
		}
	if (state == 2) {
			disp2Digit8574_inDec(TIMER2->TDR);
		}
	
	TIMER2->TCSR.CRST = 1;			// Reset up counter
	TIMER2->TCSR.CEN = 1;				// Enable Timer
	
 	TIMER1->TISR.TIF = 1;				// Write 1 to clear the interrupt flag 
	}
	
//--------------------------------------------------------------------Timer2
void InitTIMER2(void) {	// event counting
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR2_S = 2;	// Select HCLK for event counting
	// 0 = 12 MHz, 1 = 32 kHz, 2 = HCLK, 7 = 22.1184 MHz
	SYSCLK->APBCLK.TMR2_EN = 1;	// Enable Timer clock source
	
	SYS->GPBMFP.TM2_SS01 = 1;	// Multiple Function Pin GPIOB Control Register
	//SYS->ALTMFP.PB9_S11 = 0;	// Alternative Multiple Function Pin Control Register
	
	TIMER2->TEXCON.TX_PHASE = 1;// A rising edge of external co in will be counted.
	TIMER2->TEXCON.TCDB = 1;		// Enable De-bounce
	 
	TIMER2->TCSR.CTB = 1; 			//  Enable counter mode

	/* Step 2. Select Operation mode */	
	// TIMER1->TCSR.MODE = 1;	// 1 -> Select periodic mode
	// 0 = One shot, 1 = Periodic, 2 = Toggle, 3 = continuous counting mode
	
	/* Step 3. Select Time out period 
	= (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER2->TCSR.PRESCALE = 0;	// Set Prescale [0~255]
	// TIMER1->TCMPR = 1000000;		// Set TCMPR [0~16777215]
	// (1/12000000)*(11+1)*(1000000)= 1 sec or 1 Hz	
	
	/* Step 5. Enable Timer module */
	TIMER2->TCSR.CRST = 1;			// Reset up counter
	TIMER2->TCSR.CEN = 1;				// Enable Timer
	
	TIMER2->TCSR.TDR_EN = 1;		// Enable TDR function	
	}
	
//----------------------------------------------------------------------GPIO
void InitGPIO() {
	DrvGPIO_Open(E_GPA,12,E_IO_OUTPUT);	// IN1
	DrvGPIO_Open(E_GPA,13,E_IO_OUTPUT);	// EN
	//DrvGPIO_Open(E_GPA,14,E_IO_OUTPUT);	// IN2
	DrvGPIO_ClrBit(E_GPA,12);
	DrvGPIO_ClrBit(E_GPA,13);
	//DrvGPIO_ClrBit(E_GPA,14);
	}
	
//----------------------------------------------------------------------MAIN
int32_t main (void) {
	unsigned char key;
	state = 2;
	// Enable 12Mhz and set HCLK->12Mhz
	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1;
	SYSCLK->CLKSEL0.HCLK_S = 0;
	LOCKREG();

	InitGPIO();
	// right turn
	DrvGPIO_SetBit(E_GPA,13);	// EN
	DrvGPIO_ClrBit(E_GPA,12);	// IN1
	
	InitPWM6();				// IN2
	InitADC7();				// to vary the DCmotor speed
	
	//InitTIMER0();		// event counting
	InitTIMER2();			// event counting
	InitTIMER1();			// read ADC7
	
	Initial_pannel();	// call initial panel function
	clr_all_pannal();

	DrvGPIO_InitFunction(E_FUNC_I2C1);

	while (1) {
		key = Scankey();
		if (key == 1) {
				state = 1;
			}
		if (key == 2) {
				state = 2;
			}
		if (key == 3) {
				DrvGPIO_ClrBit(E_GPA,13);	// EN Stop Motor
			}
		if (key == 4) {
				DrvGPIO_SetBit(E_GPA,13);	// EN Start Motor
			}
		}
	}
