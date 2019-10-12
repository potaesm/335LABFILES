// Nu_LB-002:	L02_01_PWMgen.c

//	PWM0 -> GPA12 High level: 521 msec, Low level: 521 msec Green
//	PWM1 -> GPA13 High level: 262 msec, Low level: 786 msec Green
//	PWM2 -> GPA14 High level: 682 usec, Low level: 682 usec(microsecond) Red

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.              */
/* edited: june 2014: dejwoot.kha@mail.kmutt.ac.th                          */
/*--------------------------------------------------------------------------*/

#include <stdio.h>																											 
#include "NUC1xx.h"
#include "LCD_Driver.h"
#include "Driver\DrvSYS.h"

#define	PWM_CNR	0xFFFF

static uint16_t Timer0Counter=0;
static uint16_t Timer1Counter=0;
static uint16_t Timer2Counter=0;
static uint16_t Timer3Counter=0;

uint16_t	CaptureCounter = 0;
uint32_t	CaptureValue[2];

//--------------------------------------------------------------------------------------PWM0Gen GPA12
void InitPWM0(void) {
 	/* Step 1. GPIO initial */ 
	SYS->GPAMFP.PWM0_AD13 = 1;		// System Manager Control Registers
	
	/* Step 2. Enable and Select PWM clock source*/		
	SYSCLK->APBCLK.PWM01_EN = 1;	// Enable PWM clock
	SYSCLK->CLKSEL1.PWM01_S = 3;	// Select 12MHz for PWM clock source
	// 0:12MHz, 1:32.768 kHz, 2:HCLK, 3:22.1184 MHz 
	PWMA->PPR.CP01 = 21;	// Prescaler 0~255, Setting 0 to stop output clock
	PWMA->CSR.CSR0 = 3;		// clock divider ->	0:/2, 1:/4, 2:/8, 3:/16, 4:/1
	// PWM frequency = PWMxy_CLK/[(prescale+1)*(clock divider)*(CNR+1)]
	// Ex:= 22.1184M/[(21+1)*(16)*(2^16)] = 0.95880 Hz -> T = 1.042963 s ~ 521 ms + 521 ms

	/* Step 3. Select PWM Operation mode */
	PWMA->PCR.CH0MOD = 1;		// 0:One-shot mode, 1:Auto-load mode
	// CNR and CMR will be auto-cleared after setting CH0MOD form 0 to 1.
	PWMA->CNR0 = 0xFFFF;
	PWMA->CMR0 = 0x7FFF;
	// CMR < CNR:	PWM low width = (CNR-CMR) unit [one PWM clock cycle]
	//						PWM high width = (CMR+1) unit [T in ms / CNR]

	PWMA->PCR.CH0INV = 0;	// Inverter -> 0:off, 1:on
	PWMA->PCR.CH0EN = 1;	// PWM function -> 0:Disable, 1:Enable
 	PWMA->POE.PWM0 = 1;		// Output to pin -> 0:Diasble, 1:Enable
	}
	
//--------------------------------------------------------------------------------------PWM1Gen GPA13
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
	// Ex:= 12M/[(11+1)*(16)*(2^16)] = 0.95367 Hz -> T = 1.048576 s ~ 262 ms + 786 ms

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

//--------------------------------------------------------------------------------------PWM2Gen GPA14
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
	// Ex:= 12M/[(1+1)*(1)*(8191)] = 732.5 Hz -> T = 1365 micros. ~ 682 micros + 682 micros
	
	/* Step 3. Select PWM Operation mode */
	PWMA->PCR.CH2MOD = 1;	// 0:One-shot mode, 1:Auto-load mode
	// CNR and CMR will be auto-cleared after setting CH0MOD form 0 to 1.
	PWMA->CNR2 = 0x1FFF;	// 0x1FFF = 8191
	PWMA->CMR2 = 0x0FFF;
	// CMR < CNR:	PWM low width = (CNR-CMR) unit [one PWM clock cycle]
	//						PWM high width = (CMR+1) unit [T in ms / CNR in Dec]
	PWMA->PCR.CH2INV = 0;	// Inverter -> 0:off, 1:on
	PWMA->PCR.CH2EN = 1;	// PWM function -> 0:Disable, 1:Enable
 	PWMA->POE.PWM2 = 1;		// Output to pin -> 0:Diasble, 1:Enable
	}

