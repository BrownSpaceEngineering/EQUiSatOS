/*
 * init_rtos_tasks.c
 *
 * Created: 10/4/2016 8:50:22 PM
 *  Author: mckenna
 */

#include "satellite_state_control.h"

/************************************************************************/
/* Satellite state info - ONLY accessible in this file; ACTUALLY configured on boot */
/************************************************************************/ 
sat_state_t current_sat_state = INITIAL;
task_states current_task_states;

/************************************************************************/
/* global states for hardware		                                    */
/************************************************************************/ 
struct hw_states hardware_states = {false, RADIO_OFF, false, 0, 0, 0};

/************************************************************************/
/* List of mutexes for massive take of all of them during a task state change */
/************************************************************************/
// note; all are statically defined so have predetermined addresses
SemaphoreHandle_t* all_mutexes_ordered[NUM_MUTEXES] = {
	// ORDER IS CRUCIAL: see https://docs.google.com/document/d/1F6cmlkyZeJqcSpiwlJpkhQtoeE0EPKovmBSyXnr2SoY/edit#heading=h.12glh8n1durz
	&critical_action_mutex,
	// NOTE: is handled differently; taken before i2c_irpow mutex (because most users of i2c do), and waited on until it == IR_POW_SEMAPHORE_MAX_COUNT
	&irpow_semaphore, 
	&i2c_irpow_mutex,
	&processor_adc_mutex,
	&hardware_state_mutex,
	&watchdog_mutex,
	&mram_spi_cache_mutex,
	#if (PRINT_DEBUG == 1 || PRINT_DEBUG == 3) && defined(SAFE_PRINT)
		// to be technically correct with prints
		&print_mutex,
	#endif
	// equistack mutexes
	&_idle_equistack_mutex,
	&_attitude_equistack_mutex,
	&_flash_equistack_mutex,
	&_flash_cmp_equistack_mutex,
	&_low_power_equistack_mutex,
	// error equistack mutex last just because it follows the calls structure
	&_error_equistack_mutex
};

void configure_state_from_reboot(void);
void set_single_task_state(bool running, task_type_t task_id);
void startup_task(void* pvParameters);

// starts RTOS scheduler
void run_rtos()
{
	configASSERT(NUM_MUTEXES == sizeof(all_mutexes_ordered) / sizeof(SemaphoreHandle_t*));
	
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
	print("EQUiSatOS starting... ");
	
	#ifdef WRITE_DEFAULT_MRAM_VALS
		// utility function to write initial state to MRAM (ONCE before launch)
		write_custom_state();
	#endif
	
	/************************************************************************/
	/* DATA INITIALIZATION                                                  */
	/************************************************************************/	

	// Initialize misc. state mutexes
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
	/* ESSENTIAL INITIALIZATION                                             */
	/************************************************************************/
	// read state from MRAM for first time
	// this initializes the error equistacks, so make sure no errors
	// are logged before this (they shouldn't be before RTOS is started...)
	configure_state_from_reboot();

	// function in global to init things that use RTOS 
	// (MUST be done after config state from reboot)
	global_init_post_rtos();

	// populate task_handles array and setup constants
	pre_init_rtos_tasks();
	
	#if configUSE_TRACE_FACILITY == 1
		// name mutexes
		vTraceSetMutexName(critical_action_mutex, "CA");
		vTraceSetMutexName(i2c_irpow_mutex, "I2C");
		vTraceSetMutexName(irpow_semaphore, "IRPOW");
		vTraceSetMutexName(processor_adc_mutex, "ADC");
		vTraceSetMutexName(hardware_state_mutex, "HWST");
		vTraceSetMutexName(watchdog_mutex, "WD");
		vTraceSetMutexName(mram_spi_cache_mutex, "SPI");
		
		vTraceSetMutexName(_idle_equistack_mutex, "eqIDLE");
		vTraceSetMutexName(_attitude_equistack_mutex, "eqATT");
		vTraceSetMutexName(_flash_equistack_mutex , "eqF_BST");
		vTraceSetMutexName(_flash_cmp_equistack_mutex, "eqF_CMP");
		vTraceSetMutexName(_low_power_equistack_mutex, "eqLP");
		vTraceSetMutexName(_error_equistack_mutex, "eqERR");
	#endif
	
	// note RTOS is ready for watchdog callback, and log error if it already tripped
	rtos_ready = true;
	if (got_early_warning_callback_in_boot) {
		log_error(ELOC_WATCHDOG, ECODE_WATCHDOG_EARLY_WARNING, true);
	}

	/************************************************************************/
	/* TASK CREATION                                                        */
	/************************************************************************/

	// NOTE: We can't actually set task state before they are started below,
	// and tasks start as active (resumed), so we have the tasks themselves
	// call the function below which will either suspend them or leave them
	// running as defined by the configuration of initial state.

	// suspend the scheduler while adding tasks so their task handles
	// are non-null when they start executing (i.e. they can be controlled)
	pet_watchdog(); // in case this takes a bit and we're close
	vTaskSuspendAll();
	
	/************************************************************************/
	/* TESTING/MISC                                                         */
	/************************************************************************/
	create_testing_tasks();
	/************************************************************************/
	/* END TESTING                                                          */
	/************************************************************************/
	
	#ifndef ONLY_RUN_TESTING_TASKS
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
		TASK_WATCHDOG_PRIORITY,
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
	#endif

	xTaskResumeAll();

	setTXEnable(false);
	usart_send_string((uint8_t*) "EQUiSatOS started");
	setTXEnable(true);
	
	print("initialization complete.\n");
	
	// delete this task after it's done booting
	vTaskDelete(NULL);
}

