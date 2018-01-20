/*
 * init_rtos_tasks.c
 *
 * Created: 10/4/2016 8:50:22 PM
 *  Author: mckenna
 */

#include "satellite_state_control.h"
#include "../testing_functions/os_system_tests.h"

/* Satellite state info - ONLY accessible in this file */
uint8_t current_sat_state;

// states that should be set upon task bootup (only used on boot)
task_states boot_task_states;
// states tasks were at before their last (explicit) state change
// (used only to resume the states of T_STATE_ANY tasks that were 
//  specifically suspended or resumed on a state change and are 
//  entering another T_STATE_ANY state)
task_states prev_task_states; 
// global current states
task_states current_task_states;

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

	// Initialize mutex for battery charging
	_battery_charging_mutex = xSemaphoreCreateMutexStatic(&_battery_charging_mutex_d);

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
	
	
	// FOR TESTING TASKS
	create_testing_tasks();
	

	// TODO: Should we even store this handle?
	battery_charging_task_handle = xTaskCreateStatic(battery_charging_task,
		"battery charging action task",
		TASK_BATTERY_CHARGING_STACK_SIZE,
		NULL,
		TASK_BATTERY_CHARGING_PRIORITY,
		battery_charging_task_stack,
		&battery_charging_task_buffer);

	// TODO: Should we even store this handle?
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

	// TODO: Should we even store this handle?
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

	// get state of antenna deploy task and apply
	if (cache_get_sat_event_history(false)->antenna_deployed) { // no one writing so don't wait
		boot_task_states.states[ANTENNA_DEPLOY_TASK] = T_STATE_SUSPENDED;
	} else {
		boot_task_states.states[ANTENNA_DEPLOY_TASK] = T_STATE_RUNNING;
	}

	// add any errors we can from MRAM cache
	// (NOTE; no one should've logged any yet, or else they may be overwritten!)
	populate_error_stacks(&priority_error_equistack, &normal_error_equistack);

	// note we've rebooted
	increment_reboot_count();
}

/* Given a task handle, initializes the task to the correct startup state - called be each task when it starts
	This function will essentially be a (concurrency-related) barrier for tasks (at least those being suspended)
	to run up against; when they call this function they are ready to be suspended if necessary.
 */
void init_task_state(task_type_t task_id) {
	// one of two coming out of boot
	configASSERT (current_sat_state == INITIAL || current_sat_state == IDLE_NO_FLASH);
	set_single_task_state(boot_task_states.states[task_id], task_id);
}

/************************************************************************/
/* FOR DEBUGGING - DON'T SET STATES THIS WAY                            */
/************************************************************************/
/**
 * The "idle" hook for FreeRTOS - this is is code run in the idle task of RTOS, which
 * runs whenever something else is NOT. It should NOT call hanging RTOS functions
 * or take up much computational power in general.
 * See http://www.freertos.org/RTOS-idle-task.html for more details.
 */
void vApplicationIdleHook(void) {
	// FOR TESTING

// 	static int oldTickCount = 0;
// 	int tickCount = xTaskGetTickCount();
// 	int div_tickCount = tickCount / 1000;
// 	if (div_tickCount != oldTickCount) {
// 		oldTickCount = div_tickCount;
// 		print("%d\n\r", tickCount);
// 		//print("test\n\r");
// 	}
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

// // "assigns" the states_setting to states_to_set by array copying
// void assign_task_states(task_states states_to_set, const task_states states_setting) {
// 	memcpy(states_to_set, states_setting, sizeof(enum task_state) * NUM_TASKS);
// }

// returns whether the given task state is consistent with its current RTOS state (given by its task handle state)
bool task_state_consistent(uint8_t task_state, task_type_t task_id) {
	configASSERT(task_state <= T_STATE_ANY);
	switch (task_state) {
		case T_STATE_RUNNING:
			return eTaskGetState(task_handles[task_id]) == eRunning;
		case T_STATE_SUSPENDED:
			return eTaskGetState(task_handles[task_id]) == eSuspended;
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
	for (int task_id = 0; task_id < NUM_TASKS; task_id++) {
		result = result && task_state_consistent(current_task_states.states[task_id], task_id);
	}
	return result;
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
				trace_print("CHANGED STATE to ANTENNA_DEPLOY");
				set_all_task_states(ANTENNA_DEPLOY_TASK_STATES, ANTENNA_DEPLOY);
				return true;
			}
			return false;

		case HELLO_WORLD: ;
			if (current_sat_state == ANTENNA_DEPLOY) {
				trace_print("CHANGED STATE to HELLO_WORLD");
				set_all_task_states(HELLO_WORLD_TASK_STATES, HELLO_WORLD);
				return true;
			}
			return false;

		case HELLO_WORLD_LOW_POWER: ;
			if (current_sat_state == HELLO_WORLD) {
				trace_print("CHANGED STATE to HELLO_WORLD_LOW_POWER");
				set_all_task_states(HELLO_WORLD_LOW_POWER_TASK_STATES, HELLO_WORLD_LOW_POWER);
				return true;
			}

		case IDLE_NO_FLASH: ;
			if (current_sat_state == IDLE_FLASH || current_sat_state == HELLO_WORLD || current_sat_state == LOW_POWER) {
				trace_print("CHANGED STATE to IDLE_NO_FLASH");
				set_all_task_states(IDLE_NO_FLASH_TASK_STATES, IDLE_NO_FLASH);
				return true;
			}
			return false;

		case IDLE_FLASH: ;
			if (current_sat_state == IDLE_NO_FLASH) {
				trace_print("CHANGED STATE to IDLE_FLASH");
				set_all_task_states(IDLE_FLASH_TASK_STATES, IDLE_FLASH);
				return true;
			}
			return false;

		case LOW_POWER: ;
			if (current_sat_state == IDLE_NO_FLASH || current_sat_state == IDLE_FLASH) {
				trace_print("CHANGED STATE to LOW_POWER");
				set_all_task_states(LOW_POWER_TASK_STATES, LOW_POWER);
				return true;
			}
			return false;

		case RIP: ;
			// we can always go to RIP
			trace_print("CHANGED STATE to RIP");
			set_all_task_states(RIP_TASK_STATES, RIP);
			return true;

		default:
			log_error(ELOC_STATE_HANDLING, ECODE_UNEXPECTED_CASE, true);
			configASSERT(false); // bad state ID
	}
}

bool set_sat_state(sat_state_t state) {
	bool valid = set_sat_state_helper(state);
	if (!valid) {
		configASSERT(false); // busy loop because this is bad
	}
	return valid;
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
	watchdog_mutex_take();
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

void suspend_antenna_deploy(void) {
	watchdog_mutex_take();
	set_single_task_state(T_STATE_SUSPENDED, ANTENNA_DEPLOY_TASK);
	watchdog_mutex_give();
}
