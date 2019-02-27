// Nu_LB-002: L08_02_UART0_8bytes.c
/*
	connect GPB0 to GPB1

 GPB0 - Rx
 GPB1 - Tx
*/

//
// Smpl_UART0 : while loop for UART0-TX keep transmitting 8 bytes string
//            : IRQ routine for UART0-RX keep receiving 8 bytes string & print to LCD
//						: (TX & RX at the same time)

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

volatile uint8_t comRbuf[16];
volatile uint16_t comRbytes = 0;

char TEXT1[16] = "TX: sending...  ";
char TEXT2[16] = "RX:             ";

//-------------------------------------------------------------UART_Callback
void UART_INT_HANDLE(void) {
	uint8_t bInChar[1] = {0xFF};

	while (UART0->ISR.RDA_IF == 1) {
		DrvUART_Read(UART_PORT0, bInChar, 1);	
		if (comRbytes < 8) {	// check if Buffer is full
			comRbuf[comRbytes] = bInChar[0];
			comRbytes++;
			}
		else if (comRbytes == 8) {
			comRbuf[comRbytes] = bInChar[0];
			comRbytes = 0;
			sprintf(TEXT2+4,"%s",comRbuf);
			print_lcd(2,TEXT2);
			}			
		}
	}

//----------------------------------------------------------------------MAIN
int32_t main() {
	uint8_t  i = 0;
	uint8_t  dataout[9] = "NuMicro0";

	STR_UART_T sParam;

	UNLOCKREG();
  DrvSYS_Open(48000000);
	LOCKREG();
	Initial_pannel();  //call initial pannel function
	clr_all_pannal();
	print_lcd(0, "Smpl_UART0    ");
   	
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

	DrvUART_EnableInt(UART_PORT0, DRVUART_RDAINT, UART_INT_HANDLE);  
	
	while(1) {
 	  dataout[7] = 0x30 + i;	
		DrvUART_Write(UART_PORT0, dataout, 8);
		i++;
		if (i >= 10) i = 0;
		sprintf(TEXT1+14, "%d", i);
		print_lcd(1, TEXT1);
		
		DrvSYS_Delay(300000);
		DrvSYS_Delay(300000);
		DrvSYS_Delay(300000);
	}
	//DrvUART_Close(UART_PORT0);
}

