// Nu_LB-002: L00_03_7SEG_GPE_0_7_GPC_4_7.c

//
// Smpl_7seg : counting from 0 to 9999 and display on 7-segment LEDs
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

#define DELAY300ms	300000 // The maximal delay time is 335000 us.
#define scanDelay 4000

// display an integer on four 7-segment LEDs
void seg_display(int16_t value) {
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

int32_t main (void) {
	int32_t i = 0;

 	while(1) {
		seg_display(i/10);	// display i on 7-segment display
		i++;								// increment i
		if (i == 100000) i = 0;
		}
	}