/* loads stored state from persistent storage and sets up correct boot parameters */
void configure_state_from_reboot(void) {
	// send first read command
	read_state_from_storage();

	#ifdef OVERRIDE_INIT_SAT_STATE
		current_sat_state =		OVERRIDE_INIT_SAT_STATE;
		current_task_states =	OVERRIDE_INIT_TASK_STATES;
	#else

	// based on satellite state, set initial state
	// (it will actually be set as tasks come online)
	// (this is done differently than during normal satellite operation,
	// because we're still initializing and tasks can't be simply suspended
	// right now (RTOS isn't even running yet). So, we configure what
	// needs to be done and configure tasks as they boot up and report
	// to the function below that they're ready)
	sat_state_t state_at_reboot = cache_get_sat_state();
	if (state_at_reboot == INITIAL
		|| state_at_reboot == ANTENNA_DEPLOY
		|| state_at_reboot == HELLO_WORLD) {

		current_sat_state =		INITIAL;
		// the distinction between these two is related to T_STATE_ANY;
		// we use boot_task_states to explicitly set state for all tasks on boot
		// (so at the end of this it won't contain any T_STATE_ANYs),
		// while current_task_states signifies the current abstract state
		current_task_states =	INITIAL_TASK_STATES;

	} else {
		current_sat_state =		IDLE_NO_FLASH;
		current_task_states =	IDLE_NO_FLASH_TASK_STATES;
	}
	#endif

	// always start antenna deploy task
	current_task_states.states[ANTENNA_DEPLOY_TASK] = true;
	
	// add any errors we can from MRAM cache
	// (NOTE; no one should've logged any yet, or else they may be overwritten!)
	populate_error_stacks(&error_equistack);
	
	// if the satellite restarted because of the watchdog, log that as an error so we know
	enum system_reset_cause cause = system_get_reset_cause();
	if (cause == SYSTEM_RESET_CAUSE_WDT) {
		log_error(ELOC_WATCHDOG, ECODE_WATCHDOG_DID_KICK, true);
	} else if (cause == SYSTEM_RESET_CAUSE_SOFTWARE) {
		log_error(ELOC_WATCHDOG, ECODE_SOFTWARE_RESET, false); // mostly for debug; hopefully no aliens are hacking us
	} else if (cause == SYSTEM_RESET_CAUSE_EXTERNAL_RESET) {
		log_error(ELOC_WATCHDOG, ECODE_SAT_RESET, true);
	}

	// if we had to rewrite program memory due to corruption, log a low-pri error
	if (cache_get_prog_mem_rewritten()) {
		log_error(ELOC_BOOTLOADER, ECODE_REWROTE_PROG_MEM, false);
		update_sat_event_history(0, 0, 0, 0, 0, 0, 1);  // rare enough that the double-write here is fine
	}

	// note we've rebooted
	increment_reboot_count();
	
	// initial hardware settings (last because they have delays)
	setRadioState(false, false); // turn radio off and don't confirm (won't confirm going off)
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
	set_single_task_state(current_task_states.states[task_id], task_id);
}