//--------------------------------------------------------------------------------------PWM3Gen GPA15
void InitPWM3(void) {
 	/* Step 1. GPIO initial */ 
	SYS->GPAMFP.PWM3_I2SMCLK = 1;		// System Manager Control Registers
	
	/* Step 2. Enable and Select PWM clock source*/		
	SYSCLK->APBCLK.PWM23_EN = 1;	// Enable PWM clock
	SYSCLK->CLKSEL1.PWM23_S = 3;	// Select 12MHz for PWM clock source
	// 0:12MHz, 1:32.768 kHz, 2:HCLK, 3:22.1184 MHz 
	PWMA->PPR.CP23 = 21;	// Prescaler 0~255, Setting 0 to stop output clock
	PWMA->CSR.CSR3 = 3;		// clock divider ->	0:/2, 1:/4, 2:/8, 3:/16, 4:/1
	// PWM frequency = PWMxy_CLK/[(prescale+1)*(clock divider)*(CNR+1)]
	// Ex:= 22.1184M/[(21+1)*(16)*(2^16)] = 0.95880 Hz -> T = 1.042963 s ~ 521 ms + 521 ms

	/* Step 3. Select PWM Operation mode */
	PWMA->PCR.CH3MOD = 1;		// 0:One-shot mode, 1:Auto-load mode
	// CNR and CMR will be auto-cleared after setting CH0MOD form 0 to 1.
	PWMA->CNR3 = 0xFFFF;
	PWMA->CMR3 = 0x7FFF;
	// CMR < CNR:	PWM low width = (CNR-CMR) unit [one PWM clock cycle]
	//						PWM high width = (CMR+1) unit [T in ms / CNR]

	PWMA->PCR.CH3INV = 0;	// Inverter -> 0:off, 1:on
	PWMA->PCR.CH3EN = 1;	// PWM function -> 0:Disable, 1:Enable
 	PWMA->POE.PWM3 = 1;		// Output to pin -> 0:Diasble, 1:Enable
	}
	
//--------------------------------------------------------------------------------------PWM4Gen GPB11 Buzzer
void InitPWM4(void) {
 	/* Step 1. GPIO initial */ 
	SYS->GPBMFP.TM3_PWM4 = 1;		// System Manager Control Registers
	SYS->ALTMFP.PB11_PWM4 = 1;
	
	/* Step 2. Enable and Select PWM clock source*/		
	SYSCLK->APBCLK.PWM45_EN = 1;	// Enable PWM clock
	SYSCLK->CLKSEL2.PWM45_S = 0;	// Select 12Mhz for PWM clock source
	// 0:12MHz, 1:32.768 kHz, 2:HCLK, 3:22.1184 MHz 
	PWMB->PPR.CP01 = 1;		// Prescaler 0~255, Setting 0 to stop output clock
	PWMB->CSR.CSR0 = 4;		// clock divider ->	0:/2, 1:/4, 2:/8, 3:/16, 4:/1
	// PWM frequency = PWMxy_CLK/[(prescale+1)*(clock divider)*(CNR+1)]
	// Ex:= 12M/[(1+1)*(1)*(8191)] = 732.5 Hz -> T = 1365 micros. ~ 682+682
	
	/* Step 3. Select PWM Operation mode */
	PWMB->PCR.CH0MOD = 1;	// 0:One-shot mode, 1:Auto-load mode
	// CNR and CMR will be auto-cleared after setting CH0MOD form 0 to 1.
	PWMB->CNR0 = 0x1FFF;	// 0x1FFF = 8191
	PWMB->CMR0 = 0x0FFF;

	PWMB->PCR.CH0INV = 0;	// Inverter -> 0:off, 1:on
	PWMB->PCR.CH0EN = 1;	// PWM function -> 0:Disable, 1:Enable
 	PWMB->POE.PWM0 = 1;		// Output to pin -> 0:Diasble, 1:Enable
	}
	
//--------------------------------------------------------------------------------------PWM5Gen GE5
void InitPWM5(void) {
 	/* Step 1. GPIO initial */ 
	SYS->GPEMFP.PWM5 = 1;		// System Manager Control Registers
	
	/* Step 2. Enable and Select PWM clock source*/		
	SYSCLK->APBCLK.PWM45_EN = 1;	// Enable PWM clock
	SYSCLK->CLKSEL2.PWM45_S = 0;	// Select 12Mhz for PWM clock source
	// 0:12MHz, 1:32.768 kHz, 2:HCLK, 3:22.1184 MHz 
	PWMB->PPR.CP01 = 1;		// Prescaler 0~255, Setting 0 to stop output clock
	PWMB->CSR.CSR1 = 4;		// clock divider ->	0:/2, 1:/4, 2:/8, 3:/16, 4:/1
	// PWM frequency = PWMxy_CLK/[(prescale+1)*(clock divider)*(CNR+1)]
	// Ex:= 12M/[(1+1)*(1)*(8191)] = 732.5 Hz -> T = 1365 micros. ~ 682+682
	
	/* Step 3. Select PWM Operation mode */
	PWMB->PCR.CH1MOD = 1;	// 0:One-shot mode, 1:Auto-load mode
	// CNR and CMR will be auto-cleared after setting CH0MOD form 0 to 1.
	PWMB->CNR1 = 0x1FFF;	// 0x1FFF = 8191
	PWMB->CMR1 = 0x0FFF;

	PWMB->PCR.CH1INV = 0;	// Inverter -> 0:off, 1:on
	PWMB->PCR.CH1EN = 1;	// PWM function -> 0:Disable, 1:Enable
 	PWMB->POE.PWM1 = 1;		// Output to pin -> 0:Diasble, 1:Enable
	}
	
