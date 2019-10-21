/* 
	Onewire : GPA6
	Timer0 (GPB8) -> event counting

	PWM3 -> GPA15 to vary the DCmotor speed right
	
	Timer1: read ADC0 every 1 s.
	Pin5 of PCF8574 Connect to ADC0 (GPA0)

	DCmotor: RIHGT
	- IN1:	PWM3 GPA15
	- EN:		E_IO_OUTPUT GPA13
	- IN2:	E_IO_OUTPUT GPA14
*/

// LAB335 WEEK 10 BY SUTHINAN & PONGPITH
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.              */
/* edited: MAR 2019: suthinan.musitmani@mail.kmutt.ac.th                    */

#include <stdio.h>																											 
#include "NUC1xx.h"
#include "LCD_Driver.h"
#include "DrvSYS.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvI2C.h"
#include "Seven_Segment.h"
#define DELAY300ms	300000 		// The maximal delay time is 335000 us.

#define scanDelay 4000



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

int8_t ds1820Temp, ds1820Temp7SEG;
static uint16_t Timer3Counter=0;
unsigned char SEG_BUF_HEX[16]={SEG_N0, SEG_N1, SEG_N2, SEG_N3, SEG_N4, SEG_N5, SEG_N6, SEG_N7, SEG_N8, SEG_N9, SEG_Na, SEG_Nb, SEG_Nc, SEG_Nd, SEG_Ne, SEG_Nf}; 

