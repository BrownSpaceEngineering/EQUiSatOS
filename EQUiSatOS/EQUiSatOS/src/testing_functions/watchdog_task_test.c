/*
 * watchdog_task_test.c
 *
 * Created: 3/5/2017 2:04:59 PM
 *  Author: jleiken
 */ 

#include "watchdog_task_test.h"

void watch_test(void) {
	check_in_task(ANTENNA_DEPLOY_SHIFT);
	running_task(ANTENNA_DEPLOY_SHIFT);
	check_in_task(TRANSMIT_SHIFT);
	running_task(TRANSMIT_SHIFT);
	assert(watchdog_as_function());
	
	check_in_task(ANTENNA_DEPLOY_SHIFT);
	running_task(ANTENNA_DEPLOY_SHIFT);
	check_in_task(BATTERY_CHARGING_SHIFT);
	running_task(BATTERY_CHARGING_SHIFT);
	check_in_task(TRANSMIT_SHIFT);
	running_task(TRANSMIT_SHIFT);
	check_in_task(FLASH_ACTIVATE_SHIFT);
	running_task(FLASH_ACTIVATE_SHIFT);
	check_in_task(CURRENT_DATA_SHIFT);
	running_task(CURRENT_DATA_SHIFT);
	check_in_task(ATTITUDE_DATA_SHIFT);
	running_task(ATTITUDE_DATA_SHIFT);
	assert(watchdog_as_function());
	
	check_in_task(ANTENNA_DEPLOY_SHIFT);
	running_task(ANTENNA_DEPLOY_SHIFT);
	check_in_task(BATTERY_CHARGING_SHIFT);
	running_task(BATTERY_CHARGING_SHIFT);
	check_in_task(TRANSMIT_SHIFT);
	running_task(TRANSMIT_SHIFT);
	check_in_task(FLASH_ACTIVATE_SHIFT);
	running_task(FLASH_ACTIVATE_SHIFT);
	check_in_task(CURRENT_DATA_SHIFT);
	running_task(CURRENT_DATA_SHIFT);
	check_in_task(ATTITUDE_DATA_SHIFT);
	assert(!watchdog_as_function());
	
	assert(watchdog_as_function());
	
	check_in_task(ANTENNA_DEPLOY_SHIFT);
	running_task(ANTENNA_DEPLOY_SHIFT);
	assert(watchdog_as_function());
	
	running_task(ANTENNA_DEPLOY_SHIFT);
	assert(!watchdog_as_function());
	
	check_in_task(ANTENNA_DEPLOY_SHIFT);
	check_in_task(BATTERY_CHARGING_SHIFT);
	assert(!watchdog_as_function());
}