//--------------------------------------------------------------------------------------PWM6Gen GE0
void InitPWM6(void) {
 	/* Step 1. GPIO initial */ 
	SYS->GPEMFP.PWM6 = 1;		// System Manager Control Registers
	
	/* Step 2. Enable and Select PWM clock source*/		
	SYSCLK->APBCLK.PWM67_EN = 1;	// Enable PWM clock
	SYSCLK->CLKSEL2.PWM67_S = 0;	// Select 12Mhz for PWM clock source
	// 0:12MHz, 1:32.768 kHz, 2:HCLK, 3:22.1184 MHz 
	PWMB->PPR.CP23 = 1;		// Prescaler 0~255, Setting 0 to stop output clock
	PWMB->CSR.CSR2 = 4;		// clock divider ->	0:/2, 1:/4, 2:/8, 3:/16, 4:/1
	// PWM frequency = PWMxy_CLK/[(prescale+1)*(clock divider)*(CNR+1)]
	// Ex:= 12M/[(1+1)*(1)*(8191)] = 732.5 Hz -> T = 1365 micros. ~ 682+682
	
	/* Step 3. Select PWM Operation mode */
	PWMB->PCR.CH2MOD = 1;	// 0:One-shot mode, 1:Auto-load mode
	// CNR and CMR will be auto-cleared after setting CH0MOD form 0 to 1.
	PWMB->CNR2 = 0x1FFF;	// 0x1FFF = 8191
	PWMB->CMR2 = 0x0FFF;

	PWMB->PCR.CH2INV = 0;	// Inverter -> 0:off, 1:on
	PWMB->PCR.CH2EN = 1;	// PWM function -> 0:Disable, 1:Enable
 	PWMB->POE.PWM2 = 1;		// Output to pin -> 0:Diasble, 1:Enable
	}
	
//--------------------------------------------------------------------------------------PWM7Gen GE1
void InitPWM7(void) {
 	/* Step 1. GPIO initial */ 
	SYS->GPEMFP.PWM7 = 1;		// System Manager Control Registers
	
	/* Step 2. Enable and Select PWM clock source*/		
	SYSCLK->APBCLK.PWM67_EN = 1;	// Enable PWM clock
	SYSCLK->CLKSEL2.PWM67_S = 0;	// Select 12Mhz for PWM clock source
	// 0:12MHz, 1:32.768 kHz, 2:HCLK, 3:22.1184 MHz 
	PWMB->PPR.CP23 = 1;		// Prescaler 0~255, Setting 0 to stop output clock
	PWMB->CSR.CSR3 = 4;		// clock divider ->	0:/2, 1:/4, 2:/8, 3:/16, 4:/1
	// PWM frequency = PWMxy_CLK/[(prescale+1)*(clock divider)*(CNR+1)]
	// Ex:= 12M/[(1+1)*(1)*(8191)] = 732.5 Hz -> T = 1365 micros. ~ 682+682
	
	/* Step 3. Select PWM Operation mode */
	PWMB->PCR.CH3MOD = 1;	// 0:One-shot mode, 1:Auto-load mode
	// CNR and CMR will be auto-cleared after setting CH0MOD form 0 to 1.
	PWMB->CNR3 = 0x1FFF;	// 0x1FFF = 8191
	PWMB->CMR3 = 0x0FFF;

	PWMB->PCR.CH3INV = 0;	// Inverter -> 0:off, 1:on
	PWMB->PCR.CH3EN = 1;	// PWM function -> 0:Disable, 1:Enable
 	PWMB->POE.PWM3 = 1;		// Output to pin -> 0:Diasble, 1:Enable
	}
	
//--------------------------------------------------------------------------------------Capture0 GPA12
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
	
//--------------------------------------------------------------------------------------Capture1 GPA13
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
	PWMA->CAPENR = 2;				// Enable Capture function pin from 2^"CH" = 2^1 = 2
	PWMA->CCR0.CAPCH1EN = 1;// Enable Capture function

	/* Step 4. Set PWM Interrupt change IEx and x=CH */
	PWMA->CCR0.CRL_IE1 = 1;	// Enable Capture rising edge interrupt   
	PWMA->CCR0.CFL_IE1 = 1;	// Enable Capture falling edge interrupt
	PWMA->PIER.PWMIE1 = 1;	// Enable PWM interrupt for down-counter equal zero.
	NVIC_EnableIRQ(PWMA_IRQn);  // Enable PWM inturrupt

	/* Step 5. Enable PWM down counter*/
	PWMA->PCR.CH1EN = 1;		// Enable PWM down counter
	}
	
