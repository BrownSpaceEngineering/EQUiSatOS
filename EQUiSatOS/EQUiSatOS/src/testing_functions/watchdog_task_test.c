/*
 * watchdog_task_test.c
 *
 * Created: 3/5/2017 2:04:59 PM
 *  Author: jleiken
 */ 

#include "watchdog_task_test.h"

void watch_test(void) {
	watchdog_init();
	
	check_in_task(ANTENNA_DEPLOY_TASK);
	running_task(ANTENNA_DEPLOY_TASK);
	check_out_task(ANTENNA_DEPLOY_TASK);
	
	check_in_task(TRANSMIT_TASK);
	running_task(TRANSMIT_TASK);
	check_out_task(TRANSMIT_TASK);
	assert(watchdog_as_function());
	
	
	check_in_task(ANTENNA_DEPLOY_TASK);
	running_task(ANTENNA_DEPLOY_TASK);
	check_out_task(ANTENNA_DEPLOY_TASK);
	
	check_in_task(BATTERY_CHARGING_TASK);
	running_task(BATTERY_CHARGING_TASK);
	check_out_task(BATTERY_CHARGING_TASK);
	
	check_in_task(TRANSMIT_TASK);
	running_task(TRANSMIT_TASK);
	check_out_task(TRANSMIT_TASK);
	
	check_in_task(FLASH_ACTIVATE_TASK);
	running_task(FLASH_ACTIVATE_TASK);
	check_out_task(FLASH_ACTIVATE_TASK);
	
	check_in_task(CURRENT_DATA_TASK);
	running_task(CURRENT_DATA_TASK);
	check_out_task(CURRENT_DATA_TASK);
	
	check_in_task(ATTITUDE_DATA_TASK);
	running_task(ATTITUDE_DATA_TASK);
	check_out_task(ATTITUDE_DATA_TASK);
	assert(watchdog_as_function());
	
	
	check_in_task(ANTENNA_DEPLOY_TASK);
	running_task(ANTENNA_DEPLOY_TASK);
	check_out_task(ANTENNA_DEPLOY_TASK);
	
	check_in_task(BATTERY_CHARGING_TASK);
	running_task(BATTERY_CHARGING_TASK);
	check_out_task(BATTERY_CHARGING_TASK);
	
	check_in_task(TRANSMIT_TASK);
	running_task(TRANSMIT_TASK);
	check_out_task(TRANSMIT_TASK);
	
	check_in_task(FLASH_ACTIVATE_TASK);
	running_task(FLASH_ACTIVATE_TASK);
	check_out_task(FLASH_ACTIVATE_TASK);
	
	check_in_task(CURRENT_DATA_TASK);
	running_task(CURRENT_DATA_TASK);
	check_out_task(CURRENT_DATA_TASK);
	
	check_in_task(ATTITUDE_DATA_TASK);
	assert(!watchdog_as_function());
	
	
	assert(watchdog_as_function());
	
	
	check_in_task(ANTENNA_DEPLOY_TASK);
	running_task(ANTENNA_DEPLOY_TASK);
	check_out_task(ANTENNA_DEPLOY_TASK);
	assert(watchdog_as_function());
	
	
	running_task(ANTENNA_DEPLOY_TASK);
	assert(!watchdog_as_function());


	check_in_task(ANTENNA_DEPLOY_TASK);
	running_task(ANTENNA_DEPLOY_TASK);
	assert(!watchdog_as_function());


	check_out_task(ANTENNA_DEPLOY_TASK);
	running_task(ANTENNA_DEPLOY_TASK);
	assert(!watchdog_as_function());
	
	
	check_out_task(ANTENNA_DEPLOY_TASK);
	assert(!watchdog_as_function());
	
	
	check_in_task(ANTENNA_DEPLOY_TASK);
	check_in_task(BATTERY_CHARGING_TASK);
	assert(!watchdog_as_function());
}