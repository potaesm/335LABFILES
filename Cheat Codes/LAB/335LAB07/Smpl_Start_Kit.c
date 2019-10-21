// step1: a] capture0 -> capture5(E5)
//				b] add T0 Toggle
// toggle every 1 second (50% duty cycle) -> GPB8

// step2: Display LCD line0 with clock
//				c] key 'Int1' Display Second. on 7SEG-I2C
//				d] key '1' turn off 7SEG-I2C


// Nu_LB-002:	L02_03_PWM0Capture.c

//	PWM0 -> GPA12 - capture input
//	PWM1 -> GPA13 High level: 262 msec, Low level: 786 msec
//	PWM2 -> GPA14 High level: 682 usec, Low level: 682 usec(microsecond)


// LAB335 WEEK 8 BY SUTHINAN & PONGPITH
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.              */
/* edited: MAR 2019: suthinan.musitmani@mail.kmutt.ac.th                    */
/*--------------------------------------------------------------------------*/

#include <stdio.h>																											 
#include "NUC1xx.h"
#include "LCD_Driver.h"
#include "scankey.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvSYS.h"
#include "Driver\DrvI2C.h"

#define	PWM_CNR	0xFFFF
#define DELAY300ms	300000

static uint16_t Timer0Counter=0;
static uint16_t Timer3Counter=0;

static uint16_t state = 0;

uint16_t	CaptureCounter = 0;
uint32_t	CaptureValue[2];

//------------------------------------------------------------------Int1
void EINT1Callback(void) {
	//DrvSYS_Delay(DELAY300ms);		// delay
	state = 1;
	}
//-------------------------------------------------------------------HEX2Disp
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
//------------------------------------------------------------------RTC
void set_TLR (int32_t a,int32_t b,int32_t c,int32_t d,int32_t e,int32_t f) {
	outpw(&RTC->TLR, a<<20|b<<16|c<<12|d<<8|e<<4|f)	 ;
	}
void set_CLR (int32_t a,int32_t b,int32_t c,int32_t d,int32_t e,int32_t f) {
	outpw(&RTC->CLR, a<<20|b<<16|c<<12|d<<8|e<<4|f)	 ;
	}
void set_TAR(int32_t a,int32_t b,int32_t c,int32_t d,int32_t e,int32_t f) {
	outpw(&RTC->TAR, a<<20|b<<16|c<<12|d<<8|e<<4|f)	;
	}
void set_CAR (int32_t a,int32_t b,int32_t c,int32_t d,int32_t e,int32_t f) {
	outpw(&RTC->CAR, a<<20|b<<16|c<<12|d<<8|e<<4|f)	;
	}
void START_RTC(void) {
  	while (1) {
			RTC->INIR = 0xa5eb1357;	// to make RTC leaving reset state
			if (inpw(&RTC->INIR) == 1)
				break;
			}
  	while (1) {
  		RTC->AER.AER = 0xA965;	// RTC read/write password to enable access
    	if (inpw(&RTC->AER) & 0x10000)	// AER bit
				break;
			}
	}
void InitRTC(void) {
	UNLOCKREG();
	/* Step 1. Enable and Select RTC clock source */     
	SYSCLK->PWRCON.XTL32K_EN = 1;	// Enable 32Khz for RTC clock source
	SYSCLK->APBCLK.RTC_EN = 1;		// Enable RTC clock source	

	/* Step 2. Initiate and unlock RTC module */
	START_RTC();

	/* Step 3. Initiate Time and Calendar  setting */
	RTC->TSSR.HR24_HR12 = 1;			// Set 24hour mode
	// Set time and calendar, Calendar YYYY/MM/DD, Time 09:40:00
	// Set time and calendar, Calendar 2015/04/01, Time 09:40:00
	set_CLR(1,5,0,4,0,1);
	set_TLR(0,9,4,0,0,0);

	/* Step 4. Set alarm interrupt */
	// Set time and calendar, Calendar 2015/04/01, Time 09:40:20
	set_CAR(1,5,0,4,0,1);
	set_TAR(0,9,4,0,2,0);	
	// Enable interrupt
	RTC->RIER.AIER = 1;					// Alarm Interrupt Enable
	RTC->RIER.TIER = 1;					// Time Tick Interrupt Enable
	NVIC_EnableIRQ(RTC_IRQn);
	}
