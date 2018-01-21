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

uint8_t WATCHDOG_ALLOWED_TIMES_MS[NUM_TASKS] = {
	WATCHDOG_TASK_FREQ + WATCHDOG_BUFFER,
	STATE_HANDLING_TASK_FREQ + WATCHDOG_BUFFER,
	ANTENNA_DEPLOY_TASK_FREQ + WATCHDOG_BUFFER,
	BATTERY_CHARGING_TASK_FREQ + WATCHDOG_BUFFER,
	TRANSMIT_TASK_FREQ + WATCHDOG_BUFFER,
	FLASH_ACTIVATE_TASK_FREQ + WATCHDOG_BUFFER,
	IDLE_DATA_TASK_FREQ + WATCHDOG_BUFFER,
	LOW_POWER_DATA_TASK_FREQ + WATCHDOG_BUFFER,
	ATTITUDE_DATA_TASK_FREQ + WATCHDOG_BUFFER,
	PERSISTENT_DATA_BACKUP_TASK_FREQ + WATCHDOG_BUFFER
};

static bool check_ins[NUM_TASKS];
static uint32_t running_times[NUM_TASKS];

SemaphoreHandle_t mutex;

void watchdog_init(void) {
	memset(&check_ins, 0, sizeof(bool) * NUM_TASKS);
	memset(&running_times, 0, sizeof(uint32_t) * NUM_TASKS);
	mutex = xSemaphoreCreateMutexStatic(&_watchdog_task_mutex_d);
}

void watchdog_task(void *pvParameters) {
	// delay to offset task relative to others, then start
	vTaskDelay(WATCHDOG_TASK_FREQ_OFFSET);
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
	
	bool watch_block = false;
	if (!check_task_state_consistency()) {
		watch_block = true;
	} else {
		uint32_t curr_time = get_current_timestamp_ms();
		for (int i = 0; i < NUM_TASKS; i++) {
			if (!check_ins[i]) {
				if (running_times[i]) {
					watch_block = true;
					break;
				}
			} else {
				if (!running_times[i] || curr_time - running_times[i] > WATCHDOG_ALLOWED_TIMES_MS[i]) {
					watch_block = true;
					break;
				}
			}
		}
	}

	if (watch_block) {
		// "kick" watchdog - RESTART SATELLITE
		trace_print("Watchdog kicked - RESTARTING Satellite");
		xSemaphoreGive(mutex);

		log_error(ELOC_WATCHDOG, ECODE_WACTHDOG_RESET, true);
		write_state_to_storage();
		
		#ifdef WATCHDOG_RESET_ACTIVE
			system_reset();
		#endif
		
		// it doesn't make sense to reset values here because the satellite will reboot
		return false;
		
	} else {
		// pet watchdog - pass this watchdog test, move onto next
		pet_watchdog();
		memset(&running_times, 0, sizeof(uint32_t) * NUM_TASKS);
		trace_print("Pet watchdog");
		xSemaphoreGive(mutex);
		return true;
	}
}

/************************************************************************/
// NOTE: the watchdog is so tightly coupled with the satellite state that
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
	check_ins[task_ind] = true; // set this task bit to true
}

// tasks must check in while running to avoid the watchdog
// (we'll set 'im loose on that task if it doesn't!)
void report_task_running(task_type_t task_ind) {
	xSemaphoreTake(mutex, WATCHDOG_MUTEX_WAIT_TIME_TICKS);
	running_times[task_ind] = get_current_timestamp_ms();
	xSemaphoreGive(mutex);
}

// tasks must check out when suspending so they don't trip the watchdog
// NOTE: not safe to call without having gotten mutex
void check_out_task_unsafe(task_type_t task_ind) {
	check_ins[task_ind] = false; // set this task bit to false
	// set the running time to 0 so when the check task is run, it makes sure
	// there were no incorrect report_task_running calls
	running_times[task_ind] = 0;
}

// Writes state to storage if the watchdog is about to restart the satellite
void watchdog_early_warning_callback(void) {
	log_error(ELOC_WATCHDOG, ECODE_WATCHDOG_EARLY_WARNING, true);
	write_state_to_storage();
}