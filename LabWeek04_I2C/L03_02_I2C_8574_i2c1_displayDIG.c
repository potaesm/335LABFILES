// Nu_LB-002: L03_02_I2C_8574_i2c1_displayDIG.c
/*	
	key '1'-'4'
	key '3' count up 

	connected to 8574
	GPA10 - SDA
	GPA11 - SCL
	
	addr 0x70 the right 7SEG
	addr 0x72 the left 7SEG
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
#include "EEPROM_24LC64.h"
#include "Driver\DrvI2C.h"
#include "scankey.h"

#define DELAY300ms	300000 // The maximal delay time is 335000 us.

uint8_t HEX2Disp(uint8_t hexNum) {
	static const uint8_t lookUp[16] = {
		0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8,
		0x80, 0x90, 0x88, 0x83, 0xC6, 0xA1, 0x86, 0x8E
		};
	uint8_t hexDisp = lookUp[hexNum];
	return hexDisp;
	}

void Write_to_any8574(uint8_t i2c_addr, uint8_t data) {
	uint32_t i;
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
	
	for(i=0;i<60;i++);
	DrvI2C_Close(I2C_PORT1);
	for(i=0;i<6000;i++);
	for(i=0;i<6000;i++);
	}

int main(void) {
	unsigned char temp, i;

	/* Unlock the protected registers */	
	UNLOCKREG();
  /* Enable the 12MHz oscillator oscillation */
	DrvSYS_SetOscCtrl(E_SYS_XTL12M, 1);
	/* Waiting for 12M Xtal to stable */
	SysTimerDelay(5000);
	/* HCLK clock source. 0: external 12MHz; 4:internal 22MHz RC oscillator */
	DrvSYS_SelectHCLKSource(0);
	/*lock the protected registers */
	LOCKREG();				
	/* HCLK clock frequency = HCLK clock source / (HCLK_N + 1) */
	DrvSYS_SetClockDivider(E_SYS_HCLK_DIV, 0); 

	Initial_pannel();  // call initial panel function
	clr_all_pannal();
	
	print_lcd(0, "I2C with       ");
	print_lcd(1, "       24LC65  ");
	print_lcd(2, "       PCF8574 ");	  
	print_lcd(3, "press key1-key4");
	
	// initial keyboard
	for(i=0;i<6;i++)		
		DrvGPIO_Open(E_GPA, i, E_IO_QUASI);

	DrvGPIO_InitFunction(E_FUNC_I2C1);

	while(1) {
	  temp=Scankey();
		if (temp == 1 ) {
			print_lcd(0, "Key1 had pressed ");
			Write_to_any8574(0x70, 1);
			}
		if (temp == 2) {
			print_lcd(0, "Key2 had pressed ");
			Write_to_any8574(0x70, 0xFF);
			}
		if (temp == 3) {
			print_lcd(0, "Key3 had pressed ");

			for(i=0;i<16;i++) {
				Write_to_any8574(0x72, HEX2Disp(i));
				DrvSYS_Delay(DELAY300ms);	   	// delay
				}
			}
		if (temp == 4) {
			print_lcd(0, "Key4 had pressed ");
			Write_to_any8574(0x72, 0xFF);
			}
		}
	}
