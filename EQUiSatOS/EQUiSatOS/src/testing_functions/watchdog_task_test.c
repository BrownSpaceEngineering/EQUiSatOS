/*
 * watchdog_task_test.c
 *
 * Created: 3/5/2017 2:04:59 PM
 *  Author: jleiken
 */ 

#include "watchdog_task_test.h"

void watch_test(void) {
	init_watchdog_task();
	
	check_in_task_unsafe(ANTENNA_DEPLOY_TASK);
	report_task_running(ANTENNA_DEPLOY_TASK);
	check_out_task_unsafe(ANTENNA_DEPLOY_TASK);
	
	check_in_task_unsafe(TRANSMIT_TASK);
	report_task_running(TRANSMIT_TASK);
	check_out_task_unsafe(TRANSMIT_TASK);
	assert(watchdog_as_function());
	
	
	check_in_task_unsafe(ANTENNA_DEPLOY_TASK);
	report_task_running(ANTENNA_DEPLOY_TASK);
	check_out_task_unsafe(ANTENNA_DEPLOY_TASK);
	
	check_in_task_unsafe(BATTERY_CHARGING_TASK);
	report_task_running(BATTERY_CHARGING_TASK);
	check_out_task_unsafe(BATTERY_CHARGING_TASK);
	
	check_in_task_unsafe(TRANSMIT_TASK);
	report_task_running(TRANSMIT_TASK);
	check_out_task_unsafe(TRANSMIT_TASK);
	
	check_in_task_unsafe(FLASH_ACTIVATE_TASK);
	report_task_running(FLASH_ACTIVATE_TASK);
	check_out_task_unsafe(FLASH_ACTIVATE_TASK);
	
	check_in_task_unsafe(IDLE_DATA_TASK);
	report_task_running(IDLE_DATA_TASK);
	check_out_task_unsafe(IDLE_DATA_TASK);
	
	check_in_task_unsafe(ATTITUDE_DATA_TASK);
	report_task_running(ATTITUDE_DATA_TASK);
	check_out_task_unsafe(ATTITUDE_DATA_TASK);
	assert(watchdog_as_function());
	
	
	check_in_task_unsafe(ANTENNA_DEPLOY_TASK);
	report_task_running(ANTENNA_DEPLOY_TASK);
	check_out_task_unsafe(ANTENNA_DEPLOY_TASK);
	
	check_in_task_unsafe(BATTERY_CHARGING_TASK);
	report_task_running(BATTERY_CHARGING_TASK);
	check_out_task_unsafe(BATTERY_CHARGING_TASK);
	
	check_in_task_unsafe(TRANSMIT_TASK);
	report_task_running(TRANSMIT_TASK);
	check_out_task_unsafe(TRANSMIT_TASK);
	
	check_in_task_unsafe(FLASH_ACTIVATE_TASK);
	report_task_running(FLASH_ACTIVATE_TASK);
	check_out_task_unsafe(FLASH_ACTIVATE_TASK);
	
	check_in_task_unsafe(IDLE_DATA_TASK);
	report_task_running(IDLE_DATA_TASK);
	check_out_task_unsafe(IDLE_DATA_TASK);
	
	check_in_task_unsafe(ATTITUDE_DATA_TASK);
	assert(!watchdog_as_function());
	
	
	assert(watchdog_as_function());
	
	
	check_in_task_unsafe(ANTENNA_DEPLOY_TASK);
	report_task_running(ANTENNA_DEPLOY_TASK);
	check_out_task_unsafe(ANTENNA_DEPLOY_TASK);
	assert(watchdog_as_function());
	
	
	report_task_running(ANTENNA_DEPLOY_TASK);
	assert(!watchdog_as_function());


	check_in_task_unsafe(ANTENNA_DEPLOY_TASK);
	report_task_running(ANTENNA_DEPLOY_TASK);
	assert(!watchdog_as_function());


	check_out_task_unsafe(ANTENNA_DEPLOY_TASK);
	report_task_running(ANTENNA_DEPLOY_TASK);
	assert(!watchdog_as_function());
	
	
	check_out_task_unsafe(ANTENNA_DEPLOY_TASK);
	assert(!watchdog_as_function());
	
	
	check_in_task_unsafe(ANTENNA_DEPLOY_TASK);
	check_in_task_unsafe(BATTERY_CHARGING_TASK);
	assert(!watchdog_as_function());
}