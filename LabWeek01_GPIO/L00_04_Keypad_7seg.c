// Nu_LB-002: L00_04_Keypad_7seg.c

// need to add ScanKey.c to the project
// from "C:\Nuvoton\BSP Library\NUC100SeriesBSP_CMSIS_v1.05.003\NuvotonPlatform_Keil\Src\NUC1xx-LB_002\ScanKey.c"

//
// Smpl_7seg_keypad
//
// Input:  3x3 keypad (input = 1~9 when key is pressed, =0 when key is not pressed
// Output: 7-segment LEDs
//
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.              */
/* edited: june 2014: dejwoot.kha@mail.kmutt.ac.th                          */
/*--------------------------------------------------------------------------*/

#include <stdio.h>																											 
#include "NUC1xx.h"
#include "DrvSYS.h"
#include "Seven_Segment.h"
#include "scankey.h"

int32_t main (void) {
	int8_t number;

	OpenKeyPad();					 	
	 
	while(1) {
	  number = Scankey();           // scan keypad to get a number (1~9)
		show_seven_segment(0,number); // display number on 7-segment LEDs
		DrvSYS_Delay(5000);           // delay time for keeping 7-segment display 
		close_seven_segment();	      // turn off 7-segment LEDs								 
		}
	}
