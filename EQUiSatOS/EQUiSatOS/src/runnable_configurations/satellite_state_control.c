/*
 * init_rtos_tasks.c
 *
 * Created: 10/4/2016 8:50:22 PM
 *  Author: mckenna
 */

#include "satellite_state_control.h"
#include "../testing_functions/os_system_tests.h"

// vector of radio states       RLIIHHAI (must be in REVERSE order of states in rtos_task_config.h - see above)
const uint16_t RADIO_STATES = 0b10110100;

/************************************************************************/
/* Satellite state info - ONLY accessible in this file; ACTUALLY configured on boot */
/************************************************************************/ 
sat_state_t current_sat_state = INITIAL;

// specific state for radio
bool current_radio_state = false;

/************************************************************************/
/* task states                                                          */
/************************************************************************/
// states that should be set upon task bootup (only used on boot)
task_states boot_task_states;
// states tasks were at before their last (explicit) state change
// (used only to resume the states of T_STATE_ANY tasks that were 
//  specifically suspended or resumed on a state change and are 
//  entering another T_STATE_ANY state)
task_states prev_task_states; 
// global current states
task_states current_task_states;

/************************************************************************/
/* global states for hardware + mutex                                   */
/************************************************************************/ 
struct hw_states hardware_states = {false, false, false, false};
StaticSemaphore_t _hardware_state_mutex_d;
SemaphoreHandle_t hardware_state_mutex;

void configure_state_from_reboot(void);
void assign_task_states(task_states states_to_set, const task_states states_setting);
void set_single_task_state(enum task_state state, task_type_t task_id);
void startup_task(void* pvParameters);

// starts RTOS scheduler
void run_rtos()
{
	// create first init task to start RTOS and other tasks
	xTaskCreateStatic(startup_task,
		"initializer task",
		TASK_INIT_STACK_SIZE,
		NULL,
		TASK_INIT_PRIORITY,
		init_task_stack,
		&init_task_buffer);

	/* Start the tasks and timer running. */
	vTaskStartScheduler();
}

/**
 * Only task configured to start at boot; starts the rest of RTOS.
 * We use this task to do most of the startup because it allows
 * use of various RTOS functionality (which can cause issues when used
 * before the scheduler is running).
 * WARNING: The vApplicationDaemonTaskStartupHook did not work for us.
 */