//-------------------------------------------------------------------RTC_IRQ
void RTC_IRQHandler(void) { 	// default every 1 s.
	uint32_t clock;
	uint32_t date;
	char lcd_line0[15] = "Clock:";
	char lcd_line1[15] = "Date:20";
	
	/* tick */
	if (inpw(&RTC->RIIR) & 0x2) {	// TIF = 1?
		clock = inpw(&RTC->TLR) & 0xFFFFFF;
		sprintf(lcd_line0+6, "%02x", (clock >> 16) & 0xFF);
		sprintf(lcd_line0+9, "%02x", (clock >> 8) & 0xFF);
		sprintf(lcd_line0+12, "%02x", (clock & 0xFF));
		lcd_line0[8] = ':';
		lcd_line0[11] = ':';
		Show_Word(0,13, ' ');
		print_lcd(0, lcd_line0);
		
		if (state == 1){
			disp2Digit8574(clock & 0xFF);
			}
		if (state == 0){
			Write_to_any8574(0x72,0xFF);	
			Write_to_any8574(0x70,0xFF);
			}
		
		//date = inpw(&RTC->CLR) & 0xFFFFFF;
		//sprintf(lcd_line1+7, "%02x", (date >> 16) & 0xFF);
		//sprintf(lcd_line1+10, "%02x", (date >> 8) & 0xFF);
		//sprintf(lcd_line1+13, "%02x", date & 0xFF);
		//lcd_line1[9] = '/';
		//lcd_line1[12] = '/';
		//Show_Word(1, 13, ' ');
		//print_lcd(1, lcd_line1);
		
		outpw(&RTC->RIIR, 2);	// clear RTC Time Tick Interrupt Flag
	}
	
	/* alarm */
	/*if (inpw(&RTC->RIIR) & 0x1) {	// AIF = 1?
		GPIOC->DOUT &= 0xFF;				// LED5-8 on
		outpw(&RTC->RIIR, 1);				// clear RTC Alarm Interrupt Flag
	}*/
}
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
//------------------------------------------------------------------Capture1
void InitCapture1(void) {
 	/* Step 1. GPIO initial */ 
	SYS->GPAMFP.PWM1_AD14 = 1;
	
	/* Step 2. Enable and Select PWM clock source*/		
	SYSCLK->APBCLK.PWM01_EN = 1; // Enable PWM clock
	SYSCLK->CLKSEL1.PWM01_S = 0; // Select 12Mhz for PWM clock source
	// 0:12MHz, 1:32.768 kHz, 2:HCLK, 3:22.1184 MHz
	
	PWMA->PPR.CP01 = 11;		// Prescaler 0~255, Setting 0 to stop output clock
	PWMA->CSR.CSR1 = 4;			// clock divider -> 0:/2, 1:/4, 2:/8, 3:/16, 4:/1
									         
	/* Step 3. Select PWM Operation mode */
	PWMA->PCR.CH1MOD = 1;		// 0:One-shot mode, 1:Auto-load mode
	//CNR and CMR will be auto-cleared after setting CH0MOD from 0 to 1.
	PWMA->CNR1 = PWM_CNR;			// Set Reload register
	PWMA->CAPENR = 2;				// Enable Capture function pin
	PWMA->CCR0.CAPCH1EN = 1;// Enable Capture function

	/* Step 4. Set PWM Interrupt */
	PWMA->CCR0.CRL_IE1 = 1;	// Enable Capture rising edge interrupt
	PWMA->CCR0.CFL_IE1 = 1;	// Enable Capture falling edge interrupt
	PWMA->PIER.PWMIE1 = 1;	// Enable PWM interrupt for down-counter equal zero.
	NVIC_EnableIRQ(PWMA_IRQn);  // Enable PWM inturrupt

	/* Step 5. Enable PWM down counter*/
	PWMA->PCR.CH0EN = 1;		// Enable PWM down counter
	}
