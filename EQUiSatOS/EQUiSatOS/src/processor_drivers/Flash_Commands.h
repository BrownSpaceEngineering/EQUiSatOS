/*
 * Flash_Commands.h
 *
 * Created: 9/28/2017 19:30:35
 *  Author: mckenna
 */


#ifndef FLASH_COMMANDS_H_
#define FLASH_COMMANDS_H_

#include <global.h>
#include <proc_pins.h>
#include "Direct_Pin_Commands.h"
#include "rtos_tasks/battery_charging_task.h"

void set_lifepo_output_enable(bool enabled);
void set_lifepo_charge_disable(void);
void set_lifepo_charge_enable(void);
void flash_arm(void); // to be called at least 2ms before flashing
void flash_activate(void);
void flash_disarm(void); // to be called a minimum of 2ms after flashing

#endif /* FLASH_COMMANDS_H_ */
