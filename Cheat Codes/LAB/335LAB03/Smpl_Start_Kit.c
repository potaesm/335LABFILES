// LAB335 WEEK 5 BY SUTHINAN & PONGPITH
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.              */
/* edited: FEB 2019: suthinan.musitmani@mail.kmutt.ac.th                    */
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include "NUC1xx.h"
#include "Driver\DrvSYS.h"
#include "Driver\DrvGPIO.h"
#include "LCD_Driver.h"
#include "EEPROM_24LC64.h"
#include "Driver\DrvI2C.h"
#include "scankey.h"
#include "DrvSYS.h"
#include "Seven_Segment.h"
#define scanDelay 4000
#define DELAY300ms	300000 // The maximal delay time is 335000 us.
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

unsigned char SEG_BUF_HEX[16]={SEG_N0, SEG_N1, SEG_N2, SEG_N3, SEG_N4, SEG_N5, SEG_N6, SEG_N7, SEG_N8, SEG_N9, SEG_Na, SEG_Nb, SEG_Nc, SEG_Nd, SEG_Ne, SEG_Nf};

void seg_display(int16_t value) {
	int8_t digit;
	digit = value / 1000;
	close_seven_segment();
	//show_seven_segment(3,digit);
	DrvSYS_Delay(scanDelay);
		
	value = value - digit * 1000;
	digit = value / 100;
	close_seven_segment();
	//show_seven_segment(2,digit);
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

uint8_t HEX2Disp(uint8_t hexNum) {
	static const uint8_t lookUp[16] = {
		0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8,
		0x80, 0x90, 0x88, 0x83, 0xC6, 0xA1, 0x86, 0x8E
		};
	uint8_t hexDisp = lookUp[hexNum];
	return hexDisp;
}

void seg_display1(int16_t value) {
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

void seg_display_Hex(int16_t value) {
  int8_t digit;
			
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
	unsigned char i2cDisplay = 0;
	unsigned char i2cDisplayTemp = 0;
	char lcd0_buffer[18] = "Digit21:XY";

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
	print_lcd(3, "press key1-key8");
	
	// initial keyboard
	for(i=0;i<6;i++)		
	DrvGPIO_Open(E_GPA, i, E_IO_QUASI);
	DrvGPIO_InitFunction(E_FUNC_I2C1);

	while(1) {
	  temp = Scankey();
		if (temp == 1 ) {
			i2cDisplay = 0x9E;
			Write_to_any8574(0x72, HEX2Disp(i2cDisplay >> 4));
			Write_to_any8574(0x70, HEX2Disp(i2cDisplay & 0x0F));
			}
		if (temp == 2) {
			i2cDisplay <<= 4;
			Write_to_any8574(0x72, HEX2Disp(i2cDisplay >> 4));
			Write_to_any8574(0x70, HEX2Disp(i2cDisplay & 0x0F));
			DrvSYS_Delay(DELAY300ms);
			}
		if (temp == 3) {
			i2cDisplay >>= 4;
			Write_to_any8574(0x72, HEX2Disp(i2cDisplay >> 4));
			Write_to_any8574(0x70, HEX2Disp(i2cDisplay & 0x0F));
			DrvSYS_Delay(DELAY300ms);
			}
		if (temp == 4) 
			{
			i2cDisplay >>= 4;
			i2cDisplay <<= 4;	
			i2cDisplay |= 4;
			Write_to_any8574(0x72, HEX2Disp(i2cDisplay >> 4));
			Write_to_any8574(0x70, HEX2Disp(i2cDisplay & 0x0F));
			DrvSYS_Delay(DELAY300ms);
			}
			if (temp == 5) 
			{
			i2cDisplay <<= 4;
			i2cDisplay >>= 4;	
			i2cDisplay |= 0x50;
			Write_to_any8574(0x72, HEX2Disp(i2cDisplay >> 4));
			Write_to_any8574(0x70, HEX2Disp(i2cDisplay & 0x0F));
			DrvSYS_Delay(DELAY300ms);
			}
			if (temp == 6) 
			{
			i2cDisplay >>= 4;
			i2cDisplay <<= 4;	
			i2cDisplay = 0x66;
			Write_to_any8574(0x72, HEX2Disp(i2cDisplay >> 4));
			Write_to_any8574(0x70, HEX2Disp(i2cDisplay & 0x0F));
			DrvSYS_Delay(DELAY300ms);
			}
			if (temp == 7) 
			{
			i2cDisplayTemp = i2cDisplay;
			i2cDisplayTemp <<= 4;
			i2cDisplay >>= 4;
			i2cDisplay |= i2cDisplayTemp;
			sprintf(lcd0_buffer+8, "%X", i2cDisplay);
			clr_all_pannal();
			print_lcd(0, lcd0_buffer);
			Write_to_any8574(0x72, HEX2Disp(i2cDisplay >> 4));
			Write_to_any8574(0x70, HEX2Disp(i2cDisplay & 0x0F));
			DrvSYS_Delay(DELAY300ms);
			}
			if (temp == 8) 
			{
			i2cDisplay += 0x10;
			clr_all_pannal();
			sprintf(lcd0_buffer+8, "%X", i2cDisplay);
			print_lcd(0, lcd0_buffer);
			Write_to_any8574(0x72, HEX2Disp(i2cDisplay >> 4));
			Write_to_any8574(0x70, HEX2Disp(i2cDisplay & 0x0F));
			DrvSYS_Delay(DELAY300ms);
			}
			if (temp == 9) 
			{
			i2cDisplay -= 0x10;
			clr_all_pannal();
			sprintf(lcd0_buffer+8, "%X", i2cDisplay);
			print_lcd(0, lcd0_buffer);
			Write_to_any8574(0x72, HEX2Disp(i2cDisplay >> 4));
			Write_to_any8574(0x70, HEX2Disp(i2cDisplay & 0x0F));
			DrvSYS_Delay(DELAY300ms);
			}
			seg_display_Hex(i2cDisplay);
		}
}