//--------------------------------------------------------------------------------------Capture2 GPA14
void InitCapture2(void) {
 	/* Step 1. GPIO initial */ 
	SYS->GPAMFP.PWM2_AD15 = 1;
	
	/* Step 2. Enable and Select PWM clock source*/		
	SYSCLK->APBCLK.PWM23_EN = 1; // Enable PWM clock
	SYSCLK->CLKSEL1.PWM23_S = 0; // Select 12Mhz for PWM clock source
	// 0:12MHz, 1:32.768 kHz, 2:HCLK, 3:22.1184 MHz
	
	PWMA->PPR.CP23 = 11;		// Prescaler 0~255, Setting 0 to stop output clock
	PWMA->CSR.CSR2 = 4;			// clock divider -> 0:/2, 1:/4, 2:/8, 3:/16, 4:/1
									         
	/* Step 3. Select PWM Operation mode */
	PWMA->PCR.CH2MOD = 1;		// 0:One-shot mode, 1:Auto-load mode
	//CNR and CMR will be auto-cleared after setting CH0MOD from 0 to 1.
	PWMA->CNR2 = PWM_CNR;			// Set Reload register
	PWMA->CAPENR = 4;				// Enable Capture function pin from 2^"CH" = 2^1 = 2
	PWMA->CCR2.CAPCH2EN = 1;// Enable Capture function

	/* Step 4. Set PWM Interrupt change IEx and x=CH */
	PWMA->CCR2.CRL_IE2 = 1;	// Enable Capture rising edge interrupt   
	PWMA->CCR2.CFL_IE2 = 1;	// Enable Capture falling edge interrupt
	PWMA->PIER.PWMIE2 = 1;	// Enable PWM interrupt for down-counter equal zero.
	NVIC_EnableIRQ(PWMA_IRQn);  // Enable PWM inturrupt

	/* Step 5. Enable PWM down counter*/
	PWMA->PCR.CH2EN = 1;		// Enable PWM down counter
	}
	
//--------------------------------------------------------------------------------------Capture3 GPA15
void InitCapture3(void) {
 	/* Step 1. GPIO initial */ 
	SYS->GPAMFP.PWM3_I2SMCLK = 1;
	
	/* Step 2. Enable and Select PWM clock source*/		
	SYSCLK->APBCLK.PWM23_EN = 1; // Enable PWM clock
	SYSCLK->CLKSEL1.PWM23_S = 0; // Select 12Mhz for PWM clock source
	// 0:12MHz, 1:32.768 kHz, 2:HCLK, 3:22.1184 MHz
	
	PWMA->PPR.CP23 = 11;		// Prescaler 0~255, Setting 0 to stop output clock
	PWMA->CSR.CSR3 = 4;			// clock divider -> 0:/2, 1:/4, 2:/8, 3:/16, 4:/1
									         
	/* Step 3. Select PWM Operation mode */
	PWMA->PCR.CH3MOD = 1;		// 0:One-shot mode, 1:Auto-load mode
	//CNR and CMR will be auto-cleared after setting CH0MOD from 0 to 1.
	PWMA->CNR3 = PWM_CNR;			// Set Reload register
	PWMA->CAPENR = 8;				// Enable Capture function pin from 2^"CH" = 2^1 = 2
	PWMA->CCR2.CAPCH3EN = 1;// Enable Capture function

	/* Step 4. Set PWM Interrupt change IEx and x=CH */
	PWMA->CCR2.CRL_IE3 = 1;	// Enable Capture rising edge interrupt   
	PWMA->CCR2.CFL_IE3 = 1;	// Enable Capture falling edge interrupt
	PWMA->PIER.PWMIE3 = 1;	// Enable PWM interrupt for down-counter equal zero.
	NVIC_EnableIRQ(PWMA_IRQn);  // Enable PWM inturrupt

	/* Step 5. Enable PWM down counter*/
	PWMA->PCR.CH3EN = 1;		// Enable PWM down counter
	}
	