//------------------------------------------------------------------------------------------On Board 7SEG Hex Display
void show_seven_segment_Hex(unsigned char no, unsigned char number)
{
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
		//show_seven_segment_Hex(3,digit);
		DrvSYS_Delay(scanDelay);
			
		value = value - digit * 256;
		digit = value / 16;
		close_seven_segment();
		show_seven_segment_Hex(1,digit);
		DrvSYS_Delay(scanDelay);

		value = value - digit * 16;
		digit = value;
		close_seven_segment();
		show_seven_segment_Hex(0,digit);
		DrvSYS_Delay(scanDelay);

		value = value - digit * 10;
		digit = value;
		close_seven_segment();
		//show_seven_segment(0,digit);
		DrvSYS_Delay(scanDelay);
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
	
//-----------------------------------------------------------------------------------------Digital to Analog (PCF8591)
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
	
//-----------------------------------------------------------------------------------------Analog to Digital (PCF8591) Channel 0
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
	
//-----------------------------------------------------------------------------------------OneWireReadByteTemperature
int8_t OneWireReadByteTemperature(void) {
	int8_t i;	
	int8_t dataByte = 0xCC; // skip ROM
	
	GPIOA->DOUT &= 0xFFBF;	// Master send Reset
	DrvSYS_Delay(500);
	GPIOA->DOUT |= 0x0040;
	DrvSYS_Delay(200);			// wait for presence pulse
	
	for (i=0;i<8;i++) {			// skip ROM
		if ((dataByte&0x01 == 0x01)) {
			GPIOA->DOUT &= 0xFFBF;	// send '1'
			DrvSYS_Delay(3);				// low > 1 microsec.
			GPIOA->DOUT |= 0x0040;
			DrvSYS_Delay(60);	
			} else {
			GPIOA->DOUT &= 0xFFBF;	// send '0'
			DrvSYS_Delay(60);				// low > 60 microsec.
			GPIOA->DOUT |= 0x0040;
			DrvSYS_Delay(2);				
			}
		dataByte >>= 1;
		}
	
	dataByte = 0xBE;				// ReadScratchpad
	for (i=0;i<8;i++) {
		if ((dataByte&0x01 == 0x01)) {
			GPIOA->DOUT &= 0xFFBF;	// send '1'
			DrvSYS_Delay(3);				// low > 1 microsec.
			GPIOA->DOUT |= 0x0040;
			DrvSYS_Delay(60);	
			} else {
			GPIOA->DOUT &= 0xFFBF;	// send '0'
			DrvSYS_Delay(60);				// low > 60 microsec.
			GPIOA->DOUT |= 0x0040;
			DrvSYS_Delay(2);				
			}
		dataByte >>= 1;
		}

	// read 8 bits (byte0 scratchpad)
	DrvSYS_Delay(100);
	for (i=0;i<8;i++) {
		GPIOA->DOUT &= 0xFFBF;	// 
		DrvSYS_Delay(2);				// low > 1 microsec.
		GPIOA->DOUT |= 0x0040;
		// Read
		DrvSYS_Delay(12);
		if ((GPIOA->PIN &= 0x0040) == 0x0040) {
			dataByte >>= 1;
			dataByte |= 0x80;
		} else {
			dataByte >>= 1;
			dataByte &= 0x7F;			
		}
		DrvSYS_Delay(60);			
		}
	dataByte >>= 1;
	return dataByte;
	}
	
//-----------------------------------------------------------------------------------------OneWireTxSkipROMConvert
void OneWireTxSkipROMConvert(void) {
	int8_t i;	
	uint8_t dataByte = 0xCC; // skip ROM
	
	//GPIOE->DOUT &= 0xFEFF;	// Master send Reset
	GPIOA->DOUT &= 0xFFBF;
	DrvSYS_Delay(500);
	GPIOA->DOUT |= 0x0040;
	DrvSYS_Delay(200);		
	
	for (i=0;i<8;i++) {		// skip ROM
		if ((dataByte&0x01 == 0x01)) {
			GPIOA->DOUT &= 0xFFBF;	// send '1'
			DrvSYS_Delay(3);				// low > 1 microsec.
			GPIOA->DOUT |= 0x0040;
			DrvSYS_Delay(60);	
			} else {
			GPIOA->DOUT &= 0xFFBF;	// send '0'
			DrvSYS_Delay(60);				// low > 60 microsec.
			GPIOA->DOUT |= 0x0040;
			DrvSYS_Delay(2);				
			}
		dataByte >>= 1;
	}
	
	dataByte = 0x44;	// convert Temperature
	for (i=0;i<8;i++) {
		if ((dataByte&0x01 == 0x01)) {
			GPIOA->DOUT &= 0xFFBF;	// send '1'
			DrvSYS_Delay(3);				// low > 1 microsec.
			GPIOA->DOUT |= 0x0040;
			DrvSYS_Delay(60);	
			} else {
			GPIOA->DOUT &= 0xFFBF;	// send '0'
			DrvSYS_Delay(60);				// low > 60 microsec.
			GPIOA->DOUT |= 0x0040;
			DrvSYS_Delay(2);				
			}
		dataByte >>= 1;
	}	
}

//--------------------------------------------------------------------------------------Timer0 For Event Counting
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

//-----------------------------------------------------------------------------------------Timer3
void InitTIMER3(void)
{
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR3_S = 0;	// Select 12Mhz for Timer0 clock source
	// 0 = 12 MHz, 1 = 32 kHz, 2 = HCLK, 7 = 22.1184 MHz
  SYSCLK->APBCLK.TMR3_EN = 1;	// Enable Timer0 clock source

	/* Step 2. Select Operation mode */	
	TIMER3->TCSR.MODE = 1;				// 1 -> Select periodic mode
	// 0 = One shot, 1 = Periodic, 2 = Toggle, 3 = continuous counting mode
	
	/* Step 3. Select Time out period 
	= (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER3->TCSR.PRESCALE = 11;	// Set Prescale [0~255]
	TIMER3->TCMPR = 1000000;		// Set TCMPR [0~16777215]
	// (1/12000000)*(11+1)*(1000000)= 1 sec

	/* Step 4. Enable interrupt */
	TIMER3->TCSR.IE = 1;
	TIMER3->TISR.TIF = 1;				// Write 1 to clear the interrupt flag 		
	NVIC_EnableIRQ(TMR3_IRQn);	// Enable Timer0 Interrupt

	/* Step 5. Enable Timer module */
	TIMER3->TCSR.CRST = 1;			// Reset up counter
	TIMER3->TCSR.CEN = 1;				// Enable Timer0
}

//-----------------------------------------------------------------------------------------Timer3_IRQ
void TMR3_IRQHandler(void) 		// Timer0 interrupt subroutine 
{
	//uint16_t Timer3Counter = 0;
	uint8_t i2cdata = 0;
	char adc0_value[15] = "ADC0 Value:";
	char lcd1_buffer[18] = "T =    C";
	char lcd2_buffer[18] = "Timer3:";
	char lcd3_buffer[18] = "T3_TDR:";
	
	i2cdata = Read_A2D_8591();
	out_D2A_8591(i2cdata);
	disp2Digit8574(i2cdata);
	
	sprintf(lcd2_buffer+7," %d", Timer3Counter);
	print_lcd(2, lcd2_buffer);
	Timer3Counter++;

	// to initiate a temperature measurement and A-to-D conversion
	OneWireTxSkipROMConvert(); 
	DrvSYS_Delay(100);
	ds1820Temp = OneWireReadByteTemperature();
	ds1820Temp7SEG = ds1820Temp;
	sprintf(lcd1_buffer+4,"%d C 0x%X", ds1820Temp, ds1820Temp);
	print_lcd(1, lcd1_buffer);
	
	while (ADC->ADSR.ADF == 0);	// A/D Conversion End Flag
	// A status flag that indicates the end of A/D conversion.
		
	ADC->ADSR.ADF = 1;					// This flag can be cleared by writing 1 to self
	
//-----------------------------------------------------------------------------------------CMR0 for PWM0 (Turn Rgiht) and CMR2 for PWM2 (Turn Left)
	PWMA->CMR3 = ADC->ADDR[0].RSLT << 4;
	
	sprintf(adc0_value+11,"%X   ", ADC->ADDR[0].RSLT);
	print_lcd(0, adc0_value);

	ADC->ADCR.ADST = 1;					// 1 = Conversion start
	
	sprintf(lcd3_buffer+7, " %d       ", TIMER0->TDR);
	print_lcd(3, lcd3_buffer);
	
	TIMER0->TCSR.CRST = 1;			// Reset up counter
	TIMER0->TCSR.CEN = 1;				// Enable Timer
	
 	TIMER3->TISR.TIF = 1;				// Write 1 to clear the interrupt flag 
}

//-----------------------------------------------------------------------------------------PWM3Gen GPA15
void InitPWM3(void) {
 	/* Step 1. GPIO initial */ 
	SYS->GPAMFP.PWM3_I2SMCLK = 1;		// System Manager Control Registers
	
	/* Step 2. Enable and Select PWM clock source*/		
	SYSCLK->APBCLK.PWM23_EN = 1;	// Enable PWM clock
	SYSCLK->CLKSEL1.PWM23_S = 3;	// Select 12MHz for PWM clock source
	// 0:12MHz, 1:32.768 kHz, 2:HCLK, 3:22.1184 MHz 
	PWMA->PPR.CP23 = 1;	// Prescaler 0~255, Setting 0 to stop output clock
	PWMA->CSR.CSR3 = 0;		// clock divider ->	0:/2, 1:/4, 2:/8, 3:/16, 4:/1
	// PWM frequency = PWMxy_CLK/[(prescale+1)*(clock divider)*(CNR+1)]
	// Ex:= 22.1184M/[(21+1)*(16)*(2^16)] = 0.95880 Hz -> T = 1.042963 s ~ 521 ms + 521 ms

	/* Step 3. Select PWM Operation mode */
	PWMA->PCR.CH3MOD = 1;		// 0:One-shot mode, 1:Auto-load mode
	// CNR and CMR will be auto-cleared after setting CH0MOD form 0 to 1.
	PWMA->CNR3 = 0xFFFF;
	PWMA->CMR3 = 0xFFFF;
	// CMR < CNR:	PWM low width = (CNR-CMR) unit [one PWM clock cycle]
	//						PWM high width = (CMR+1) unit [T in ms / CNR]

	PWMA->PCR.CH3INV = 0;	// Inverter -> 0:off, 1:on
	PWMA->PCR.CH3EN = 1;	// PWM function -> 0:Disable, 1:Enable
 	PWMA->POE.PWM3 = 1;		// Output to pin -> 0:Diasble, 1:Enable
	}
	
//-----------------------------------------------------------------------------------------ADC0 GPA0
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
	ADC->ADCHER.CHEN = 0x1;		// 8-bits -> ch0
	
	/* Step 5. Enable ADC interrupt */
	//ADC->ADSR.ADF = 1;					// clear the A/D interrupt flags
	//ADC->ADCR.ADIE = 1;
	//NVIC_EnableIRQ(ADC_IRQn);
	
	/* Step x. compare setup */
	//ADC->ADCMPR[0].CMPD = 0x250;	// Comparison Data 0x250
	//ADC->ADCMPR[0].CMPCH = 0;			// Compare Channel Selection
	//ADC->ADCMPR[0].CMPCOND = 0;		// Compare Condition
	// 1: greater or equal, 0: less than
	//ADC->ADCMPR[0].CMPIE = 1;			// Compare Interrupt Enable
	//ADC->ADCMPR[0].CMPEN = 1;			// Compare Enable
	//NVIC_EnableIRQ(ADC_IRQn);
	
	/* Step 6. A/D Conversion Start */
	ADC->ADCR.ADST = 1;						
	// ADST will be cleared to 0 by hardware automatically 
	// at the ends of single mode and single cycle scan mode.
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
	
//-----------------------------------------------------------------------------------------MAIN
int32_t main (void) {
	
	UNLOCKREG();
	DrvSYS_Open(48000000);
	LOCKREG();

	/* Configure general GPIO interrupt */
	DrvGPIO_Open(E_GPB, 15, E_IO_INPUT);
	
	InitGPIOR();
	
	Initial_pannel();  //call initial pannel function
	clr_all_pannal();
	print_lcd(0, "DS1820 Onewire");
	
	GPC_12 = 0;
	GPC_14 = 0;
	
	//DrvGPIO_Open(E_GPE, 8, E_IO_QUASI);
	DrvGPIO_Open(E_GPA, 6, E_IO_QUASI);
	InitTIMER0();
	InitTIMER3();
	InitPWM3();
	InitADC0();
	
	DrvGPIO_InitFunction(E_FUNC_I2C1);
	
	while (1) {
			seg_display_Hex(ds1820Temp7SEG);
		}
	}
