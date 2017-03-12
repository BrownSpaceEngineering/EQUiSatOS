/*
 * Watchdog_Task.c
 *
 * Created: 3/1/2017 9:26:57 PM
 *  Author: jleiken
 *
 * Make sure to move into rtos_tasks.c upon branch merge
 * When we do this, put it at priority 
 */ 

#include "Watchdog_Task.h"
uint8_t check_ins = 0;
uint8_t is_running = 0;

static uint8_t watch_block = 0;

void watchdog_task(void *pvParameters) {
	TickType_t xNextWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		vTaskDelayUntil( &xNextWakeTime, WATCHDOG_TASK_FREQ / portTICK_PERIOD_MS);
		if (/*battery charing task isn't running*/0 == 1) {
			watch_block = 1;
		}
		if ((check_ins ^ is_running) > 0 || watch_block == 1) {
			check_ins = 0;
			is_running = 0;
			// "kick" watchdog
		} else {
			// pet watchdog
			check_ins = 0;
			is_running = 0;
		}
	}
	
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}

int watchdog_as_function(void) {
	if (/*battery charing task isn't running*/0 == 1) {
		watch_block = 1;
	}
	if ((check_ins ^ is_running) > 0 || watch_block == 1) {
		// "kick" watchdog
		check_ins = 0;
		is_running = 0;
		return 0;
	} else {
		// pet watchdog
		check_ins = 0;
		is_running = 0;
		return 1;
	}
}

void check_in_task(uint8_t task_ind) {
	check_ins = check_ins | (1 << task_ind);
}

void running_task(uint8_t task_ind) {
	is_running = is_running | (1 << task_ind);
}