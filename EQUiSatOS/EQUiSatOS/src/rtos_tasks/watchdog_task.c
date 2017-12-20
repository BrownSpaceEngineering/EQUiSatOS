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
static uint16_t check_ins;
static uint16_t is_running;

static bool watch_block;
SemaphoreHandle_t mutex;

void watchdog_init(void) {
	check_ins = 0;
	is_running = 0;
	watch_block = 0;
	mutex = xSemaphoreCreateMutexStatic(&_watchdog_task_mutex_d);
}

void watchdog_task(void *pvParameters) {
	TickType_t prev_wake_time = xTaskGetTickCount();

	for( ;; )
	{
		vTaskDelayUntil( &prev_wake_time, WATCHDOG_TASK_FREQ / portTICK_PERIOD_MS);
		watchdog_as_function();
	}
	
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}

bool watchdog_as_function(void) {
	xSemaphoreTake(mutex, WATCHDOG_MUTEX_WAIT_TIME_TICKS);
	
	// make sure the battery task is definitely set to run
	eTaskState battery_task_state = eTaskGetState(*task_handles[BATTERY_CHARGING_TASK]);
	if (battery_task_state == eDeleted || battery_task_state == eSuspended) {
		watch_block = 1;
	} else if (!check_task_state_consistency()) {
		watch_block = 1;
	}
	if ((check_ins ^ is_running) > 0 || watch_block == 1) {
		// "kick" watchdog - RESTART SATELLITE
		trace_print("Watchdog kicked - RESTARTING Satellite");
		xSemaphoreGive(mutex);
		// it doesn't make sense to reset values here because the satellite will reboot
		return false;
	} else {
		trace_print("Pet watchdog");
		// pet watchdog - pass this watchdog test, move onto next
		is_running = 0;
		xSemaphoreGive(mutex);
		return true;
	}
}

/************************************************************************/
// NOTE: the watchdog is to tighly coupled with the satellite state that
// those state handling functions need to lock it before changing state 
// (this is also because we suspend the scheduler during state changes,	
// and mutexes cannot be used without the scheduler running.)           
// Also, the check_in_task_unsafe and check_out_task_unsafe should only 
// be called when the watchdog mutex has been locked (even if the		
// scheduler is suspended, a blocked task may still come in after that and
// overwrite what was done)
/************************************************************************/

// These MUST be called when using check_in_task_unsafe and check_out_take_unsafe
void watchdog_mutex_take(void) {
	xSemaphoreTake(mutex, WATCHDOG_MUTEX_WAIT_TIME_TICKS);
}

void watchdog_mutex_give(void) {
	xSemaphoreGive(mutex);
}

// tasks must check in when resuming from suspension or launching
// NOTE: not safe to call without having gotten mutex
void check_in_task_unsafe(task_type_t task_ind) {
	check_ins = check_ins | (1 << task_ind); // set this task bit to 1
	// set the running bit to 1 until next reset (we have to give the task the
	// benefit of the doubt because it may take a while to initially start)
	is_running = is_running | (1 << task_ind);
}

// tasks must check in while running to avoid the watchdog
// (we'll set 'im loose on that task if it doesn't!)
void report_task_running(task_type_t task_ind) {
	xSemaphoreTake(mutex, WATCHDOG_MUTEX_WAIT_TIME_TICKS);
	is_running = is_running | (1 << task_ind);
	xSemaphoreGive(mutex);
}

// tasks must check out when suspending so they don't trip the watchdog
// NOTE: not safe to call without having gotten mutex
void check_out_task_unsafe(task_type_t task_ind) {
	check_ins = check_ins & ~(1 << task_ind); // set this task bit to 0
	// set the running bit to 0 as well, to avoid a watchdog reset on the next
	// iteration (before the watchdog can clear is_running)
	is_running = is_running & ~(1 << task_ind);
}