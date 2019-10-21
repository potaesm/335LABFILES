/* 
connect GPB0 to GPB1

UART0:GPB0 - Rx
			GPB1 - Tx
UART1:GPB4 - Rx
			GPB5 - Tx
UART2:GPD14 - Rx
			GPD15 - Tx			
*/

//
// Smpl_UART0 : while loop for UART0-TX keep transmitting 8 bytes string
//            : IRQ routine for UART0-RX keep receiving 8 bytes string & print to LCD
//						: (TX & RX at the same time)

// LAB335 WEEK 9 BY SUTHINAN & PONGPITH

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.              */
/* edited: APRIL 2019: suthinan.musitmani@mail.kmutt.ac.th                  */
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include "Driver\DrvUART.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvSYS.h"
#include "NUC1xx.h"
#include "NUC1xx-LB_002\LCD_Driver.h"
#include "Driver\DrvI2C.h"
#include "Seven_Segment.h"

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

volatile uint8_t comRbuf[16];
volatile uint16_t comRbytes = 0;

char TEXT1[16] = "TX: sending...  ";
char TEXT2[16] = "RX:             ";

#define	PWM_CNR	0xFFFF

uint32_t UART_Display_Counter=0;

static uint16_t Timer2Counter=0;
static uint16_t Timer3Counter=0;
uint16_t	CaptureCounter = 0;
uint32_t	CaptureValue[2];

unsigned char SEG_BUF_HEX[16]={SEG_N0, SEG_N1, SEG_N2, SEG_N3, SEG_N4, SEG_N5, SEG_N6, SEG_N7, SEG_N8, SEG_N9, SEG_Na, SEG_Nb, SEG_Nc, SEG_Nd, SEG_Ne, SEG_Nf}; 

//------------------------------------------------------------------------------------------On Board 7SEG Dec Display
void seg_display(int16_t value)
{
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
	
//------------------------------------------------------------------------------------------HEX2Disp for 7SEG-I2C
uint8_t HEX2Disp(uint8_t hexNum) {
	static const uint8_t lookUp[16] = {
		0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8,
		0x80, 0x90, 0x88, 0x83, 0xC6, 0xA1, 0x86, 0x8E
		};
	uint8_t hexDisp = lookUp[hexNum];
		
	return hexDisp;
	}
//------------------------------------------------------------------------------------------Write to PCF8574
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
	
//------------------------------------------------------------------------------------------Display to 7SEG-I2C in Dec
void disp2Digit8574_DEC(int16_t value)
{
  int8_t digit;
		digit = value / 1000;
			
		value = value - digit * 1000;
		digit = value / 100;
		Write_to_any8574(0x72,HEX2Disp(digit));	

		value = value - digit * 100;
		digit = value / 10;
		Write_to_any8574(0x70,HEX2Disp(digit));

		value = value - digit * 10;
		digit = value;
}
//------------------------------------------------------------------------------------------D2A For PCF8591
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
//------------------------------------------------------------------------------------------A2D For PCF8591
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

//------------------------------------------------------------------------------------------ADC3 For Read DAC (PCF8591)
void InitADC3(void) {
	/* Step 1. GPIO initial */ 
	GPIOA->OFFD |= 0x00080000;		// Disable digital input path (when input is analog signal)
	SYS->GPAMFP.ADC3_AD10 = 1;					// Set ADC function 
				
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
	ADC->ADCHER.CHEN = 0x0008;		// 8-bits -> ch0
	
	/* Step 5. Enable ADC interrupt */
	//ADC->ADSR.ADF = 1;					// clear the A/D interrupt flags
	//ADC->ADCR.ADIE = 1;
	//NVIC_EnableIRQ(ADC_IRQn);
	
	/* Step 6. A/D Conversion Start */
	ADC->ADCR.ADST = 1;						
	// ADST will be cleared to 0 by hardware automatically 
	// at the ends of single mode and single cycle scan mode.
	}

//------------------------------------------------------------------------------------------PWM1 To Be Captured
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

//------------------------------------------------------------------------------------------Capturer2 GPA14
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
	PWMA->CAPENR = 4;				// Enable Capture function pin
	PWMA->CCR2.CAPCH2EN = 1;// Enable Capture function

	/* Step 4. Set PWM Interrupt */
	PWMA->CCR2.CRL_IE2 = 1;	// Enable Capture rising edge interrupt
	PWMA->CCR2.CFL_IE2 = 1;	// Enable Capture falling edge interrupt
	PWMA->PIER.PWMIE2 = 1;	// Enable PWM interrupt for down-counter equal zero.
	NVIC_EnableIRQ(PWMA_IRQn);  // Enable PWM inturrupt

	/* Step 5. Enable PWM down counter*/
	PWMA->PCR.CH2EN = 1;		// Enable PWM down counter
	}