void startup_task(void* pvParameters) {
	#if PRINT_DEBUG != 0
		rtos_started = true;
	#endif
	
	// utility function to write initial state to MRAM (ONCE before launch)
	//write_custom_state();
	
	/************************************************************************/
	/* ESSENTIAL INITIALIZATION                                             */
	/************************************************************************/

	// read state from MRAM for first time
	// this initializes the error equistacks, so make sure no errors
	// are logged before this (they shouldn't be before RTOS is started...)
	configure_state_from_reboot();

	// function in global to init things that use RTOS
	global_init_post_rtos();

	// populate task_handles array and setup constants
	pre_init_rtos_tasks();

	// Initialize misc. state mutexes
	battery_charging_mutex = xSemaphoreCreateMutexStatic(&_battery_charging_mutex_d);
	hardware_state_mutex = xSemaphoreCreateMutexStatic(&_hardware_state_mutex_d);
	critical_action_mutex = xSemaphoreCreateMutexStatic(&_critical_action_mutex_d);

	// Initialize EQUiStack mutexes
	_idle_equistack_mutex = xSemaphoreCreateMutexStatic(&_idle_equistack_mutex_d);
	_attitude_equistack_mutex = xSemaphoreCreateMutexStatic(&_attitude_equistack_mutex_d);
	_flash_equistack_mutex = xSemaphoreCreateMutexStatic(&_flash_equistack_mutex_d);
	_flash_cmp_equistack_mutex = xSemaphoreCreateMutexStatic(&_flash_cmp_equistack_mutex_d);
	_low_power_equistack_mutex = xSemaphoreCreateMutexStatic(&_low_power_equistack_mutex_d);

	// Initialize EQUiStacks
	equistack_Init(&idle_readings_equistack, &_idle_equistack_arr,
		sizeof(idle_data_t), IDLE_STACK_MAX, _idle_equistack_mutex);
	equistack_Init(&attitude_readings_equistack, &_attitude_equistack_arr,
		sizeof(attitude_data_t), ATTITUDE_STACK_MAX, _attitude_equistack_mutex);
	equistack_Init(&flash_readings_equistack, &_flash_equistack_arr,
		sizeof(flash_data_t), FLASH_STACK_MAX, _flash_equistack_mutex);
 	equistack_Init(&flash_cmp_readings_equistack, &_flash_cmp_equistack_arr,
		sizeof(flash_cmp_data_t), FLASH_CMP_STACK_MAX, _flash_cmp_equistack_mutex);
 	equistack_Init(&low_power_readings_equistack, &_low_power_equistack_arr,
		sizeof(low_power_data_t), LOW_POWER_STACK_MAX, _low_power_equistack_mutex);

	/************************************************************************/
	/* TASK CREATION                                                        */
	/************************************************************************/

	// NOTE: We can't actually set task state before they are started below,
	// and tasks start as active (resumed), so we have the tasks themselves
	// call the function below which will either suspend them or leave them
	// running as defined by the configuration of initial state.

	// suspend the scheduler while adding tasks so their task handles
	// are non-null when they start executing (i.e. they can be controlled)
	vTaskSuspendAll();
	
	/************************************************************************/
	/* TESTING/MISC                                                         */
	/************************************************************************/
	create_testing_tasks();
	/************************************************************************/
	/* END TESTING                                                          */
	/************************************************************************/

	battery_charging_task_handle = xTaskCreateStatic(battery_charging_task,
		"battery charging action task",
		TASK_BATTERY_CHARGING_STACK_SIZE,
		NULL,
		TASK_BATTERY_CHARGING_PRIORITY,
		battery_charging_task_stack,
		&battery_charging_task_buffer);

	antenna_deploy_task_handle = xTaskCreateStatic(antenna_deploy_task,
		"antenna deploy action task",
		TASK_ANTENNA_DEPLOY_STACK_SIZE,
		NULL,
		TASK_ANTENNA_DEPLOY_PRIORITY,
		antenna_deploy_task_stack,
		&antenna_deploy_task_buffer);

	state_handling_task_handle = xTaskCreateStatic(state_handling_task,
		"state handling action task",
		TASK_STATE_HANDLING_STACK_SIZE,
		NULL,
		TASK_STATE_HANDLING_PRIORITY,
		state_handling_task_stack,
		&state_handling_task_buffer);

	watchdog_task_handle = xTaskCreateStatic(watchdog_task,
		"watchdog task",
		TASK_WATCHDOG_STACK_SIZE,
		NULL,
		TASK_WATCHDOG_STACK_PRIORITY,
		watchdog_task_stack,
		&watchdog_task_buffer);

	persistent_data_backup_task_handle = xTaskCreateStatic(persistent_data_backup_task,
		"persistent data backup task",
		TASK_PERSISTENT_DATA_BACKUP_STACK_SIZE,
		NULL,
		TASK_PERSISTENT_DATA_BACKUP_PRIORITY,
		persistent_data_backup_task_stack,
		&persistent_data_backup_task_buffer);

	flash_activate_task_handle = xTaskCreateStatic(flash_activate_task,
		"flash action task",
		TASK_FLASH_ACTIVATE_STACK_SIZE,
		NULL,
		TASK_FLASH_ACTIVATE_PRIORITY,
		flash_activate_task_stack,
		&flash_activate_task_buffer);

	transmit_task_handle = xTaskCreateStatic(transmit_task,
		"transmit action task",
		TASK_TRANSMIT_STACK_SIZE,
		NULL,
		TASK_TRANSMIT_PRIORITY,
		transmit_task_stack,
		&transmit_task_buffer);

	/* Data tasks */

	idle_data_task_handle = xTaskCreateStatic(idle_data_task,
		"idle data reader task",
		TASK_IDLE_DATA_RD_STACK_SIZE,
		NULL,
		TASK_IDLE_DATA_RD_PRIORITY,
		idle_data_task_stack,
		&idle_data_task_buffer);

	attitude_data_task_handle = xTaskCreateStatic(attitude_data_task,
		"attitude data reader task",
		TASK_ATTITUDE_DATA_RD_STACK_SIZE,
		NULL,
		TASK_ATTITUDE_DATA_DATA_RD_PRIORITY,
		attitude_data_task_stack,
		&attitude_data_task_buffer);

	low_power_data_task_handle = xTaskCreateStatic(low_power_data_task,
		"low power data reader task",
		TASK_LOW_POWER_DATA_RD_STACK_SIZE,
		NULL,
		TASK_LOW_POWER_DATA_RD_PRIORITY,
		low_power_data_task_stack,
		&low_power_data_task_buffer);

	xTaskResumeAll();

	// delete this task after it's done booting
	vTaskDelete(NULL);
}