//--------------------------------------------------------------------------------------Capture4 GPB11 Buzzer
void InitCapture4(void) {
 	/* Step 1. GPIO initial */ 
	SYS->GPBMFP.TM3_PWM4 = 1;
	
	/* Step 2. Enable and Select PWM clock source*/		
	SYSCLK->APBCLK.PWM45_EN = 1; // Enable PWM clock
	SYSCLK->CLKSEL2.PWM45_S = 0; // Select 12Mhz for PWM clock source
	// 0:12MHz, 1:32.768 kHz, 2:HCLK, 3:22.1184 MHz
	
	PWMB->PPR.CP01 = 11;		// Prescaler 0~255, Setting 0 to stop output clock
	PWMB->CSR.CSR0 = 4;			// clock divider -> 0:/2, 1:/4, 2:/8, 3:/16, 4:/1
									         
	/* Step 3. Select PWM Operation mode */
	PWMB->PCR.CH0MOD = 1;		// 0:One-shot mode, 1:Auto-load mode
	//CNR and CMR will be auto-cleared after setting CH0MOD from 0 to 1.
	PWMB->CNR0 = PWM_CNR;			// Set Reload register
	PWMB->CAPENR = 1;				// Enable Capture function pin
	PWMB->CCR0.CAPCH0EN = 1;// Enable Capture function

	/* Step 4. Set PWM Interrupt */
	PWMB->CCR0.CRL_IE0 = 1;	// Enable Capture rising edge interrupt
	PWMB->CCR0.CFL_IE0 = 1;	// Enable Capture falling edge interrupt
	PWMB->PIER.PWMIE0 = 1;	// Enable PWM interrupt for down-counter equal zero.
	NVIC_EnableIRQ(PWMB_IRQn);  // Enable PWM inturrupt

	/* Step 5. Enable PWM down counter*/
	PWMB->PCR.CH0EN = 1;		// Enable PWM down counter
	}
	
//--------------------------------------------------------------------------------------Capture5 GE5
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
	
//--------------------------------------------------------------------------------------Capture6 GE0
void InitCapture6(void) {
 	/* Step 1. GPIO initial */ 
	SYS->GPEMFP.PWM6 = 1;
	
	/* Step 2. Enable and Select PWM clock source*/		
	SYSCLK->APBCLK.PWM67_EN = 1; // Enable PWM clock
	SYSCLK->CLKSEL2.PWM67_S = 0; // Select 12Mhz for PWM clock source
	// 0:12MHz, 1:32.768 kHz, 2:HCLK, 3:22.1184 MHz
	
	PWMB->PPR.CP23 = 11;		// Prescaler 0~255, Setting 0 to stop output clock
	PWMB->CSR.CSR2 = 4;			// clock divider -> 0:/2, 1:/4, 2:/8, 3:/16, 4:/1
									         
	/* Step 3. Select PWM Operation mode */
	PWMB->PCR.CH2MOD = 1;		// 0:One-shot mode, 1:Auto-load mode
	//CNR and CMR will be auto-cleared after setting CH0MOD from 0 to 1.
	PWMB->CNR2 = PWM_CNR;			// Set Reload register
	PWMB->CAPENR = 4;				// Enable Capture function pin from 2^"CH" = 2^1 = 2
	PWMB->CCR2.CAPCH2EN = 1;// Enable Capture function

	/* Step 4. Set PWM Interrupt change IEx and x=CH */
	PWMB->CCR2.CRL_IE2 = 1;	// Enable Capture rising edge interrupt   
	PWMB->CCR2.CFL_IE2 = 1;	// Enable Capture falling edge interrupt
	PWMB->PIER.PWMIE2 = 1;	// Enable PWM interrupt for down-counter equal zero.
	NVIC_EnableIRQ(PWMB_IRQn);  // Enable PWM inturrupt

	/* Step 5. Enable PWM down counter*/
	PWMB->PCR.CH2EN = 1;		// Enable PWM down counter
	}
	
//--------------------------------------------------------------------------------------Capture7 GE1
void InitCapture7(void) {
 	/* Step 1. GPIO initial */ 
	SYS->GPEMFP.PWM7 = 1;
	
	/* Step 2. Enable and Select PWM clock source*/		
	SYSCLK->APBCLK.PWM67_EN = 1; // Enable PWM clock
	SYSCLK->CLKSEL2.PWM67_S = 0; // Select 12Mhz for PWM clock source
	// 0:12MHz, 1:32.768 kHz, 2:HCLK, 3:22.1184 MHz
	
	PWMB->PPR.CP23 = 11;		// Prescaler 0~255, Setting 0 to stop output clock
	PWMB->CSR.CSR3 = 4;			// clock divider -> 0:/2, 1:/4, 2:/8, 3:/16, 4:/1
									         
	/* Step 3. Select PWM Operation mode */
	PWMB->PCR.CH3MOD = 1;		// 0:One-shot mode, 1:Auto-load mode
	//CNR and CMR will be auto-cleared after setting CH0MOD from 0 to 1.
	PWMB->CNR3 = PWM_CNR;			// Set Reload register
	PWMB->CAPENR = 8;				// Enable Capture function pin from 2^"CH" = 2^1 = 2
	PWMB->CCR2.CAPCH3EN = 1;// Enable Capture function

	/* Step 4. Set PWM Interrupt change IEx and x=CH */
	PWMB->CCR2.CRL_IE3 = 1;	// Enable Capture rising edge interrupt   
	PWMB->CCR2.CFL_IE3 = 1;	// Enable Capture falling edge interrupt
	PWMB->PIER.PWMIE3 = 1;	// Enable PWM interrupt for down-counter equal zero.
	NVIC_EnableIRQ(PWMB_IRQn);  // Enable PWM inturrupt

	/* Step 5. Enable PWM down counter*/
	PWMB->PCR.CH3EN = 1;		// Enable PWM down counter
	}
	
