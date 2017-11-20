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

void set_lifepo_output_enable(bool enabled);
void flash_leds(void);
void reset_flash_pin(void);

#endif /* FLASH_COMMANDS_H_ */