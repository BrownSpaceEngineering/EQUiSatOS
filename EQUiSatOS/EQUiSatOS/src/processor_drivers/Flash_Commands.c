/*
 * Flash_Commands.c
 *
 * Created: 9/28/2017 19:31:03
 *  Author: mckenna
 */ 

#include "Flash_Commands.h"

void set_lifepo_output_enable(bool enabled) 
{
	set_output(enabled, P_LF_B1_OUTEN);
	set_output(enabled, P_LF_B2_OUTEN);
	
	// should be followed by a delay of at least 2ms before calling flash_leds
}

void flash_leds(void) {	
	// send _falling_ edge on LED_CMD to tell subprocessor to activate LEDs for 100ms
	set_output(false, P_LED_CMD);
	
	// should be followed by a delay of at least 2ms before calling reset_flash_pin
}

void reset_flash_pin(void) 
{
	set_output(true, P_LED_CMD);
}