/* loads stored state from persistent storage and sets up correct boot parameters */
void configure_state_from_reboot(void) {
	// send first read command
	read_state_from_storage();

	#ifdef OVERRIDE_INIT_SAT_STATE
		current_sat_state =		OVERRIDE_INIT_SAT_STATE;
		boot_task_states =		OVERRIDE_INIT_TASK_STATES; // may be modified
		prev_task_states =		OVERRIDE_INIT_TASK_STATES;
		current_task_states =	OVERRIDE_INIT_TASK_STATES;
	#else

	// based on satellite state, set initial state
	// (it will actually be set as tasks come online)
	// (this is done differently than during normal satellite operation,
	// because we're still initializing and tasks can't be simply suspended
	// right now (RTOS isn't even running yet). So, we configure what
	// needs to be done and configure tasks as they boot up and report
	// to the function below that they're ready)
	sat_state_t state_at_reboot = cache_get_sat_state(false); // no one will be writing so don't wait
	if (state_at_reboot == INITIAL
		|| state_at_reboot == ANTENNA_DEPLOY
		|| state_at_reboot == HELLO_WORLD) {

		current_sat_state =		INITIAL;
		// the distinction between these two is related to T_STATE_ANY;
		// we use boot_task_states to explicitly set state for all tasks on boot
		// (so at the end of this it won't contain any T_STATE_ANYs),
		// while current_task_states signifies the current abstract state
		boot_task_states =		INITIAL_TASK_STATES; // may be modified
		prev_task_states =		INITIAL_TASK_STATES;
		current_task_states =	INITIAL_TASK_STATES;

	} else {
		current_sat_state =		IDLE_NO_FLASH;
		boot_task_states =		IDLE_NO_FLASH_TASK_STATES; // may be modified
		prev_task_states =		IDLE_NO_FLASH_TASK_STATES;
		current_task_states =	IDLE_NO_FLASH_TASK_STATES;
	}
	#endif

	// get state of antenna deploy task and apply
	if (cache_get_sat_event_history().antenna_deployed) { 
		boot_task_states.states[ANTENNA_DEPLOY_TASK] = T_STATE_SUSPENDED;
	} else {
		boot_task_states.states[ANTENNA_DEPLOY_TASK] = T_STATE_RUNNING;
	}
	
	// if the satellite restarted because of the watchdog, log that as an error so we know
	if (did_watchdog_kick()) {
		log_error(ELOC_WATCHDOG, ECODE_WATCHDOG_DID_KICK, true);
	}

	// if we had to rewrite program memory due to corruption, log a low-pri error
	if (cache_get_prog_mem_rewritten()) {
		log_error(ELOC_BOOTLOADER, ECODE_REWROTE_PROG_MEM, false);
		update_sat_event_history(0, 0, 0, 0, 0, 0, 1);
	}

	// add any errors we can from MRAM cache
	// (NOTE; no one should've logged any yet, or else they may be overwritten!)
	populate_error_stacks(&error_equistack);

	// note we've rebooted
	increment_reboot_count();
}

/* Given a task handle, initializes the task to the correct startup state - called be each task when it starts
	This function will essentially be a (concurrency-related) barrier for tasks (at least those being suspended)
	to run up against; when they call this function they are ready to be suspended if necessary.
 */
void init_task_state(task_type_t task_id) {
	// one of two coming out of boot
	#ifndef OVERRIDE_INIT_SAT_STATE
		configASSERT (current_sat_state == INITIAL || current_sat_state == IDLE_NO_FLASH);
	#endif
	set_single_task_state(boot_task_states.states[task_id], task_id);
}

/************************************************************************/
/* STATE SETTING METHODS                                                */
/************************************************************************/
void set_all_task_states(task_states states, sat_state_t state);

/* Getter for current global satellite state */
sat_state_t get_sat_state(void) {
	return current_sat_state;
}

/* Getter for current global task states (what's supposed to be running, etc.) */
task_states get_sat_task_states(void) {
	return current_task_states;
}

// returns whether the given task state is consistent with its current RTOS state (given by its task handle state)
bool task_state_consistent(uint8_t expected_state, task_type_t task_id) {
	configASSERT(expected_state <= T_STATE_ANY);
	eTaskState task_state_rtos = eTaskGetState(*task_handles[task_id]);
	switch (expected_state) {
		case T_STATE_RUNNING:
			// note: a task will usually be blocked and possibly be ready; 
			// it will only be running if it has called this function itself
			return task_state_rtos == eBlocked || task_state_rtos == eReady || task_state_rtos == eRunning;
		case T_STATE_SUSPENDED:
			return task_state_rtos == eSuspended;
		case T_STATE_ANY:
			// if a tasks state is not set in this state, make sure it matches its 
			// previous state
			return task_state_consistent(prev_task_states.states[task_id], task_id);
		default:
			configASSERT(false);
			return true;
	}
}

