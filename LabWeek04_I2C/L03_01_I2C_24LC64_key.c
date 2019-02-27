// Nu_LB-002: L03_01_I2C_24LC64_key.c

// need to add ScanKey.c to the project
// from "C:\Nuvoton\BSP Library\NUC100SeriesBSP_CMSIS_v1.05.003\NuvotonPlatform_Keil\Src\NUC1xx-LB_002\ScanKey.c"

// add DrvI2C.c (..\..\..\Src\Driver\DrvI2C.c)
// add EEPROM_24LC64.c (..\..\..\Src\NUC1xx-LB_002\EEPROM_24LC64.c)

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

int main(void) {
	uint32_t i2cdata = 0, i;
	unsigned char temp;
	char addr[16] = "Address:";
	char Write[16] = "Write:";
	char read[16] = "Read:";

	UNLOCKREG();	/* Unlock the protected registers */	
	DrvSYS_SetOscCtrl(E_SYS_XTL12M, 1); /* Enable the 12MHz oscillator */
	SysTimerDelay(5000); /* Waiting for 12M Xtal stalble */
	/* HCLK clock source. 0: external 12MHz; 4:internal 22MHz RC oscillator */
	DrvSYS_SelectHCLKSource(0);		
	LOCKREG(); /*lock the protected registers */	
	DrvSYS_SetClockDivider(E_SYS_HCLK_DIV, 0); 
	/* HCLK clock frequency = HCLK clock source / (HCLK_N + 1) */

	Initial_pannel();  //call initial pannel function
	clr_all_pannal();
	print_lcd(0, "I2C with 24LC65");
	print_lcd(1, "test read and  ");
	print_lcd(2, "write function ");	  
	print_lcd(3, "press key1-key9");
	
	//initial key board
	for(i=0;i<6;i++)		
		DrvGPIO_Open(E_GPA, i, E_IO_QUASI);
	
	DrvGPIO_InitFunction(E_FUNC_I2C1);
	
	while (1) {
		temp = Scankey();
		if (temp==1) {
			print_lcd(0,"Key1 had pressed ");
		  print_lcd(1,"               ");
		  print_lcd(2,"               ");
			print_lcd(3,"               ");
		 	Write_24LC64(0x00000000+temp,temp+11);
			i2cdata= Read_24LC64(0x00000000+temp);
		  sprintf(addr+8,"%x",temp);
			sprintf(Write+6,"%x",temp+11);
			sprintf(read+5,"%x",i2cdata);
			print_lcd(1,addr);
			print_lcd(2,Write);
		  print_lcd(3,read);	
			}
		if (temp==2) {
			print_lcd(0,"Key2 had pressed ");
		  print_lcd(1,"               ");
		  print_lcd(2,"               ");
			print_lcd(3,"               ");
			print_lcd(3,"               "); 	  	  
		 	Write_24LC64(0x00000000+temp,temp+22);
			i2cdata= Read_24LC64(0x00000000+temp);
		  sprintf(addr+8,"%x",temp);
			sprintf(Write+6,"%x",temp+22);
			sprintf(read+5,"%x",i2cdata);
			print_lcd(1,addr);
			print_lcd(2,Write);
		  print_lcd(3,read);
			}
		if (temp==3) {
			print_lcd(0,"Key3 had pressed ");
		  print_lcd(1,"               ");
		  print_lcd(2,"               ");
			print_lcd(3,"               ");	  	  
		 	Write_24LC64(0x00000000+temp,temp+33);
			i2cdata= Read_24LC64(0x00000000+temp);
		  sprintf(addr+8,"%x",temp);
			sprintf(Write+6,"%x",temp+33);
			sprintf(read+5,"%x",i2cdata);
			print_lcd(1,addr);
			print_lcd(2,Write);
		  print_lcd(3,read);
			}
		if (temp==4) {
			print_lcd(0,"Key4 had pressed ");
			print_lcd(1,"               ");
		  print_lcd(2,"               ");
			print_lcd(3,"               ");  	  
		 	Write_24LC64(0x00000000+temp,temp+44);
			i2cdata= Read_24LC64(0x00000000+temp);
		  sprintf(addr+8,"%x",temp);
			sprintf(Write+6,"%x",temp+44);
			sprintf(read+5,"%x",i2cdata);
			print_lcd(1,addr);
			print_lcd(2,Write);
		  print_lcd(3,read);
			}
		if (temp==5) {
			print_lcd(0,"Key5 had pressed ");
		  print_lcd(1,"               ");
		  print_lcd(2,"               ");	
			print_lcd(3,"               ");  	  
		 	Write_24LC64(0x00000000+temp,temp+55);
			i2cdata= Read_24LC64(0x00000000+temp);
		  sprintf(addr+8,"%x",temp);
			sprintf(Write+6,"%x",temp+55);
			sprintf(read+5,"%x",i2cdata);
			print_lcd(1,addr);
			print_lcd(2,Write);
		  print_lcd(3,read);
			}
		if (temp==6) {
			print_lcd(0,"Key6 had pressed ");
		  print_lcd(1,"               ");
		  print_lcd(2,"               ");
			print_lcd(3,"               ");	  	  
		 	Write_24LC64(0x00000000+temp,temp+66);
			i2cdata= Read_24LC64(0x00000000+temp);
		  sprintf(addr+8,"%x",temp);
			sprintf(Write+6,"%x",temp+66);
			sprintf(read+5,"%x",i2cdata);
			print_lcd(1,addr);
			print_lcd(2,Write);
		  print_lcd(3,read);
	    }
		if (temp==7) {
			print_lcd(0,"Key7 had pressed ");
		  print_lcd(1,"               ");
		  print_lcd(2,"               ");
			print_lcd(3,"               ");  	  
		 	Write_24LC64(0x00000000+temp,temp+77);
			i2cdata= Read_24LC64(0x00000000+temp);
		  sprintf(addr+8,"%x",temp);
			sprintf(Write+6,"%x",temp+77);
			sprintf(read+5,"%x",i2cdata);
			print_lcd(1,addr);
			print_lcd(2,Write);
		  print_lcd(3,read);
			}
		if (temp==8) {
			print_lcd(0,"Key8 had pressed");
		  print_lcd(1,"               ");
		  print_lcd(2,"               ");
			print_lcd(3,"               "); 	  	  
		 	Write_24LC64(0x00000000+temp,temp+88);
			i2cdata= Read_24LC64(0x00000000+temp);
		  sprintf(addr+8,"%x",temp);
			sprintf(Write+6,"%x",temp+88);
			sprintf(read+5,"%x",i2cdata);
			print_lcd(1,addr);
			print_lcd(2,Write);
		  print_lcd(3,read);
			}
		if (temp==9) {
			print_lcd(0,"Key9 had pressed");
		  print_lcd(1,"               ");
		  print_lcd(2,"               ");
			print_lcd(3,"               ");  	  
		 	Write_24LC64(0x00000000+temp,temp+99);
			i2cdata= Read_24LC64(0x00000000+temp);
		  sprintf(addr+8,"%x",temp);
			sprintf(Write+6,"%x",temp+99);
			sprintf(read+5,"%x",i2cdata);
			print_lcd(1,addr);
			print_lcd(2,Write);
		  print_lcd(3,read);
			}
		}
	}
