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
static uint8_t is_running;

static uint8_t watch_block;
SemaphoreHandle_t mutex;

void watchdog_init(void) {
	check_ins = 0;
	is_running = 0;
	watch_block = 0;
	mutex = xSemaphoreCreateMutexStatic(&_watchdog_task_mutex_d);
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
	xSemaphoreTake(mutex, (TickType_t) WATCHDOG_MUTEX_WAIT_TIME_TICKS);
		
	// make sure the battery task is definitely set to run
	eTaskState battery_task_state = eTaskGetState(task_handles[BATTERY_CHARGING_TASK]);
	if (battery_task_state == eDeleted || battery_task_state == eSuspended) {
		watch_block = 1;
	}
	if ((check_ins ^ is_running) > 0 || watch_block == 1) {
		// "kick" watchdog - RESTART SATELLITE
		xSemaphoreGive(mutex);
		return false;
	} else {
		// pet watchdog - pass this watchdog test, move onto next
		is_running = 0;
		xSemaphoreGive(mutex);
		return true;
	}
}

// tasks must check in when launching
void check_in_task(task_type_t task_ind) {
	xSemaphoreTake(mutex, (TickType_t) MUTEX_WAIT_TIME_TICKS);
	check_ins = check_ins | (1 << task_ind); // set this task bit to 1
	xSemaphoreGive(mutex);
}

// tasks must check in while running to avoid the watchdog
// (we'll set 'im loose on that task if it doesn't!)
void report_task_running(task_type_t task_ind) {
	xSemaphoreTake(mutex, (TickType_t) MUTEX_WAIT_TIME_TICKS);
	is_running = is_running | (1 << task_ind);
	xSemaphoreGive(mutex);
}

// tasks must check in when suspending so they don't trip the watchdog
void check_out_task(task_type_t task_ind) {
	xSemaphoreTake(mutex, (TickType_t) MUTEX_WAIT_TIME_TICKS);
	check_ins = check_ins & ~(1 << task_ind); // set this task bit to 0
	xSemaphoreGive(mutex);
}