//--------------------------------------------------------------------------------------PWMA_IRQ Capture0 GPA12
/*void PWMA_IRQHandler(void) {		// PWM interrupt subroutine 
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
	}*/
	
//--------------------------------------------------------------------------------------PWMA_IRQ Capture1 GPA13
/*void PWMA_IRQHandler(void) {		// PWM interrupt subroutine 
	if (PWMA->PIIR.PWMIF1) {
		CaptureCounter++;						// Delay (PWM_CNR+1) usec
		if (CaptureCounter == 0) {	// Overflow
			}
		PWMA->PIIR.PWMIF1	=	1;			// write 1 to clear this bit to zero
		}
	if (PWMA->CCR0.CAPIF1) {
		if (PWMA->CCR0.CFLRI1) {		// Calculate High Level width
			CaptureValue[0] = CaptureCounter*(PWM_CNR+1)+(PWM_CNR-PWMA->CFLR1);//usec
			CaptureCounter = 0;				// reset
			PWMA->CCR0.CFLRI1 = 0;// write 0 to clear this bit to zero if BCn bit is 0
		}
		if (PWMA->CCR0.CRLRI1) {		//Calculate Low Level width
			CaptureValue[1] = CaptureCounter*(PWM_CNR+1)+(PWM_CNR-PWMA->CRLR1);//usec
			CaptureCounter = 0;				// reset
			PWMA->CCR0.CRLRI1 = 0;// write 0 to clear this bit to zero if BCn bit is 0	
			}
		PWMA->CCR0.CAPIF1 = 1;	// write 1 to clear this bit to zero
		}
	}*/
	
//--------------------------------------------------------------------------------------PWMA_IRQ Capture2 GPA14
/*void PWMA_IRQHandler(void) {		// PWM interrupt subroutine
	if (PWMA->PIIR.PWMIF2) {
		CaptureCounter++;						// Delay (PWM_CNR+1) usec
		if (CaptureCounter == 0) {	// Overflow
			}
		PWMA->PIIR.PWMIF2	=	1;			// write 1 to clear this bit to zero
		}
	if (PWMA->CCR2.CAPIF2) {
		if (PWMA->CCR2.CFLRI2) {		// Calculate High Level width
			CaptureValue[0] = CaptureCounter*(PWM_CNR+1)+(PWM_CNR-PWMA->CRLR2);//usec
			CaptureCounter = 0;				// reset
			PWMA->CCR2.CFLRI2 = 0;// write 0 to clear this bit to zero if BCn bit is 0
		}
		if (PWMA->CCR2.CRLRI2) {		//Calculate Low Level width
			CaptureValue[1] = CaptureCounter*(PWM_CNR+1)+(PWM_CNR-PWMA->CRLR2);//usec
			CaptureCounter = 0;				// reset
			PWMA->CCR2.CRLRI2 = 0;// write 0 to clear this bit to zero if BCn bit is 0	
			}
		PWMA->CCR2.CAPIF2 = 1;	// write 1 to clear this bit to zero
		}
	}*/
	
//--------------------------------------------------------------------------------------PWMA_IRQ Capture3 GPA15
/*void PWMA_IRQHandler(void) {		// PWM interrupt subroutine
	if (PWMA->PIIR.PWMIF3) {
		CaptureCounter++;						// Delay (PWM_CNR+1) usec
		if (CaptureCounter == 0) {	// Overflow
			}
		PWMA->PIIR.PWMIF3	=	1;			// write 1 to clear this bit to zero
		}
	if (PWMA->CCR2.CAPIF3) {
		if (PWMA->CCR2.CFLRI3) {		// Calculate High Level width
			CaptureValue[0] = CaptureCounter*(PWM_CNR+1)+(PWM_CNR-PWMA->CRLR3);//usec
			CaptureCounter = 0;				// reset
			PWMA->CCR2.CFLRI3 = 0;// write 0 to clear this bit to zero if BCn bit is 0
		}
		if (PWMA->CCR2.CRLRI3) {		//Calculate Low Level width
			CaptureValue[1] = CaptureCounter*(PWM_CNR+1)+(PWM_CNR-PWMA->CRLR3);//usec
			CaptureCounter = 0;				// reset
			PWMA->CCR2.CRLRI3 = 0;// write 0 to clear this bit to zero if BCn bit is 0	
			}
		PWMA->CCR2.CAPIF3 = 1;	// write 1 to clear this bit to zero
		}
	}*/
	