/* Returns whether the tasks that should be running/stopped actually are  (according to current RTOS internal task state) */
bool check_task_state_consistency(void) {
	bool result = true;
	for (task_type_t task_id = 0; task_id < NUM_TASKS; task_id++) {
		result = task_state_consistent(current_task_states.states[task_id], task_id) && result;
	}
	return result;
}

/* wrapper for setting radio power state */
void set_radio_by_sat_state(sat_state_t state) {
	bool new_state = RADIO_STATES & 1 << state;
	
	if (new_state) {
		// don't re-enable, because this entails waiting to confirm power on
		if (current_radio_state != new_state) 
			setRadioState(true, true);
			//TODO: //submit_radio_command(POWER_ON, true /* (confirm) */, NULL, NULL, 4000);
	} else {
		setRadioState(false, false);
		//TODO: //submit_radio_command(POWER_OFF, true /* (confirm) */, NULL, NULL, 2000);
	}
	current_radio_state = new_state;
}

/* Sets the current satellite state to the given state, if a transition from
   the current state is valid; returns whether the state change was made (i.e. was valid)
   CAN be called consistenly (i.e. if state == CurrentState), which will ensure all correct tasks
   for a state are running, etc. */
bool set_sat_state_helper(sat_state_t state)
{
	// setting the state to the current state is not wrong,
	// but we don't want to actually change task states every time this is called
	// when the state is the current state
	if (state == current_sat_state) {
		return true;
	}

	switch(state)
	{
		case INITIAL:
			return false; // only done initially (via direct set)

		case ANTENNA_DEPLOY: ;
			if (current_sat_state == INITIAL) {
				print("CHANGED STATE to ANTENNA_DEPLOY");
				set_all_task_states(ANTENNA_DEPLOY_TASK_STATES, ANTENNA_DEPLOY);
				set_radio_by_sat_state(ANTENNA_DEPLOY);
				return true;
			}
			return false;

		case HELLO_WORLD: ;
			if (current_sat_state == ANTENNA_DEPLOY) {
				print("CHANGED STATE to HELLO_WORLD");
				set_all_task_states(HELLO_WORLD_TASK_STATES, HELLO_WORLD);
				set_radio_by_sat_state(HELLO_WORLD);
				return true;
			}
			return false;

		case HELLO_WORLD_LOW_POWER: ;
			if (current_sat_state == HELLO_WORLD) {
				print("CHANGED STATE to HELLO_WORLD_LOW_POWER");
				set_all_task_states(HELLO_WORLD_LOW_POWER_TASK_STATES, HELLO_WORLD_LOW_POWER);
				set_radio_by_sat_state(HELLO_WORLD_LOW_POWER);
				return true;
			}

		case IDLE_NO_FLASH: ;
			if (current_sat_state == IDLE_FLASH || current_sat_state == HELLO_WORLD || current_sat_state == LOW_POWER) {
				print("CHANGED STATE to IDLE_NO_FLASH");
				set_all_task_states(IDLE_NO_FLASH_TASK_STATES, IDLE_NO_FLASH);
				set_radio_by_sat_state(IDLE_NO_FLASH);
				return true;
			}
			return false;

		case IDLE_FLASH: ;
			if (current_sat_state == IDLE_NO_FLASH) {
				print("CHANGED STATE to IDLE_FLASH");
				set_all_task_states(IDLE_FLASH_TASK_STATES, IDLE_FLASH);
				set_radio_by_sat_state(IDLE_FLASH);
				return true;
			}
			return false;

		case LOW_POWER: ;
			if (current_sat_state == IDLE_NO_FLASH || current_sat_state == IDLE_FLASH) {
				print("CHANGED STATE to LOW_POWER");
				set_all_task_states(LOW_POWER_TASK_STATES, LOW_POWER);
				set_radio_by_sat_state(LOW_POWER);
				return true;
			}
			return false;

		case RIP: ;
			// we can always go to RIP
			print("CHANGED STATE to RIP");
			set_all_task_states(RIP_TASK_STATES, RIP);
			set_radio_by_sat_state(RIP);
			return true;

		default:
			log_error(ELOC_STATE_HANDLING, ECODE_UNEXPECTED_CASE, true);
			configASSERT(false); // bad state ID
	}
}