/************************************************************************/
/* STATE HELPERS		                                                */
/************************************************************************/
/* Getter for current global satellite state */
sat_state_t get_sat_state(void) {
	return current_sat_state;
}

/* Getter for current global task states (what's supposed to be running, etc.) */
task_states get_sat_task_states(void) {
	return current_task_states;
}

// returns whether the given task state is consistent with its current RTOS state (given by its task handle state)
static bool task_state_consistent(bool expected_state, task_type_t task_id) {
	configASSERT(task_handles[task_id] != NULL && *task_handles[task_id] != NULL);
	eTaskState task_state_rtos = eTaskGetState(*task_handles[task_id]);
	// a task is "running" (expected_state should be true) if it's NOT suspended
	// see https://www.freertos.org/RTOS-task-states.html
	return expected_state == (task_state_rtos != eSuspended);
}

/* Returns whether the tasks that should be running/stopped actually are  (according to current RTOS internal task state) */
bool check_task_state_consistency(void) {
	bool result = true;
	for (task_type_t task_id = 0; task_id < NUM_TASKS; task_id++) {
		result = task_state_consistent(current_task_states.states[task_id], task_id) && result;
	}
	return result;
}

/* Returns whether we're currently in a low power state */
bool low_power_active(void) {
	return current_sat_state == LOW_POWER;
}

/************************************************************************/
/* GLOBAL STATE SETTING                                                 */
/************************************************************************/
/* Sets the current satellite state to the given state, if a transition from
   the current state is valid; returns whether the state change was made (i.e. was valid)
   CAN be called consistently (i.e. if state == CurrentState), which will ensure all correct tasks
   for a state are running, etc. */
bool set_sat_state_helper(sat_state_t state)
{	
	// setting the state to the current state is not wrong,
	// but we don't want to actually change task states every time this is called
	// when the state is the current state
	if (state == current_sat_state) {
		return true;
	}
	
	sat_state_t prev_sat_state = current_sat_state;

	switch(state)
	{
		case INITIAL:
			return false; // only done initially (via direct set)

		case ANTENNA_DEPLOY: ;
			print("\n\nCHANGED STATE to ANTENNA_DEPLOY\n\n");
			set_all_task_states(ANTENNA_DEPLOY_TASK_STATES, ANTENNA_DEPLOY, prev_sat_state);
			// turn radio off and don't confirm (won't confirm going off)
			// (this is the only state that can be re-entered where the transmit task is suspended)
			setRadioState(false, false);
			return prev_sat_state == INITIAL || prev_sat_state == LOW_POWER;

		case HELLO_WORLD: ;
			print("\n\nCHANGED STATE to HELLO_WORLD\n\n");
			set_all_task_states(HELLO_WORLD_TASK_STATES, HELLO_WORLD, prev_sat_state);
			return prev_sat_state == ANTENNA_DEPLOY;

		case IDLE_NO_FLASH: ;
			print("\n\nCHANGED STATE to IDLE_NO_FLASH\n\n");
			set_all_task_states(IDLE_NO_FLASH_TASK_STATES, IDLE_NO_FLASH, prev_sat_state);
			return prev_sat_state == IDLE_FLASH || prev_sat_state == HELLO_WORLD || prev_sat_state == LOW_POWER;

		case IDLE_FLASH: ;
			print("\n\nCHANGED STATE to IDLE_FLASH\n\n");
			set_all_task_states(IDLE_FLASH_TASK_STATES, IDLE_FLASH, prev_sat_state);
			return prev_sat_state == IDLE_NO_FLASH;

		case LOW_POWER: ;
			print("\n\nCHANGED STATE to LOW_POWER\n\n");
			set_all_task_states(LOW_POWER_TASK_STATES, LOW_POWER, prev_sat_state);
			return prev_sat_state == ANTENNA_DEPLOY || prev_sat_state == HELLO_WORLD || 
					prev_sat_state == IDLE_NO_FLASH || prev_sat_state == IDLE_FLASH;

		default:
			log_error(ELOC_STATE_HANDLING, ECODE_UNEXPECTED_CASE, false);
			configASSERT(false); // bad state ID
	}
	return false;
}