//--------------------------------------------------------------------------------------PWMB_IRQ Capture4 GPB11 Buzzer
/*void PWMB_IRQHandler(void) {		// PWM interrupt subroutine 
	if (PWMB->PIIR.PWMIF0) {
		CaptureCounter++;						// Delay (PWM_CNR+1) usec
		if (CaptureCounter == 0) {	// Overflow
			}
		PWMB->PIIR.PWMIF0	=	1;			// write 1 to clear this bit to zero
		}
	if (PWMB->CCR0.CAPIF0) {
		if (PWMB->CCR0.CFLRI0) {		// Calculate High Level width
			CaptureValue[0] = CaptureCounter*(PWM_CNR+1)+(PWM_CNR-PWMB->CFLR0);//usec
			CaptureCounter = 0;				// reset
			PWMB->CCR0.CFLRI0 = 0;// write 0 to clear this bit to zero if BCn bit is 0
		}
		if (PWMB->CCR0.CRLRI0) {		//Calculate Low Level width
			CaptureValue[1] = CaptureCounter*(PWM_CNR+1)+(PWM_CNR-PWMB->CRLR0);//usec
			CaptureCounter = 0;				// reset
			PWMB->CCR0.CRLRI0 = 0;// write 0 to clear this bit to zero if BCn bit is 0	
			}
		PWMB->CCR0.CAPIF0 = 1;	// write 1 to clear this bit to zero
		}
	}*/
	
//--------------------------------------------------------------------------------------PWMB_IRQ Capture5 GE5
/*void PWMB_IRQHandler(void) {		// PWM interrupt subroutine 
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
	}*/

//--------------------------------------------------------------------------------------PWMB_IRQ Capture6 GE0
/*void PWMB_IRQHandler(void) {		// PWM interrupt subroutine
	if (PWMB->PIIR.PWMIF2) {
		CaptureCounter++;						// Delay (PWM_CNR+1) usec
		if (CaptureCounter == 0) {	// Overflow
			}
		PWMB->PIIR.PWMIF2	=	1;			// write 1 to clear this bit to zero
		}
	if (PWMB->CCR2.CAPIF2) {
		if (PWMB->CCR2.CFLRI2) {		// Calculate High Level width
			CaptureValue[0] = CaptureCounter*(PWM_CNR+1)+(PWM_CNR-PWMB->CRLR2);//usec
			CaptureCounter = 0;				// reset
			PWMB->CCR2.CFLRI2 = 0;// write 0 to clear this bit to zero if BCn bit is 0
		}
		if (PWMB->CCR2.CRLRI2) {		//Calculate Low Level width
			CaptureValue[1] = CaptureCounter*(PWM_CNR+1)+(PWM_CNR-PWMB->CRLR2);//usec
			CaptureCounter = 0;				// reset
			PWMB->CCR2.CRLRI2 = 0;// write 0 to clear this bit to zero if BCn bit is 0	
			}
		PWMB->CCR2.CAPIF2 = 1;	// write 1 to clear this bit to zero
		}
	}*/
	
//--------------------------------------------------------------------------------------PWMB_IRQ Capture7 GE1
void PWMB_IRQHandler(void) {		// PWM interrupt subroutine
	if (PWMB->PIIR.PWMIF3) {
		CaptureCounter++;						// Delay (PWM_CNR+1) usec
		if (CaptureCounter == 0) {	// Overflow
			}
		PWMB->PIIR.PWMIF3	=	1;			// write 1 to clear this bit to zero
		}
	if (PWMB->CCR2.CAPIF3) {
		if (PWMB->CCR2.CFLRI3) {		// Calculate High Level width
			CaptureValue[0] = CaptureCounter*(PWM_CNR+1)+(PWM_CNR-PWMB->CRLR3);//usec
			CaptureCounter = 0;				// reset
			PWMB->CCR2.CFLRI3 = 0;// write 0 to clear this bit to zero if BCn bit is 0
		}
		if (PWMB->CCR2.CRLRI3) {		//Calculate Low Level width
			CaptureValue[1] = CaptureCounter*(PWM_CNR+1)+(PWM_CNR-PWMB->CRLR3);//usec
			CaptureCounter = 0;				// reset
			PWMB->CCR2.CRLRI3 = 0;// write 0 to clear this bit to zero if BCn bit is 0	
			}
		PWMB->CCR2.CAPIF3 = 1;	// write 1 to clear this bit to zero
		}
	}
	
