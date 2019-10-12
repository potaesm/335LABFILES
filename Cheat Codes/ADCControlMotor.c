/* 
	Timer0 (GPB8) -> event counting

	PWM0 -> GPA12 to vary the DCmotor speed right
	PWM2 -> GPA14 to vary the DCmotor speed left
	
	Timer1: read ADC7 every 1 s. 

	DCmotor:
	RIHGT
	- IN1:	PWM0 GPA12
	- EN:		E_IO_OUTPUT GPA13
	- IN2:	E_IO_OUTPUT GPA14
	LEFT
	- IN1:	E_IO_OUTPUT GPA12
	- EN:		E_IO_OUTPUT GPA13
	- IN2:	PWM2 GPA14
*/

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.              */
/* edited: APRIL 2019: suthinan.musitmani@mail.kmutt.ac.th                  */
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include "NUC1xx.h"
#include "LCD_Driver.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvSYS.h"
#include "Driver\DrvI2C.h"
#include "Seven_Segment.h"

#define scanDelay 4000
#define DELAY300ms	300000 		// The maximal delay time is 335000 us.
#define DELAY100ms	100000 		// The maximal delay time is 335000 us.
#define DELAY50ms	50000 		// The maximal delay time is 335000 us.

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

static uint16_t Timer1Counter = 0;
uint8_t int1_toggler = 0;

unsigned char SEG_BUF_HEX[16]={SEG_N0, SEG_N1, SEG_N2, SEG_N3, SEG_N4, SEG_N5, SEG_N6, SEG_N7, SEG_N8, SEG_N9, SEG_Na, SEG_Nb, SEG_Nc, SEG_Nd, SEG_Ne, SEG_Nf};

//--------------------------------------------------------------------------------------7SEG-LB Show Digit in Hex
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