bool set_sat_state(sat_state_t state) {
	#if OVERRIDE_STATE_HOLD_INIT != 1
		bool valid = set_sat_state_helper(state);
		if (!valid) {
			log_error(ELOC_STATE_HANDLING, ECODE_INVALID_STATE_CHANGE, false);
			#ifdef USE_STRICT_ASSERTIONS
				configASSERT(false); // busy loop because this should only be radiation issues
			#endif
		} else {
			log_error(ELOC_STATE_HANDLING, ECODE_VALID_STATE_CHANGE, false);
		}
		return valid;
	#endif
}

/* function to handle specific setting of antenna deploy task
   which operates semi-independently of satellite state 
   (it must run in some, must not run in some, and doesn't 
    matter in others) */
static void set_antenna_deploy_by_sat_state(sat_state_t prev_sat_state, sat_state_t next_sat_state, bool antenna_deployed) {
	bool antenna_task_state = false;
	switch (prev_sat_state) {
		// coming from initial, always turn on (we only ever go to antenna deploy)
		case INITIAL:
			antenna_task_state = true;
			break;
			
		// if coming from antenna deploy, suspend
		// if we've actually deployed the antenna, otherwise continue
		case ANTENNA_DEPLOY:
			// "if the antenna DID NOT deploy, remain running"
			antenna_task_state = !antenna_deployed;
			break;
			
		// if coming from one of the normal states,
		// we would only explicitly shutdown the task if we're going into low
		// power, otherwise we'd set it state as appropriate by the antenna
		// deploy state
		case HELLO_WORLD:
		case IDLE_NO_FLASH:
		case IDLE_FLASH:
			if (next_sat_state == LOW_POWER) {
				antenna_task_state = false;
			} else {
				// "if the antenna DID NOT deploy, remain running"
				antenna_task_state = !antenna_deployed;
			}
			break;
			
		// coming from low power, we always resume (it was suspended)
		// if going into ANTENNA_DEPLOY, but if we're entering IDLE_NO_FLASH,
		// we have to set the appropriate task state
		case LOW_POWER:
			if (next_sat_state == ANTENNA_DEPLOY) {
				antenna_task_state = true;
			} else {
				// "if the antenna DID NOT deploy, remain running"
				antenna_task_state = !antenna_deployed;
			}
			break;
		
		default:
			configASSERT(false); // bad state ID or bit flip
			log_error(ELOC_STATE_HANDLING, ECODE_UNEXPECTED_CASE, false);
			break;
	}
	
	// actually set designated state
	set_single_task_state(antenna_task_state, ANTENNA_DEPLOY_TASK);
	// mainly for debugging; ensures check_task_state_consistenty is valid
	current_task_states.states[ANTENNA_DEPLOY_TASK] = antenna_task_state;
}

/************************************************************************/
// TASK STATE CONTROL WRAPPERS
// - task_suspend and task_resume are called while the scheduler
//   is suspended and the watchdog mutex is locked so don't need to be safe
/************************************************************************/

// helper to wait until a semaphore is at zero
static bool wait_on_semaphore(SemaphoreHandle_t sem, TickType_t wait_time) {
	// won't infinite loop if tick count wraps around in here bcs. we're using uints
	TickType_t start_ticks = xTaskGetTickCount();
	while (uxSemaphoreGetCount(sem) < IR_POW_SEMAPHORE_MAX_COUNT) {
		vTaskDelay(SEMAPHORE_EMPTY_POLL_TIME_TICKS);
		if (xTaskGetTickCount() - start_ticks > wait_time) {
			return false;
		}
	}
	return true;
}