//------------------------------------------------------------------------------------------PWMA_IRQ For Capturer2
void PWMA_IRQHandler(void) {		// PWM interrupt subroutine 
	if (PWMA->PIIR.PWMIF2) {
		CaptureCounter++;						// Delay (PWM_CNR+1) usec
		if (CaptureCounter == 0) {	// Overflow
			}
		PWMA->PIIR.PWMIF2	=	1;			// write 1 to clear this bit to zero
		}
	if (PWMA->CCR2.CAPIF2) {
		if (PWMA->CCR2.CFLRI2) {		// Calculate High Level width
			CaptureValue[0] = CaptureCounter*(PWM_CNR+1)+(PWM_CNR-PWMA->CFLR2);//usec
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
	}
	
//------------------------------------------------------------------------------------------TIMER2 For Toggle 0.8 s GPB10
void InitTIMER2(void) {
	/* Step 0. GPIO initial */
	SYS->GPBMFP.TM2_SS01 = 1;				// System Manager Control Registers
	
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR2_S = 0;	// Select 12Mhz for Timer0 clock source
	// 0 = 12 MHz, 1 = 32 kHz, 2 = HCLK, 7 = 22.1184 MHz
	SYSCLK->APBCLK.TMR2_EN = 1;	// Enable Timer0 clock source

	/* Step 2. Select Operation mode */	
	TIMER2->TCSR.MODE = 2;			// 2 -> Select Toggle mode
	// 0 = One shot, 1 = Periodic, 2 = Toggle, 3 = continuous counting mode
	
	/* Step 3. Select Time out period 
	= (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER2->TCSR.PRESCALE = 11;	// Set Prescale [0~255]
	TIMER2->TCMPR = 800000;		// Set TCMPR [0~16777215]
	// (1/12000000)*(11+1)*(1000000)= 1 sec or 1 Hz

	/* Step 4. Enable interrupt */
	TIMER2->TCSR.IE = 1;
	TIMER2->TISR.TIF = 1;				// Write 1 to clear the interrupt flag 		
	NVIC_EnableIRQ(TMR2_IRQn);	// Enable Timer0 Interrupt

	/* Step 5. Enable Timer module */
	TIMER2->TCSR.CRST = 1;			// Reset up counter
	TIMER2->TCSR.CEN = 1;				// Enable Timer2
	}

//------------------------------------------------------------------------------------------TIMER2 Handler For Toggle LED5&7
void TMR2_IRQHandler(void) {	// Timer0 interrupt subroutine
	char lcd2_buffer[18] = "Timer0:";
	Timer2Counter += 1;
	sprintf(lcd2_buffer, "T2:%d(0.8s.)", Timer2Counter);
	print_lcd(1, lcd2_buffer);
	GPIOC->DOUT ^= 0x5000;
	TIMER2->TISR.TIF = 1;    		// Write 1 to clear the interrupt flag 
	}

//------------------------------------------------------------------------------------------Timer3 Count Every 1 s
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
//------------------------------------------------------------------------------------------Timer3_IRQ For Display Everything On LCD
void TMR3_IRQHandler(void) {	// Timer0 interrupt subroutine
	char lcd1_buffer[18] = "Timer3:";
	char lcd2_buffer[18] = "High:";
	char lcd3_buffer[18] = "Low: ";
	char adc_value[15] = "ADC3 Value:";
	
	uint8_t i2cdata = 0;
	i2cdata = Read_A2D_8591();
	out_D2A_8591(i2cdata);
	//disp2Digit8574(i2cdata);
	
	Timer3Counter += 1;
	sprintf(lcd1_buffer+7, " %d s.", Timer3Counter);
	//print_lcd(1, lcd1_buffer);
 	
	/* Display capture values */
	if (CaptureValue[0] >= 1000000) {
		sprintf(lcd2_buffer+5, "%dsec ", CaptureValue[0]/1000000);
		} else if (CaptureValue[0] >= 1000) {
		sprintf(lcd2_buffer+5, "%dmsec ", CaptureValue[0]/1000);
		} else
		sprintf(lcd2_buffer+5, "%dusec ", CaptureValue[0]);
	print_lcd(2, lcd2_buffer);

	if (CaptureValue[1] >= 1000000) {
		sprintf(lcd3_buffer+5, "%dsec ", CaptureValue[1]/1000000);
		} else if (CaptureValue[1] >= 1000) {
		sprintf(lcd3_buffer+5, "%dmsec ", CaptureValue[1]/1000);
		} else
		sprintf(lcd3_buffer+5, "%dusec ", CaptureValue[1]);
	print_lcd(3, lcd3_buffer);
		
	while (ADC->ADSR.ADF == 0);	// A/D Conversion End Flag
	// A status flag that indicates the end of A/D conversion.
		
	ADC->ADSR.ADF = 1;					// This flag can be cleared by writing 1 to self
	sprintf(adc_value+11,"%d   ",ADC->ADDR[3].RSLT);
	//print_lcd(0, adc_value);
	disp2Digit8574_DEC(ADC->ADDR[3].RSLT);
	ADC->ADCR.ADST = 1;
	
	TIMER3->TISR.TIF = 1;    		// Write 1 to clear the interrupt flag 
	}

//------------------------------------------------------------------------------------------UART_Callback
void UART_INT_HANDLE(void) {
	uint8_t bInChar[1] = {0xFF};

	while (UART1->ISR.RDA_IF == 1) {
		DrvUART_Read(UART_PORT1, bInChar, 1);	
		if (comRbytes < 8) {	// check if Buffer is full
			comRbuf[comRbytes] = bInChar[0];
			comRbytes++;
			}
		else if (comRbytes == 8) {
			comRbuf[comRbytes] = bInChar[0];
			comRbytes = 0;
			sprintf(TEXT2+4,"%s",comRbuf);
			print_lcd(0,TEXT2);
			}			
		}
	}

//------------------------------------------------------------------------------------------MAIN
int32_t main() {
	uint8_t  i = 0;
	uint8_t  dataout[9] = "NuMicro0";

	STR_UART_T sParam;

	UNLOCKREG();
  DrvSYS_Open(48000000);
	LOCKREG();
	Initial_pannel();  //call initial pannel function
	clr_all_pannal();
	
	DrvGPIO_Open(E_GPC, 13, E_IO_OUTPUT); // GPC13 pin set to output mode
	DrvGPIO_Open(E_GPC, 15, E_IO_OUTPUT); // GPC15 pin set to output mode
	
	GPC_13 = 0;
	GPC_15 = 0;
	
	print_lcd(0, "Smpl_UART2    ");
	
	InitADC3();
	
	InitPWM1();
	InitCapture2();
	InitTIMER2();
	InitTIMER3();
  
	/* Set UART Pin */
	//DrvGPIO_InitFunction(E_FUNC_UART0);
	DrvGPIO_InitFunction(E_FUNC_UART1);
	DrvGPIO_InitFunction(E_FUNC_UART2);

	/* UART Setting */
	sParam.u32BaudRate = 9600;
	sParam.u8cDataBits = DRVUART_DATABITS_8;
	sParam.u8cStopBits = DRVUART_STOPBITS_1;
	sParam.u8cParity = DRVUART_PARITY_NONE;
	sParam.u8cRxTriggerLevel = DRVUART_FIFO_1BYTES;

	/* Set UART Configuration */
 	//if (DrvUART_Open(UART_PORT0,&sParam) != E_SUCCESS);
	if (DrvUART_Open(UART_PORT1,&sParam) != E_SUCCESS);
	if (DrvUART_Open(UART_PORT2,&sParam) != E_SUCCESS); 

	DrvUART_EnableInt(UART_PORT1, DRVUART_RDAINT, UART_INT_HANDLE);  
	DrvGPIO_InitFunction(E_FUNC_I2C1);
	
	while(1) {
 	  dataout[7] = 0x30 + i;	
		//DrvUART_Write(UART_PORT2, dataout, 8);
		i++;
		if (i >= 10) i = 0;
		sprintf(TEXT1+14, "%d", i);
		//print_lcd(1, TEXT1);
		if (UART_Display_Counter == 20) {
				DrvUART_Write(UART_PORT2, dataout, 8);
				UART_Display_Counter = 0;
			}
		UART_Display_Counter++;
		//DrvSYS_Delay(300000);
		//DrvSYS_Delay(300000);
		//DrvSYS_Delay(300000);
		seg_display(ADC->ADDR[3].RSLT);
	}
	//DrvUART_Close(UART_PORT0);
}