bool set_sat_state(sat_state_t state) {
	#if OVERRIDE_STATE_HOLD != 1
		bool valid = set_sat_state_helper(state);
		if (!valid) {
			configASSERT(false); // busy loop because this is bad
		}
		return valid;
	#endif
}

/************************************************************************/
// TASK STATE CONTROL WRAPPERS
// - task_suspend and task_resume are called while the scheduler
//   is suspended and the watchdog mutex is locked so don't need to be safe
/************************************************************************/
void task_suspend(task_type_t task_id);
void task_resume(task_type_t task_id);

// sets all task states atomically by suspending the RTOS scheduler and watchdog,
// resuming/suspending/ignoring task states, and setting the current
// state variable (atomically in here)
void set_all_task_states(const task_states states, sat_state_t state)
{
	// Don't allow other tasks to run while we're changing state,
	// and make sure to get the watchdog mutex so its state is stable
	if (!watchdog_mutex_take()) {
		// if the watchdog manages to time out, 
		log_error(ELOC_STATE_HANDLING, ECODE_WATCHDOG_MUTEX_TIMEOUT, true);
	}
	vTaskSuspendAll();

	// values given by external-facing functions
	current_sat_state = state;
	current_task_states = states;

	for (int task_id = 0; task_id < NUM_TASKS; task_id++) {
		set_single_task_state(states.states[task_id], task_id);
	}

	xTaskResumeAll();
	watchdog_mutex_give();
}

// sets whether this task is running, suspended, or resumes a previous
// state if set to don't care.
// if set to run or suspend, effective immediately
void set_single_task_state(enum task_state state, task_type_t task_id) {
	configASSERT(task_id < NUM_TASKS);
	switch (state) {
		case T_STATE_RUNNING:
			prev_task_states.states[task_id] = T_STATE_RUNNING;
			task_resume(task_id);
			return;
		case T_STATE_SUSPENDED:
			prev_task_states.states[task_id] = T_STATE_SUSPENDED;
			task_suspend(task_id);
			return;
		case T_STATE_ANY:
			// note we don't set prev_task_states because it will 
			// continue the (current) previous
			configASSERT(prev_task_states.states[task_id] != T_STATE_ANY);
			set_single_task_state(prev_task_states.states[task_id], task_id);
			return;
		default:
			configASSERT(false);
	};
}

// suspends the given task if it was not already suspended, and (always) checks it out of the watchdog
void task_suspend(task_type_t task_id) {
	TaskHandle_t* task_handle = task_handles[task_id];

	configASSERT(task_handle != NULL && *task_handle != NULL); // the latter would suspend THIS task

	// always check out of watchdog when called (to be double-sure)
	// this is only called here so doesn't need to be safe
	check_out_task_unsafe(task_id);
	if (eTaskGetState(*task_handle) != eSuspended) {
		vTaskSuspend(*task_handle); // actually suspend using handle
	}
}

// resumes the given task if it was suspended, and (always) checks it in to the watchdog
void task_resume(task_type_t task_id)
{
	TaskHandle_t* task_handle = task_handles[task_id];
	configASSERT(task_handle != NULL);

	// always check in for watchdog when called
	// (in case it wasn't, for example on BOOT)
	// this is only called here so doesn't need to be safe
	check_in_task_unsafe(task_id);

	if (eTaskGetState(*task_handle) == eSuspended)
	{
		// actually resume task (will be graceful if task_handle task is not actually suspended)

		configASSERT(*task_handle != NULL);

		vTaskResume(*task_handle);
	}
}

/* suspends or resumes the given task, safely pausing the watchdog 
   NOTE: CURRENTLY SHOULD ONLY BE CALLED ON THE ANTENNA_DEPLOY_TASK */
void set_task_state_safe(task_type_t task_id, bool run) {
	watchdog_mutex_take();
	if (run) {
		task_resume(task_id);
	} else {
		task_suspend(task_id);
	}
	watchdog_mutex_give();
}


/************************************************************************/
/* HARDWARE STATE HANDLING                                              */
/************************************************************************/

// allows access to hardware states - make sure you have the mutex if changing
// it or having the expectation that its true for any period of time
struct hw_states* get_hw_states(void) {
	return &hardware_states;
}

BaseType_t hardware_state_mutex_take(void) {
	return xSemaphoreTake(hardware_state_mutex, HARDWARE_STATE_MUTEX_WAIT_TIME_TICKS);
}

void hardware_state_mutex_give(void) {
	xSemaphoreGive(hardware_state_mutex);
}