//--------------------------------------------------------------------------------------7SEG-LB Display in Dec
void seg_display(int16_t value) {
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

//--------------------------------------------------------------------------------------7SEG-LB Display in Hex
void seg_display_Hex(int16_t value) {
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

		value = value - digit * 10;
		digit = value;
		close_seven_segment();
		show_seven_segment(0,digit);
		DrvSYS_Delay(scanDelay);
}

//--------------------------------------------------------------------------------------PWM0
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
	
//--------------------------------------------------------------------------------------PWM2Gen GPA14
void InitPWM2(void) {
 	/* Step 1. GPIO initial */ 
	SYS->GPAMFP.PWM2_AD15 = 1;		// System Manager Control Registers
	
	/* Step 2. Enable and Select PWM clock source*/		
	SYSCLK->APBCLK.PWM23_EN = 1;	// Enable PWM clock
	SYSCLK->CLKSEL1.PWM23_S = 3;	// Select 12Mhz for PWM clock source
	// 0:12MHz, 1:32.768 kHz, 2:HCLK, 3:22.1184 MHz 
	PWMA->PPR.CP23 = 1;		// Prescaler 0~255, Setting 0 to stop output clock
	PWMA->CSR.CSR2 = 0;		// clock divider ->	0:/2, 1:/4, 2:/8, 3:/16, 4:/1
	// PWM frequency = PWMxy_CLK/[(prescale+1)*(clock divider)*(CNR+1)]
	// Ex:= 12M/[(1+1)*(1)*(8191)] = 732.5 Hz -> T = 1365 micros. ~ 682 micros + 682 micros
	
	/* Step 3. Select PWM Operation mode */
	PWMA->PCR.CH2MOD = 1;	// 0:One-shot mode, 1:Auto-load mode
	// CNR and CMR will be auto-cleared after setting CH0MOD form 0 to 1.
	PWMA->CNR2 = 0xFFFF;	// 0x1FFF = 8191
	PWMA->CMR2 = 0xFFFF;
	// CMR < CNR:	PWM low width = (CNR-CMR) unit [one PWM clock cycle]
	//						PWM high width = (CMR+1) unit [T in ms / CNR in Dec]
	PWMA->PCR.CH2INV = 0;	// Inverter -> 0:off, 1:on
	PWMA->PCR.CH2EN = 1;	// PWM function -> 0:Disable, 1:Enable
 	PWMA->POE.PWM2 = 1;		// Output to pin -> 0:Diasble, 1:Enable
	}
	
//------------------------------------------------------------------------------------------ADC1 GPA1
void InitADC1(void) {
	/* Step 1. GPIO initial */ 
	GPIOA->OFFD |= 0x00020000;		// Disable digital input path (when input is analog signal)
	SYS->GPAMFP.ADC1_AD12 = 1;					// Set ADC function 
				
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
	ADC->ADCHER.CHEN = 0x2;		// 8-bits -> ch1
	
	/* Step 5. Enable ADC interrupt */
	//ADC->ADSR.ADF = 1;					// clear the A/D interrupt flags
	//ADC->ADCR.ADIE = 1;
	//NVIC_EnableIRQ(ADC_IRQn);
	
	/* Step 6. A/D Conversion Start */
	ADC->ADCR.ADST = 1;						
	// ADST will be cleared to 0 by hardware automatically 
	// at the ends of single mode and single cycle scan mode.
	}
	
//--------------------------------------------------------------------------------------ADC7
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
	
//------------------------------------------------------------------------------------------Convert Hex number to use with 7SEG-I2C (PCF8574)
uint8_t HEX2Disp(uint8_t hexNum) {
	static const uint8_t lookUp[16] = {
		0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8,
		0x80, 0x90, 0x88, 0x83, 0xC6, 0xA1, 0x86, 0x8E
		};
	uint8_t hexDisp = lookUp[hexNum];
		
	return hexDisp;
	}
//------------------------------------------------------------------------------------------Write to 7SEG-I2C (PCF8574)
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

//------------------------------------------------------------------------------------------Display on 7SEG-I2C (PCF8574) in Hex
void disp2Digit8574(uint8_t data) {
	Write_to_any8574(0x72,HEX2Disp(data >> 4));	// Left
	Write_to_any8574(0x70,HEX2Disp(data &= 0x0F)); //Right
	}

//------------------------------------------------------------------------------------------Display on 7SEG-I2C (PCF8574) in Dec
void disp2Digit8574_Dec(int16_t value) {
		int8_t digit;
		digit = value / 1000;
			
		value = value - digit * 1000;
		digit = value / 100;

		value = value - digit * 100;
		digit = value / 10;
		Write_to_any8574(0x72,HEX2Disp(digit));	// Left

		value = value - digit * 10;
		digit = value;
		Write_to_any8574(0x70,HEX2Disp(digit)); //Right
}
	
//------------------------------------------------------------------------------------------Digital to Analog (PCF8591)
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
	
//------------------------------------------------------------------------------------------Analog to Digital (PCF8591) Channel 0
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
	
//--------------------------------------------------------------------------------------Timer0
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
	
//--------------------------------------------------------------------------------------Timer1
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
	
//--------------------------------------------------------------------------------------Timer1_IRQ
void TMR1_IRQHandler(void) {	// Timer0 interrupt subroutine
	uint8_t i2cdata = 0;
	char adc7_value[15] = "ADC7 Value:";
	char adc1_value[15] = "ADC1 Value:";
	char lcd2_buffer[18] = "Timer1:";
	char lcd3_buffer[18] = "T0_TDR:";
	
	i2cdata = Read_A2D_8591();
	out_D2A_8591(i2cdata);
	disp2Digit8574(i2cdata);
	//disp2Digit8574_Dec(i2cdata);

	while (ADC->ADSR.ADF == 0);	// A/D Conversion End Flag
	// A status flag that indicates the end of A/D conversion.
		
	ADC->ADSR.ADF = 1;					// This flag can be cleared by writing 1 to self
	
	//-----------------------------------------------------------------------------------------CMR0 for PWM0 (Turn Rgiht) and CMR2 for PWM2 (Turn Left)
	PWMA->CMR2 = ADC->ADDR[7].RSLT << 4;
	
	sprintf(adc7_value+11,"%d   ", ADC->ADDR[7].RSLT);
	print_lcd(1, adc7_value);
	sprintf(adc1_value+11,"%d   ", ADC->ADDR[1].RSLT);
	print_lcd(0, adc1_value);
	//disp2Digit8574_Dec(ADC->ADDR[1].RSLT);

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
	
//------------------------------------------------------------------------------------------ADC IRQ
void ADC_IRQHandler(void) {
	
	}
	
//-----------------------------------------------------------------------------------------Init Right Turn Motor
void InitGPIOR() {
	DrvGPIO_Open(E_GPA,12,E_IO_OUTPUT);	// IN1
	DrvGPIO_Open(E_GPA,13,E_IO_OUTPUT);	// EN
	DrvGPIO_Open(E_GPA,14,E_IO_OUTPUT);	// IN2
	DrvGPIO_SetBit(E_GPA,12); // IN1
	DrvGPIO_SetBit(E_GPA,13);	// EN
	DrvGPIO_ClrBit(E_GPA,14);	// IN2
	}
	
//-----------------------------------------------------------------------------------------Init Left Turn Motor
void InitGPIOL() {
	DrvGPIO_Open(E_GPA,12,E_IO_OUTPUT);	// IN1 PWM0
	DrvGPIO_Open(E_GPA,13,E_IO_OUTPUT);	// EN
	DrvGPIO_Open(E_GPA,14,E_IO_OUTPUT);	// IN2 PWM2
	DrvGPIO_ClrBit(E_GPA,12); // IN1 PWM0
	DrvGPIO_SetBit(E_GPA,13);	// EN
	DrvGPIO_SetBit(E_GPA,14);	// IN2 PWM2
	}
	
//-----------------------------------------------------------------------------------------INT1 Callback
void EINT1Callback(void) {
	if (int1_toggler == 0) {
		int1_toggler = 1;
		}
		else {
			int1_toggler = 0;
			}
	DrvSYS_Delay(DELAY300ms);		// delay
	}
	
//-----------------------------------------------------------------------------------------MAIN
int32_t main (void) {
	uint8_t state = 0;
	// Enable 12Mhz and set HCLK->12Mhz
	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1;
	SYSCLK->CLKSEL0.HCLK_S = 0;
	LOCKREG();
	
	/* Configure general GPIO interrupt */
	DrvGPIO_Open(E_GPB, 15, E_IO_INPUT);

	/* Configure external interrupt */
	DrvGPIO_EnableEINT1(E_IO_BOTH_EDGE, E_MODE_EDGE, EINT1Callback);

	Initial_pannel();	// call initial panel function
	clr_all_pannal();
	
	InitGPIOL();
	
	
	//InitPWM0();				// IN1 Control Right Turn Motor Speed
	InitPWM2();				// IN2  Control Left Turn Motor Speed
	
	InitTIMER0();			// event counting
	InitTIMER1();			// read ADC7
	
	DrvGPIO_InitFunction(E_FUNC_I2C1);
					 
	while (1) {
		seg_display(ADC->ADDR[1].RSLT);
		if (state == 0) {
				InitADC1();
				state = 1;
			}
			else {
					InitADC7();				// to vary the DCmotor speed
					state = 0;
				}
		if (int1_toggler == 0) {
			GPC_12 = 1;
				DrvGPIO_SetBit(E_GPA,13);	// EN
			}
			else {
				GPC_12 = 0;
					DrvGPIO_ClrBit(E_GPA,13);	// EN
				}
		}
	}