//------------------------------------------------------------------Capture5
void InitCapture5(void) {
 	/* Step 1. GPIO initial */ 
	SYS->GPEMFP.PWM5 = 1;
	
	/* Step 2. Enable and Select PWM clock source*/		
	SYSCLK->APBCLK.PWM45_EN = 1; // Enable PWM clock
	SYSCLK->CLKSEL2.PWM45_S = 0; // Select 12Mhz for PWM clock source
	// 0:12MHz, 1:32.768 kHz, 2:HCLK, 3:22.1184 MHz
	
	PWMB->PPR.CP01 = 11;		// Prescaler 0~255, Setting 0 to stop output clock
	PWMB->CSR.CSR1 = 4;			// clock divider -> 0:/2, 1:/4, 2:/8, 3:/16, 4:/1
									         
	/* Step 3. Select PWM Operation mode */
	PWMB->PCR.CH1MOD = 1;		// 0:One-shot mode, 1:Auto-load mode
	//CNR and CMR will be auto-cleared after setting CH0MOD from 0 to 1.
	PWMB->CNR1 = PWM_CNR;			// Set Reload register
	PWMB->CAPENR = 2;				// Enable Capture function pin
	PWMB->CCR0.CAPCH1EN = 1;// Enable Capture function

	/* Step 4. Set PWM Interrupt */
	PWMB->CCR0.CRL_IE1 = 1;	// Enable Capture rising edge interrupt
	PWMB->CCR0.CFL_IE1 = 1;	// Enable Capture falling edge interrupt
	PWMB->PIER.PWMIE1 = 1;	// Enable PWM interrupt for down-counter equal zero.
	NVIC_EnableIRQ(PWMB_IRQn);  // Enable PWM inturrupt

	/* Step 5. Enable PWM down counter*/
	PWMB->PCR.CH1EN = 1;		// Enable PWM down counter
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
//------------------------------------------------------------------PWMB_IRQ
void PWMB_IRQHandler(void) {		// PWM interrupt subroutine 
	if (PWMB->PIIR.PWMIF1) {
		CaptureCounter++;						// Delay (PWM_CNR+1) usec
		if (CaptureCounter == 0) {	// Overflow
			}
		PWMB->PIIR.PWMIF1	=	1;			// write 1 to clear this bit to zero
		}
	if (PWMB->CCR0.CAPIF1) {
		if (PWMB->CCR0.CFLRI1) {		// Calculate High Level width
			CaptureValue[0] = CaptureCounter*(PWM_CNR+1)+(PWM_CNR-PWMB->CFLR1);//usec //FLR Falling
			CaptureCounter = 0;				// reset
			PWMB->CCR0.CFLRI1 = 0;// write 0 to clear this bit to zero if BCn bit is 0
		}
		if (PWMB->CCR0.CRLRI1) {		//Calculate Low Level width
			CaptureValue[1] = CaptureCounter*(PWM_CNR+1)+(PWM_CNR-PWMB->CRLR1);//usec //RLR Raising
			CaptureCounter = 0;				// reset
			PWMB->CCR0.CRLRI1 = 0;// write 0 to clear this bit to zero if BCn bit is 0	
			}
		PWMB->CCR0.CAPIF1 = 1;	// write 1 to clear this bit to zero
		}
	}
//--------------------------------------------------------------------Timer0
void InitTIMER0(void) {
	/* Step 0. GPIO initial */
	SYS->GPBMFP.TM0 = 1;				// System Manager Control Registers
	
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR0_S = 0;	// Select 12Mhz for Timer0 clock source
	// 0 = 12 MHz, 1 = 32 kHz, 2 = HCLK, 7 = 22.1184 MHz
	SYSCLK->APBCLK.TMR0_EN = 1;	// Enable Timer0 clock source

	/* Step 2. Select Operation mode */	
	TIMER0->TCSR.MODE = 2;			// 2 -> Select Toggle mode
	// 0 = One shot, 1 = Periodic, 2 = Toggle, 3 = continuous counting mode
	
	/* Step 3. Select Time out period 
	= (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER0->TCSR.PRESCALE = 11;	// Set Prescale [0~255]
	TIMER0->TCMPR = 1000000;		// Set TCMPR [0~16777215]
	// (1/12000000)*(11+1)*(1000000)= 1 sec or 1 Hz

	/* Step 4. Enable interrupt */
	TIMER0->TCSR.IE = 1;
	TIMER0->TISR.TIF = 1;				// Write 1 to clear the interrupt flag 		
	NVIC_EnableIRQ(TMR0_IRQn);	// Enable Timer0 Interrupt

	/* Step 5. Enable Timer module */
	TIMER0->TCSR.CRST = 1;			// Reset up counter
	TIMER0->TCSR.CEN = 1;				// Enable Timer0
	}
//----------------------------------------------------------------Timer0_IRQ
void TMR0_IRQHandler(void) {	// Timer0 interrupt subroutine
	char lcd2_buffer[18] = "Timer0:";
	Timer0Counter += 1;
	sprintf(lcd2_buffer+7, " %d s.", Timer0Counter);
	print_lcd(2, lcd2_buffer);
	TIMER0->TISR.TIF = 1;    		// Write 1 to clear the interrupt flag 
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
		sprintf(lcd2_buffer+5, "%dsec    ", CaptureValue[0]/1000000);
		} else if (CaptureValue[0] >= 1000) {
		sprintf(lcd2_buffer+5, "%dmsec", CaptureValue[0]/1000);
		} else
		sprintf(lcd2_buffer+5, "%dusec", CaptureValue[0]);
	print_lcd(2, lcd2_buffer);

	if (CaptureValue[1] >= 1000000) {
		sprintf(lcd3_buffer+5, "%dsec    ", CaptureValue[1]/1000000);
		} else if (CaptureValue[1] >= 1000) {
		sprintf(lcd3_buffer+5, "%dmsec", CaptureValue[1]/1000);
		} else
		sprintf(lcd3_buffer+5, "%dusec", CaptureValue[1]);
	print_lcd(3, lcd3_buffer);
	
	TIMER3->TISR.TIF = 1;    		// Write 1 to clear the interrupt flag 
	}
//----------------------------------------------------------------------MAIN
int32_t main (void) {
	int8_t number;
	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1;
	SYSCLK->CLKSEL0.HCLK_S = 0;
	LOCKREG();
	Initial_pannel();  //call initial pannel function
	clr_all_pannal();
	InitRTC();
	InitPWM1();
	InitPWM2();
	//InitCapture0();
	//InitCapture1();
	InitCapture5();
	InitTIMER0();
	InitTIMER3();
	print_lcd(0, "Capture demo");
	print_lcd(1, ">>GPE5");
	OpenKeyPad();
	/* Configure general GPIO interrupt */
	DrvGPIO_Open(E_GPB, 15, E_IO_INPUT);
	/* Configure external interrupt */
	DrvGPIO_EnableEINT1(E_IO_BOTH_EDGE, E_MODE_EDGE, EINT1Callback);
	/* Waiting for interrupts */
	DrvGPIO_InitFunction(E_FUNC_I2C1);
	while(1) {
		number = Scankey();
		if (number == 1) {
				state = 0;
			}
		}
	}
