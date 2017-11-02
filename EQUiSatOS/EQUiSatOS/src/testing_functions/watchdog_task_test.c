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
	report_task_running(ANTENNA_DEPLOY_TASK);
	check_out_task(ANTENNA_DEPLOY_TASK);
	
	check_in_task(TRANSMIT_TASK);
	report_task_running(TRANSMIT_TASK);
	check_out_task(TRANSMIT_TASK);
	assert(watchdog_as_function());
	
	
	check_in_task(ANTENNA_DEPLOY_TASK);
	report_task_running(ANTENNA_DEPLOY_TASK);
	check_out_task(ANTENNA_DEPLOY_TASK);
	
	check_in_task(BATTERY_CHARGING_TASK);
	report_task_running(BATTERY_CHARGING_TASK);
	check_out_task(BATTERY_CHARGING_TASK);
	
	check_in_task(TRANSMIT_TASK);
	report_task_running(TRANSMIT_TASK);
	check_out_task(TRANSMIT_TASK);
	
	check_in_task(FLASH_ACTIVATE_TASK);
	report_task_running(FLASH_ACTIVATE_TASK);
	check_out_task(FLASH_ACTIVATE_TASK);
	
	check_in_task(CURRENT_DATA_TASK);
	report_task_running(CURRENT_DATA_TASK);
	check_out_task(CURRENT_DATA_TASK);
	
	check_in_task(ATTITUDE_DATA_TASK);
	report_task_running(ATTITUDE_DATA_TASK);
	check_out_task(ATTITUDE_DATA_TASK);
	assert(watchdog_as_function());
	
	
	check_in_task(ANTENNA_DEPLOY_TASK);
	report_task_running(ANTENNA_DEPLOY_TASK);
	check_out_task(ANTENNA_DEPLOY_TASK);
	
	check_in_task(BATTERY_CHARGING_TASK);
	report_task_running(BATTERY_CHARGING_TASK);
	check_out_task(BATTERY_CHARGING_TASK);
	
	check_in_task(TRANSMIT_TASK);
	report_task_running(TRANSMIT_TASK);
	check_out_task(TRANSMIT_TASK);
	
	check_in_task(FLASH_ACTIVATE_TASK);
	report_task_running(FLASH_ACTIVATE_TASK);
	check_out_task(FLASH_ACTIVATE_TASK);
	
	check_in_task(CURRENT_DATA_TASK);
	report_task_running(CURRENT_DATA_TASK);
	check_out_task(CURRENT_DATA_TASK);
	
	check_in_task(ATTITUDE_DATA_TASK);
	assert(!watchdog_as_function());
	
	
	assert(watchdog_as_function());
	
	
	check_in_task(ANTENNA_DEPLOY_TASK);
	report_task_running(ANTENNA_DEPLOY_TASK);
	check_out_task(ANTENNA_DEPLOY_TASK);
	assert(watchdog_as_function());
	
	
	report_task_running(ANTENNA_DEPLOY_TASK);
	assert(!watchdog_as_function());


	check_in_task(ANTENNA_DEPLOY_TASK);
	report_task_running(ANTENNA_DEPLOY_TASK);
	assert(!watchdog_as_function());


	check_out_task(ANTENNA_DEPLOY_TASK);
	report_task_running(ANTENNA_DEPLOY_TASK);
	assert(!watchdog_as_function());
	
	
	check_out_task(ANTENNA_DEPLOY_TASK);
	assert(!watchdog_as_function());
	
	
	check_in_task(ANTENNA_DEPLOY_TASK);
	check_in_task(BATTERY_CHARGING_TASK);
	assert(!watchdog_as_function());
}