// Takes all mutexes in RTOS, in particular those that a task could hold.
// This is done because these mutexes would be inaccessible on that task's suspension.
// Returns whether all were obtained. If they weren't, NO mutexes will be held.
static bool take_all_mutexes(void) {
	// starting from the top (first-taken mutex), try to take ALL the mutexes
	int8_t i = 0;
	while (i < NUM_MUTEXES) {
		configASSERT(all_mutexes_ordered[i] != NULL);
		if (all_mutexes_ordered[i] != NULL) { // avoid null pointer and leave it to watchdog
			// handle special case for irpow counting semaphore (we can't just take it,
			// we want to wait until everyone is done using it, so we wait for it to get to 0)
			// NOTE that there are no guarantees we'll get it, but because all the things that use IR power
			// take the i2c mutex, which we have, we'll be okay. (The auto-IR-power turn off logic handles this)
			if (*(all_mutexes_ordered[i]) == irpow_semaphore) {
				bool semaphore_got_to_zero = wait_on_semaphore(irpow_semaphore, TASK_STATE_CHANGE_MUTEX_WAIT_TIME_TICKS);
				if (!semaphore_got_to_zero) {
					// don't unravel mutexes, but log error (this could lead to timing issues in the future)
					log_error(ELOC_STATE_HANDLING, ECODE_IR_POW_IN_USE_ON_STATE_CHANGE, false);
				}
			} else {
				bool got_mutex = xSemaphoreTake(*(all_mutexes_ordered[i]), TASK_STATE_CHANGE_MUTEX_WAIT_TIME_TICKS);
				if (!got_mutex) {
					break; // start unraveling taken mutexes
				}
			}
		}
		i++;
	}

	// if we got all, we're good
	if (i == NUM_MUTEXES) {
		return true;
	} else {
		// don't try and give the mutex we couldn't get
		i--;
		
		// if any cannot be obtained in their timeout, we have to unroll all our
		// previous takings so we're not holding any mutexes, and THEN try again.
		while (i >= 0) {
			// (assume null check above worked for real satellite)
			if (*(all_mutexes_ordered[i]) != irpow_semaphore) {
				xSemaphoreGive(*(all_mutexes_ordered[i]));
			}
			i--;
		}
		return false;
	}
}

// Gives all mutexes that were taken above
static void give_all_mutexes(void) {
	for (int8_t i = 0; i < NUM_MUTEXES; i++) {
		configASSERT(all_mutexes_ordered[i] != NULL);
		if (all_mutexes_ordered[i] != NULL // avoid null pointer and leave it to watchdog
			&& *(all_mutexes_ordered[i]) != irpow_semaphore) { // can't give the semaphore
			xSemaphoreGive(*(all_mutexes_ordered[i]));
		}
	}
}

/* 
	Sets all task states atomically by suspending the RTOS scheduler and watchdog,
	grabbing all mutexes to ensure no task is suspended while holding one, and
	resuming or suspending tasks as specified, and setting the current
	state variable (atomically in here)
*/
void set_all_task_states(const task_states states, sat_state_t state, sat_state_t prev_sat_state)
{
	// determine whether antenna deployed before taking ALL mutexes
	bool antenna_deployed = antenna_did_deploy();
	
	// IMPORTANT: take all mutexes to ensure no task is suspended while holding one.
	// Note that the ordering here is important to avoid deadlock 
	// (this is a coarse operation, but we consider it fine because these transitions
	// should be fairly rare)
	bool got_all = true;
	uint8_t num_retries = 0;
	do {
		// take all mutexes, but give them back if any fails and try again
		got_all = take_all_mutexes();
		configASSERT(got_all);
		num_retries++;
		
	} while (!got_all && num_retries < TASK_STATE_CHANGE_MUTEX_TAKE_RETRIES);
	
	configASSERT(got_all);
	
	// if we failed to get some mutex, continue on (if we've failed at this point
	// it's probably deadlock, so continue with the state change) (log error of course)
	if (!got_all) {
		// (use watchdog to represent this crazy failure)
		log_error(ELOC_STATE_HANDLING, ECODE_ALL_MUTEX_TIMEOUT, true);
	}
	{
		// NOTE we have the watchdog mutex as required for calling set_single_task_state
		pet_watchdog(); // in case this takes a bit and we're close
		vTaskSuspendAll();
		// values given by external-facing functions
		current_sat_state = state;
		current_task_states = states;

		for (int task_id = 0; task_id < NUM_TASKS; task_id++) {
			// antenna deploy task is independently handled
			if (task_id != ANTENNA_DEPLOY_TASK) {
				set_single_task_state(states.states[task_id], task_id);
			} else {
				set_antenna_deploy_by_sat_state(prev_sat_state, state, antenna_deployed);
			}
		}
		xTaskResumeAll();
	}
	give_all_mutexes();
}

