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
static uint8_t check_ins;
static uint8_t check_outs;
static uint8_t is_running;

static uint8_t watch_block;
SemaphoreHandle_t mutex;

void watchdog_init(void) {
	check_ins = 0;
	check_outs = 0;
	is_running = 0;
	watch_block = 0;
	mutex = xSemaphoreCreateMutex();
}

void watchdog_task(void *pvParameters) {
	TickType_t xNextWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		vTaskDelayUntil( &xNextWakeTime, WATCHDOG_TASK_FREQ / portTICK_PERIOD_MS);
		watchdog_as_function();
	}
	
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}

bool watchdog_as_function(void) {
	xSemaphoreTake(mutex, (TickType_t) MUTEX_WAIT_TIME_TICKS);
	if (/*battery charing task isn't running*/0 == 1) {
		watch_block = 1;
	}
	if ((check_ins ^ check_outs) > 0 || (is_running ^ check_outs) > 0 || (is_running ^ check_ins) > 0 || watch_block == 1) {
		// "kick" watchdog
		check_ins = 0;
		check_outs = 0;
		is_running = 0;
		xSemaphoreGive(mutex);
		return false;
	} else {
		// pet watchdog
		check_ins = 0;
		check_outs = 0;
		is_running = 0;
		xSemaphoreGive(mutex);
		return true;
	}
}

void check_in_task(uint8_t task_ind) {
	xSemaphoreTake(mutex, (TickType_t) MUTEX_WAIT_TIME_TICKS);
	check_ins = check_ins | (1 << task_ind);
	xSemaphoreGive(mutex);
}

void running_task(uint8_t task_ind) {
	xSemaphoreTake(mutex, (TickType_t) MUTEX_WAIT_TIME_TICKS);
	is_running = is_running | (1 << task_ind);
	xSemaphoreGive(mutex);
}

void check_out_task(uint8_t task_ind) {
	xSemaphoreTake(mutex, (TickType_t) MUTEX_WAIT_TIME_TICKS);
	check_outs = check_outs | (1 << task_ind);
	xSemaphoreGive(mutex);
}