//--------------------------------------------------------------------------------------Timer0
void InitTIMER0(void) {
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR0_S = 0;	// Select 12Mhz for Timer0 clock source
	// 0 = 12 MHz, 1 = 32 kHz, 2 = HCLK, 7 = 22.1184 MHz
	SYSCLK->APBCLK.TMR0_EN = 1;	// Enable Timer0 clock source

	/* Step 2. Select Operation mode */	
	TIMER0->TCSR.MODE = 1;			// 1 -> Select periodic mode
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
	// (1/12000000)*(11+1)*(1000000)= 1 sec or 1 Hz

	/* Step 4. Enable interrupt */
	TIMER1->TCSR.IE = 1;
	TIMER1->TISR.TIF = 1;				// Write 1 to clear the interrupt flag 		
	NVIC_EnableIRQ(TMR1_IRQn);	// Enable Timer0 Interrupt

	/* Step 5. Enable Timer module */
	TIMER1->TCSR.CRST = 1;			// Reset up counter
	TIMER1->TCSR.CEN = 1;				// Enable Timer0
	}
	
//--------------------------------------------------------------------------------------Timer2
void InitTIMER2(void) {
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR2_S = 0;	// Select 12Mhz for Timer0 clock source
	// 0 = 12 MHz, 1 = 32 kHz, 2 = HCLK, 7 = 22.1184 MHz
	SYSCLK->APBCLK.TMR2_EN = 1;	// Enable Timer0 clock source

	/* Step 2. Select Operation mode */	
	TIMER2->TCSR.MODE = 1;			// 1 -> Select periodic mode
	// 0 = One shot, 1 = Periodic, 2 = Toggle, 3 = continuous counting mode
	
	/* Step 3. Select Time out period 
	= (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER2->TCSR.PRESCALE = 11;	// Set Prescale [0~255]
	TIMER2->TCMPR = 1000000;		// Set TCMPR [0~16777215]
	// (1/12000000)*(11+1)*(1000000)= 1 sec or 1 Hz

	/* Step 4. Enable interrupt */
	TIMER2->TCSR.IE = 1;
	TIMER2->TISR.TIF = 1;				// Write 1 to clear the interrupt flag 		
	NVIC_EnableIRQ(TMR2_IRQn);	// Enable Timer0 Interrupt

	/* Step 5. Enable Timer module */
	TIMER2->TCSR.CRST = 1;			// Reset up counter
	TIMER2->TCSR.CEN = 1;				// Enable Timer0
	}
	
//--------------------------------------------------------------------------------------Timer3
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
	
//--------------------------------------------------------------------------------------Timer0_IRQ
void TMR0_IRQHandler(void) {	// Timer0 interrupt subroutine
	char lcd1_buffer[18] = "Timer0:";
	char lcd2_buffer[18] = "High:";
	char lcd3_buffer[18] = "Low: ";
	
	Timer0Counter += 1;
	sprintf(lcd1_buffer+7, " %d s.", Timer0Counter);
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
	
	TIMER0->TISR.TIF = 1;    		// Write 1 to clear the interrupt flag 
	}
//--------------------------------------------------------------------------------------Timer1_IRQ
void TMR1_IRQHandler(void) {	// Timer0 interrupt subroutine
	char lcd1_buffer[18] = "Timer1:";
	char lcd2_buffer[18] = "High:";
	char lcd3_buffer[18] = "Low: ";
	
	Timer1Counter += 1;
	sprintf(lcd1_buffer+7, " %d s.", Timer1Counter);
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
	
	TIMER1->TISR.TIF = 1;    		// Write 1 to clear the interrupt flag 
	}
//--------------------------------------------------------------------------------------Timer2_IRQ
void TMR2_IRQHandler(void) {	// Timer0 interrupt subroutine
	char lcd1_buffer[18] = "Timer2:";
	char lcd2_buffer[18] = "High:";
	char lcd3_buffer[18] = "Low: ";
	
	Timer2Counter += 1;
	sprintf(lcd1_buffer+7, " %d s.", Timer2Counter);
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
	
	TIMER2->TISR.TIF = 1;    		// Write 1 to clear the interrupt flag 
	}
	
//--------------------------------------------------------------------------------------Timer3_IRQ
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
	
//--------------------------------------------------------------------------------------MAIN
int32_t main (void) {
	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1;	
	SYSCLK->CLKSEL0.HCLK_S = 0;
	// 0:Clock source from external 4~24 MHz(12MHz) high speed crystal clock 
	LOCKREG();
	Initial_pannel();  //call initial pannel function
	clr_all_pannal();
	
	//Generator
	InitPWM0();
	InitPWM1();
	InitPWM2();
	//InitPWM3();
	//InitPWM4();
	InitPWM5();
	InitPWM6();
	//InitPWM7();
	
	//Capture
	//InitCapture0();
	//InitCapture1();
	//InitCapture2();
	//InitCapture3();
	//InitCapture4();
	//InitCapture5();
	//InitCapture6();
	InitCapture7();
	
	//Display
	InitTIMER0();
	//InitTIMER1();
	//InitTIMER2();
	//InitTIMER3();
	 
	while(1) {}				 
	}
