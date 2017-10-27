/*
 * Flash_Commands.c
 *
 * Created: 9/28/2017 19:31:03
 *  Author: mckenna
 */ 

#include "Flash_Commands.h"

void init_flash(void) {
	// set to write on led pin
	setup_pin(true, P_LED_CMD);
}

void flash_leds(void) {
	// send _falling_ edge on LED_CMD to tell subprocessor to activate LEDs for 100ms
	set_output(false, P_LED_CMD);
	
	// keep it low for a bit (it activates on transition, but give it a buffer)
	// should be more than one clock cycle, but don't want to wait too long
	delay_cycles_us(FLASH_LED_CMD_LOW_TIME); 
	set_output(true, P_LED_CMD);
}