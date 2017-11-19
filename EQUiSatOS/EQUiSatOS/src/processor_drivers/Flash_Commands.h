/*
 * Flash_Commands.h
 *
 * Created: 9/28/2017 19:30:35
 *  Author: mckenna
 */ 

#include <global.h>
#include <proc_pins.h>
#include "Direct_Pin_Commands.h"

#ifndef FLASH_COMMANDS_H_
#define FLASH_COMMANDS_H_

#define FLASH_LED_CMD_LOW_TIME 1000 // us;

void flash_leds(void);

#endif /* FLASH_COMMANDS_H_ */