/*
 * button.c
 *
 *  Created on: Jul 18, 2016
 *      Author: Eric Middleton, Zhao Zhang, Chad Nelson, & Zachary Glanz.
 *
 *  @edit: Lindsey Sleeth and Sam Stifter on 02/04/2019
 *  @edit: Phillip Jones 05/30/2019: Merged Spring 2019 version with Fall 2018
 *  @edit: Diane Rover 02/01/20: Corrected comments about ordering of switches for new LCD board and added busy-wait on PRGPIO
 */

//The buttons are on PORTE 3:0
// GPIO_PORTE_DATA_R -- Name of the memory mapped register for GPIO Port E,
// which is connected to the push buttons
#include "button.h"


/**
 * Initialize PORTE and configure bits 0-3 to be used as inputs for the buttons.
 */
void button_init()
{
	static uint8_t initialized = 0;

	//Check if already initialized
	if(initialized)
	{
		return;
	}

	//enable clock port for Port E
	SYSCTL_RCGCGPIO_R |= 0x10;
	
	//wait for Port E to be ready
	while ((SYSCTL_PRGPIO_R & 0x10) == 0);

	//set GPIO Port E directions pins 0 - 3 to input (clear bits 0 - 3)
	GPIO_PORTE_DIR_R &= ~0x0F;

    //enable digital functions for GPIO Port E pins 0 - 3
    GPIO_PORTE_DEN_R |= 0x0F;

	initialized = 1;
}

/**
 * Returns the position of the rightmost button being pushed.
 * @return the position of the rightmost button being pushed. 1 is the leftmost button, 4 is the rightmost button.  0 indicates no button being pressed
 */
uint8_t button_getButton()
{

    uint8_t buttons = GPIO_PORTE_DATA_R & 0xF;

    if ((buttons | 0x7) == 0x7) return 4;
    else if ((buttons | 0xB) == 0xB) return 3;
    else if ((buttons | 0xD) == 0xD) return 2;
    else if ((buttons | 0xE) == 0xE) return 1;

	return 0;
}
