/*
 * Watchdog_Task.c
 *
 * Created: 3/1/2017 9:26:57 PM
 *  Author: jleiken
 *
 */ 

#include "Watchdog_Task.h"

uint32_t WATCHDOG_ALLOWED_TIMES_MS[NUM_TASKS] = {
	WATCHDOG_TASK_FREQ + WATCHDOG_BUFFER,
	STATE_HANDLING_TASK_FREQ + WATCHDOG_BUFFER,
	ANTENNA_DEPLOY_TASK_LESS_FREQ + WATCHDOG_BUFFER, // TODO: this won't monitor very tight during ANTENNA_DEPLOY
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
	configure_watchdog();
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
	bool got_mutex = true;
	if (!xSemaphoreTake(mutex, WATCHDOG_MUTEX_WAIT_TIME_TICKS)) {
		// this is an error but the watchdog is so critical we'll keep running
		log_error(ELOC_WATCHDOG, ECODE_WATCHDOG_MUTEX_TIMEOUT, true);
		got_mutex = false;
	}
	
	bool watch_block = false;
	// we only care about ticks since boot, not total timestamp
	uint32_t curr_time = xTaskGetTickCount();
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

	if (watch_block) {
		// "kick" watchdog; RESTART SATELLITE
		print("Watchdog kicked - RESTARTING Satellite");
		if (got_mutex) xSemaphoreGive(mutex);

		log_error(ELOC_WATCHDOG, ECODE_WATCHDOG_RESET, true);
		write_state_to_storage();
		
		#ifdef WATCHDOG_RESET_ACTIVE
			system_reset();
		#endif
		
		// it doesn't make sense to reset values here because the satellite will reboot
		return false;
		
	} else {
		// pet watchdog - pass this watchdog test, move onto next
		pet_watchdog();
		print("Pet watchdog");
		if (got_mutex) xSemaphoreGive(mutex);
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
BaseType_t watchdog_mutex_take(void) {
	return xSemaphoreTake(mutex, WATCHDOG_MUTEX_WAIT_TIME_TICKS);
}

void watchdog_mutex_give(void) {
	xSemaphoreGive(mutex);
}

// tasks must check in when resuming from suspension or launching
// NOTE: not safe to call without having gotten mutex
void check_in_task_unsafe(task_type_t task_ind) {
	check_ins[task_ind] = true; // set this task bit to true
	// set a tasks running time to the current one 
	// (this acts as its first "check in"; it may not run before watchdog does right after changing state)
	running_times[task_ind] = xTaskGetTickCount();
}

// tasks must check in while running to avoid the watchdog
// (we'll set 'im loose on that task if it doesn't!)
void report_task_running(task_type_t task_ind) {
	bool got_mutex = true;
	if (!xSemaphoreTake(mutex, WATCHDOG_MUTEX_WAIT_TIME_TICKS)) {
		// log error, but continue becasue this is crucial 
		log_error(ELOC_WATCHDOG, ECODE_WATCHDOG_MUTEX_TIMEOUT, true);
		got_mutex = false;
	}
	running_times[task_ind] = xTaskGetTickCount();
	if (got_mutex) xSemaphoreGive(mutex);
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
	// TODO: is this all good from an interrupt???
	log_error_from_isr(ELOC_WATCHDOG, ECODE_WATCHDOG_EARLY_WARNING, true);
	write_state_to_storage_emergency(true); // from ISR
}