// sets whether this task is running, suspended, or resumes a previous
// state if set to don't care.
// if set to run or suspend, effective immediately
void set_single_task_state(bool running, task_type_t task_id) {
	configASSERT(task_id < NUM_TASKS);
	if (running) {
		task_resume(task_id);
	} else {
		task_suspend(task_id);
	}
}

// suspends the given task if it was not already suspended, and (always) checks it out of the watchdog
void task_suspend(task_type_t task_id) {
	TaskHandle_t* task_handle = task_handles[task_id];
	configASSERT(task_handle != NULL && *task_handle != NULL); // the latter would suspend THIS task

	configASSERT(task_id != BATTERY_CHARGING_TASK
				&& task_id != STATE_HANDLING_TASK 
				&& task_id != WATCHDOG_TASK 
				&& task_id != PERSISTENT_DATA_BACKUP_TASK);

	// always check out of watchdog when called (to be double-sure)
	// this is only called here so doesn't need to be safe
	check_out_task_unsafe(task_id);
	if (task_handle != NULL && *task_handle != NULL
		&& eTaskGetState(*task_handle) != eSuspended) 
	{
		// actually suspend using handle
		vTaskSuspend(*task_handle);
	}
	// if task_handle or it's value was NULL, we're in for a watchdog reset
	// which is what we want because somethings very wrong
}

// resumes the given task if it was suspended, and (always) checks it in to the watchdog
void task_resume(task_type_t task_id)
{
	TaskHandle_t* task_handle = task_handles[task_id];
	configASSERT(task_handle != NULL && *task_handle != NULL);

	// always check in for watchdog when called
	// (in case it wasn't, for example on BOOT)
	// this is only called here so doesn't need to be safe
	check_in_task_unsafe(task_id);

	if (task_handle != NULL && *task_handle != NULL
		&& eTaskGetState(*task_handle) == eSuspended)
	{
		// actually resume task (will be graceful if task_handle task is not actually suspended)
		vTaskResume(*task_handle);
	}
	// if task_handle or it's value was NULL, we're in for a watchdog reset
	// which is what we want because somethings very wrong
}

/* Resumes the given task, safely pausing the watchdog
   NOTE: CURRENTLY SHOULD ONLY BE CALLED ON THE ANTENNA_DEPLOY_TASK */
void task_resume_safe(task_type_t task_id) {
	if (xSemaphoreTake(watchdog_mutex, WATCHDOG_MUTEX_WAIT_TIME_TICKS)) {
		task_resume(task_id);
		xSemaphoreGive(watchdog_mutex);
	} else {
		log_error(ELOC_STATE_HANDLING, ECODE_WATCHDOG_MUTEX_TIMEOUT, false);
	}
}

/************************************************************************/
/* HARDWARE STATE HANDLING                                              */
/************************************************************************/

// allows access to hardware states - make sure you have the mutex if changing
// it or having the expectation that its true for any period of time
struct hw_states* get_hw_states(void) {
	return &hardware_states;
}

bool hardware_state_mutex_take(uint8_t eloc) {
	if (!xSemaphoreTake(hardware_state_mutex, HARDWARE_STATE_MUTEX_WAIT_TIME_TICKS)) {
		log_error(eloc, ECODE_HW_STATE_MUTEX_TIMEOUT, false);
		return false;	
	};
	return true;
}

void hardware_state_mutex_give(void) {
	xSemaphoreGive(hardware_state_mutex);
}