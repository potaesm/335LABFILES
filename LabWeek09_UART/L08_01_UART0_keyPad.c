// Nu_LB-002: L08_01_UART0_keyPad.c
/* 
connect GPB0 to GPB1

UART0:GPB0 - Rx
			GPB1 - Tx
UART1:GPB4 - Rx
			GPB5 - Tx
UART2:GPD14 - Rx
			GPD15 - Tx			
*/

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.              */
/* edited: june 2014: dejwoot.kha@mail.kmutt.ac.th                          */
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include "Driver\DrvUART.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvSYS.h"
#include "NUC1xx.h"
#include "NUC1xx-LB_002\LCD_Driver.h"
#include "scankey.h"

#define DELAY300ms	300000 // The maximal delay time is 335000 us.

volatile uint8_t comRbuf[16];
volatile uint16_t comRbytes = 0;

char TEXT2[16] = "RX:             ";

//-------------------------------------------------------------UART_Callback
void UART0_INT_HANDLE(void) {
	uint8_t bInChar[1] = {0xFF};

	while (UART0->ISR.RDA_IF == 1) {	// Receive Data Available Interrupt Flag
		DrvUART_Read(UART_PORT0,bInChar, 1);	
		if (comRbytes < 2) { // check if Buffer is full
			comRbuf[comRbytes] = bInChar[0];
			comRbytes++;
			}
		else if (comRbytes == 2) {
			comRbuf[comRbytes] = bInChar[0];
			comRbytes = 0;
			sprintf(TEXT2+4, "%s", comRbuf);
			print_lcd(2, TEXT2);
			}
		}
	}

//----------------------------------------------------------------------MAIN
int32_t main() {
	int8_t number;
	uint8_t LCDcolumn = 1;
	uint8_t dataout[1] = "1";

	STR_UART_T sParam;

	UNLOCKREG();
	DrvSYS_Open(48000000);
	LOCKREG();
	
	Initial_pannel();  //call initial pannel function
	clr_all_pannal();
	print_lcd(0,"Smpl_UART0    ");
   	
	/* Set UART Pin */
	DrvGPIO_InitFunction(E_FUNC_UART0);		

	/* UART Setting */
	sParam.u32BaudRate = 9600;
	sParam.u8cDataBits = DRVUART_DATABITS_8;
	sParam.u8cStopBits = DRVUART_STOPBITS_1;
	sParam.u8cParity = DRVUART_PARITY_NONE;
	sParam.u8cRxTriggerLevel = DRVUART_FIFO_1BYTES;

	/* Set UART Configuration */
 	if (DrvUART_Open(UART_PORT0,&sParam) != E_SUCCESS);  

	DrvUART_EnableInt(UART_PORT0, DRVUART_RDAINT, UART0_INT_HANDLE);  
	
	while (1) {
		number = Scankey();
		
		if (number == 1) {
			dataout[0] = 0x31;
			DrvUART_Write(UART_PORT0, dataout,1);

			Show_Word(1,LCDcolumn,'1');
			Show_Word(1,LCDcolumn+1,' ');
			LCDcolumn++;
				if (LCDcolumn > 14) LCDcolumn = 1;			
			DrvSYS_Delay(DELAY300ms);
			}
		if (number == 2) {
			dataout[0] = 0x32;
			DrvUART_Write(UART_PORT0, dataout,1);

			Show_Word(1,LCDcolumn,'2');
			Show_Word(1,LCDcolumn+1,' ');
			LCDcolumn++;
				if (LCDcolumn > 14) LCDcolumn = 1;			
			DrvSYS_Delay(DELAY300ms);			
			}			
		if (number == 3) {
			dataout[0] = 0x33;
			DrvUART_Write(UART_PORT0, dataout,1);

			Show_Word(1,LCDcolumn,'3');
			Show_Word(1,LCDcolumn+1,' ');
			LCDcolumn++;
				if (LCDcolumn > 14) LCDcolumn = 1;			
			DrvSYS_Delay(DELAY300ms);	
			}
		if (number == 4) {
			dataout[0] = 0x34;
			DrvUART_Write(UART_PORT0, dataout,1);

			Show_Word(1,LCDcolumn,'4');
			Show_Word(1,LCDcolumn+1,' ');
			LCDcolumn++;
				if (LCDcolumn > 14) LCDcolumn = 1;			
			DrvSYS_Delay(DELAY300ms);	
			}
		if (number == 5) {
			dataout[0] = 0x35;
			DrvUART_Write(UART_PORT0, dataout,1);

			Show_Word(1,LCDcolumn,'5');
			Show_Word(1,LCDcolumn+1,' ');
			LCDcolumn++;
				if (LCDcolumn > 14) LCDcolumn = 1;			
			DrvSYS_Delay(DELAY300ms);
			}	
		}
	//DrvUART_Close(UART_PORT0);
	}
