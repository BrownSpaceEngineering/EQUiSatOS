/*
 * Watchdog_Task.c
 *
 * Created: 3/1/2017 9:26:57 PM
 *  Author: jleiken
 *
 */ 

#include "Watchdog_Task.h"

uint32_t WATCHDOG_ALLOWED_TIMES_MS[NUM_TASKS] = {
	WATCHDOG_TASK_FREQ + WATCHDOG_TASK_TIMEOUT_BUFFER, // doesn't really matter, not used
	STATE_HANDLING_TASK_FREQ + WATCHDOG_TASK_TIMEOUT_BUFFER,
	ANTENNA_DEPLOY_TASK_WATCHDOG_TIMEOUT + WATCHDOG_TASK_TIMEOUT_BUFFER,
	BATTERY_CHARGING_TASK_FREQ + WATCHDOG_TASK_TIMEOUT_BUFFER,
	TRANSMIT_TASK_LESS_FREQ + WATCHDOG_TASK_TIMEOUT_BUFFER,
	FLASH_ACTIVATE_TASK_FREQ + WATCHDOG_TASK_TIMEOUT_BUFFER,
	IDLE_DATA_TASK_FREQ + WATCHDOG_TASK_TIMEOUT_BUFFER,
	LOW_POWER_DATA_TASK_FREQ + WATCHDOG_TASK_TIMEOUT_BUFFER,
	ATTITUDE_DATA_TASK_FREQ + WATCHDOG_TASK_TIMEOUT_BUFFER,
	PERSISTENT_DATA_BACKUP_TASK_FREQ + WATCHDOG_TASK_TIMEOUT_BUFFER
};

static bool check_ins[NUM_TASKS];
static uint32_t running_times[NUM_TASKS];
static TickType_t prev_time;

void init_watchdog_clock(void) {
	rtos_ready = false;
	got_early_warning_callback_in_boot = false;
	configure_watchdog(watchdog_early_warning_callback);
}

// initializes our watchdog-monitoring task
void init_watchdog_task(void) {
	configASSERT(WATCHDOG_TASK == 0);
	memset(&check_ins, 0, sizeof(bool) * NUM_TASKS);
	memset(&running_times, 0, sizeof(uint32_t) * NUM_TASKS);
	watchdog_mutex = xSemaphoreCreateMutexStatic(&_watchdog_task_mutex_d);
	prev_time = xTaskGetTickCount();
}

static void task_pet_watchdog(bool got_mutex) {
	pet_watchdog();
	print("Pet watchdog");
	if (got_mutex) xSemaphoreGive(watchdog_mutex);
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
	if (!xSemaphoreTake(watchdog_mutex, WATCHDOG_MUTEX_WAIT_TIME_TICKS)) {
		// this is an error but the watchdog is so critical we'll keep running
		log_error(ELOC_WATCHDOG, ECODE_WATCHDOG_MUTEX_TIMEOUT, true);
		got_mutex = false;
	}
	
	bool watch_block = false;
	// we only care about ticks since boot, not total timestamp
	TickType_t curr_time = xTaskGetTickCount();
	// if the time has wrapped around it will be less than the previous, so just say it's fine
	// and wait for the next call of the task
	if (curr_time < prev_time) {
		task_pet_watchdog(got_mutex);
		if (got_mutex) xSemaphoreGive(watchdog_mutex);
		return true;
	}
	prev_time = curr_time;
	
	// NOTE: WATCHDOG_TASK == 0
	for (task_type_t i = WATCHDOG_TASK + 1; i < NUM_TASKS; i++) { 
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
		if (got_mutex) xSemaphoreGive(watchdog_mutex);

		log_error(ELOC_WATCHDOG, ECODE_WATCHDOG_RESET, true);
		write_state_to_storage();
		
		#ifdef WATCHDOG_RESET_ACTIVE
			system_reset();
		#endif
		
		// it doesn't make sense to reset values here because the satellite will reboot
		return false;
		
	} else {
		// pet watchdog - pass this watchdog test, move onto next
		task_pet_watchdog(got_mutex);
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
	if (!xSemaphoreTake(watchdog_mutex, WATCHDOG_MUTEX_WAIT_TIME_TICKS)) {
		// log error, but continue becasue this is crucial
		log_error(ELOC_WATCHDOG, ECODE_WATCHDOG_MUTEX_TIMEOUT, true);
		got_mutex = false;
	}
	running_times[task_ind] = xTaskGetTickCount();
	if (got_mutex) xSemaphoreGive(watchdog_mutex);
}

// tasks must check out when suspending so they don't trip the watchdog
// NOTE: not safe to call without having gotten mutex
void check_out_task_unsafe(task_type_t task_ind) {
	check_ins[task_ind] = false; // set this task bit to false
	// set the running time to 0 so when the check task is run, it makes sure
	// there were no incorrect report_task_running calls
	running_times[task_ind] = 0;
}

// INTERRUPT; Writes state to storage if the watchdog is about to restart the satellite
void watchdog_early_warning_callback(void) {
	// if RTOS is ready we can manually log errors/write to storage
	if (rtos_ready) {
		log_error_from_isr(ELOC_WATCHDOG, ECODE_WATCHDOG_EARLY_WARNING, true);
		write_state_to_storage_emergency(true); // from ISR
	}
	// otherwise; defer this operation by telling RTOS to do it once started
	got_early_warning_callback_in_boot = true;
}

/* testing functions */
// testing function to get whether a task is checked in
bool _get_task_checked_in(task_type_t task) {
	return check_ins[task];
}
// testing function to get time a task was checked in
uint32_t _get_task_checked_in_time(task_type_t task) {
	return running_times[task];
}