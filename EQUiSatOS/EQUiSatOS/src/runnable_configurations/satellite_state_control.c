/*
 * init_rtos_tasks.c
 *
 * Created: 10/4/2016 8:50:22 PM
 *  Author: mckenna
 */

#include "satellite_state_control.h"
#include "../testing_functions/os_system_tests.h"

/* Satellite state - ONLY accessible in this file */
int8_t CurrentState;

void run_rtos()
{
	/************************************************************************/
	/* ESSENTIAL INITIALIZATION                                             */
	/************************************************************************/
	
	CurrentState = INITIAL;
	
	//configure_i2c_master(SERCOM4);
	pre_init_rtos_tasks(); // populate task_handles array and setup constants

	// watchdog has some extra initialization
	watchdog_init();

	// Initialize EQUiStack mutexes
	_idle_equistack_mutex = xSemaphoreCreateMutexStatic(&_idle_equistack_mutex_d);
	_attitude_equistack_mutex = xSemaphoreCreateMutexStatic(&_attitude_equistack_mutex_d);
	_flash_equistack_mutex = xSemaphoreCreateMutexStatic(&_flash_equistack_mutex_d);
	_flash_cmp_equistack_mutex = xSemaphoreCreateMutexStatic(&_flash_cmp_equistack_mutex_d);
	_low_power_equistack_mutex = xSemaphoreCreateMutexStatic(&_low_power_equistack_mutex_d);

	// Initialize EQUiStacks
	equistack_Init(&idle_readings_equistack, &_idle_equistack_arr,
		sizeof(idle_data_t), IDLE_STACK_MAX, &_idle_equistack_mutex);
	equistack_Init(&attitude_readings_equistack, &_attitude_equistack_arr,
		sizeof(attitude_data_t), ATTITUDE_STACK_MAX, &_attitude_equistack_mutex);
	equistack_Init(&flash_readings_equistack, &_flash_equistack_arr,
		sizeof(flash_data_t), FLASH_STACK_MAX, &_flash_equistack_mutex);
 	equistack_Init(&flash_cmp_readings_equistack, &_flash_cmp_equistack_arr,
		sizeof(flash_cmp_data_t), FLASH_CMP_STACK_MAX, &_flash_cmp_equistack_mutex);
 	equistack_Init(&low_power_readings_equistack, &_low_power_equistack_arr,
		sizeof(low_power_data_t), LOW_POWER_STACK_MAX, &_low_power_equistack_mutex);

	/************************************************************************/
	/* TASK CREATION                                                        */
	/************************************************************************/

	// Started at boot
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

	 // TODO: Should we even store this handle?
	watchdog_task_handle = xTaskCreateStatic(watchdog_task,
		"watchdog task",
		TASK_WATCHDOG_STACK_SIZE,
		NULL,
		TASK_WATCHDOG_STACK_PRIORITY,
		watchdog_task_stack,
		&watchdog_task_buffer);

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

// 		xTaskCreate(task_suicide_test,
// 		"task suicide testing",
// 		TASK_SENS_RD_IDLE_STACK_SIZE,
// 		NULL,
// 		TASK_SENS_RD_IDLE_PRIORITY,
// 		suicide_test_handle);

// 		xTaskCreate(task_stack_size_overflow_test,
// 			"task stack size purpose test",
// 			320/sizeof(portSTACK_TYPE), // 10 bytes of stack space
// 			NULL,
// 			tskIDLE_PRIORITY,
// 			suicide_test_handle);

  /*xTaskCreate(test_free,
  		"Tests the freeing of structs",
  		TASK_SENS_RD_IDLE_STACK_SIZE,
  		NULL,
  		TASK_SENS_RD_IDLE_PRIORITY,
  		NULL);*/

	// NOTE: We can't actually set task state before RTOS is started below,
	// and tasks start as active (resumed), so we have the task themselves set
	// their initial state (if you look at task you'll see most
	// will suspend themselves on creation.)

	/* Start the tasks and timer running. */
	vTaskStartScheduler();
}

/* Given a task handle, initializes the task to the correct startup state - called be each task when it starts */
void init_task_state(task_type_t task) {
	// TODO: will depend on persistent memory
	
	
	// NOTE: The last "true" argument signifies that the suspend functions should
	// call vTaskSuspend(NULL);, which we MUST use to suspend (vs. task handles)
	// when RTOS is first starting
	switch (task) {
		case BATTERY_CHARGING_TASK:
			task_resume(BATTERY_CHARGING_TASK); // REAL ONE
			return;
		case ANTENNA_DEPLOY_TASK:
			task_suspend(ANTENNA_DEPLOY_TASK); // REAL ONE
			return;
		case IDLE_DATA_TASK:
			task_suspend(IDLE_DATA_TASK); // REAL ONE
			return;
		case FLASH_ACTIVATE_TASK:
			task_suspend(FLASH_ACTIVATE_TASK); // REAL ONE
			return;
		case TRANSMIT_TASK:
			task_suspend(TRANSMIT_TASK); // REAL ONE
			return;
		case ATTITUDE_DATA_TASK:
			task_resume(ATTITUDE_DATA_TASK); // REAL ONE
			return;
		case LOW_POWER_DATA_TASK:
			task_suspend(LOW_POWER_DATA_TASK); // REAL ONE
			return;
		default:
			return;
	}
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
	
	test_normal_satellite_state_sequence();
//	test_all_state_transitions();
// 	test_watchdog_reset_bat_charging();
// 	test_watchdog_reset_bat_charging();
// 	test_watchdog_reset_attitude_data();
// 	test_watchdog_reset_antenna_deploy();
// 	test_watchdog_reset_transmit_task();
// 	test_watchdog_reset_idle_data_task();
// 	test_watchdog_reset_flash_activate_task();
// 	test_watchdog_reset_low_power_data_task();
}

/************************************************************************/
/* STATE SETTING METHODS                                                */
/************************************************************************/
void set_state_initial(void);
void set_state_antenna_deploy(void);
void set_state_hello_world(void);
void set_state_idle_no_flash(void);
void set_states_of_idle_no_flash(void);
void set_state_idle_flash(void);
void set_state_low_power(void);
void set_state_rip(void);

/* Getter for global state */
global_state_t get_sat_state(void) {
	return CurrentState;
}

/* Sets the current satellite state to the given state, if a transition from
   the current state is valid; returns whether the state change was made (i.e. was valid) 
   CAN be called consistenly (i.e. if state == CurrentState), which will ensure all correct tasks
   for a state are running, etc. */
bool set_sat_state_helper(global_state_t state) 
{
	// setting the state to the current state is not wrong,
	// but we don't want to actually change task states every time this is called
	// when the state is the current state
	if (state == CurrentState) {
		return true;
	}
	
	switch(state) 
	{
		case INITIAL:
			return false;
			
		case ANTENNA_DEPLOY:
			if (CurrentState == INITIAL) {
				trace_print("CHANGED STATE to ANTENNA_DEPLOY");
				set_state_antenna_deploy();
				return true;
			}
			return false;
			
		case HELLO_WORLD:
			if (CurrentState == ANTENNA_DEPLOY) {
				trace_print("CHANGED STATE to HELLO_WORLD");
				set_state_hello_world();
				return true;
			}
			return false;
			
		case IDLE_NO_FLASH:
			if (CurrentState == IDLE_FLASH || CurrentState == HELLO_WORLD || CurrentState == LOW_POWER) {
				trace_print("CHANGED STATE to IDLE_NO_FLASH");
				set_state_idle_no_flash();
				return true;
			}
			return false;
			
		case IDLE_FLASH:
			if (CurrentState == IDLE_NO_FLASH) {
				trace_print("CHANGED STATE to IDLE_FLASH");
				set_state_idle_flash();
				return true;
			}
			return false;
			
		case LOW_POWER:
			if (CurrentState == IDLE_NO_FLASH || CurrentState == IDLE_FLASH) {
				trace_print("CHANGED STATE to LOW_POWER");
				set_state_low_power();
				return true;
			}
			return false;
		
		case RIP:
			// we can always go to RIP
			trace_print("CHANGED STATE to RIP");
			set_state_rip();
			return true;
	}
	configASSERT(false); // bad state ID
}

bool set_sat_state(global_state_t state) {
	bool valid = set_sat_state_helper(state);
// 	if (!valid) {
// 		configASSERT(false); // busy loop because this is bad
// 	}
	return valid;
}

void set_state_initial()
{
	// Don't allow other tasks to run while we're changing state
	vTaskSuspendAll();

	CurrentState = INITIAL;

	task_resume(BATTERY_CHARGING_TASK); // should never be stopped
	task_suspend(ANTENNA_DEPLOY_TASK);
	task_suspend(IDLE_DATA_TASK);
	task_suspend(FLASH_ACTIVATE_TASK);
	task_suspend(TRANSMIT_TASK);
	task_resume(ATTITUDE_DATA_TASK);
	task_suspend(LOW_POWER_DATA_TASK);

	xTaskResumeAll();
}

void set_state_antenna_deploy()
{
	// Don't allow other tasks to run while we're changing state
	vTaskSuspendAll();

	CurrentState = ANTENNA_DEPLOY;

	task_resume(BATTERY_CHARGING_TASK); // should never be stopped
	task_resume(ANTENNA_DEPLOY_TASK);
	task_suspend(IDLE_DATA_TASK);
	task_suspend(FLASH_ACTIVATE_TASK);
	task_suspend(TRANSMIT_TASK);
	task_resume(ATTITUDE_DATA_TASK);
	task_suspend(LOW_POWER_DATA_TASK); 

	xTaskResumeAll();
}

void set_state_hello_world()
{
	// Don't allow other tasks to run while we're changing state
	vTaskSuspendAll();

	CurrentState = HELLO_WORLD;

	task_resume(BATTERY_CHARGING_TASK); // should never be stopped
	task_resume(IDLE_DATA_TASK);
	task_suspend(FLASH_ACTIVATE_TASK);
	task_resume(TRANSMIT_TASK);
	task_resume(ATTITUDE_DATA_TASK);
	task_suspend(LOW_POWER_DATA_TASK);

	xTaskResumeAll();
	
	// this must be at the end because this state change will be 
	// triggered by the ANTENNA_DEPLOY_TASK, and suspending the
	// task will suspend this function
	task_suspend(ANTENNA_DEPLOY_TASK); 
}

void set_state_idle_flash()
{
	// Don't allow other tasks to run while we're changing state
	vTaskSuspendAll();

	CurrentState = IDLE_FLASH;

	task_resume(BATTERY_CHARGING_TASK); // should never be stopped
	task_suspend(ANTENNA_DEPLOY_TASK);
	task_resume(IDLE_DATA_TASK);
	task_resume(FLASH_ACTIVATE_TASK);
	task_resume(TRANSMIT_TASK);
	task_resume(ATTITUDE_DATA_TASK);
	task_suspend(LOW_POWER_DATA_TASK);

	xTaskResumeAll();
}

/* Shortcut because several states are technically the same as idle not flash, 
   but just lead to different operations due to the CurrentState global */
void set_states_of_idle_no_flash() {
	task_resume(BATTERY_CHARGING_TASK); // should never be stopped
	task_suspend(ANTENNA_DEPLOY_TASK);
	task_resume(IDLE_DATA_TASK);
	task_suspend(FLASH_ACTIVATE_TASK);
	task_resume(TRANSMIT_TASK);
	task_resume(ATTITUDE_DATA_TASK);
	task_suspend(LOW_POWER_DATA_TASK);
}

void set_state_idle_no_flash()
{
	// Don't allow other tasks to run while we're changing state
	vTaskSuspendAll();

	CurrentState = IDLE_NO_FLASH;

	set_states_of_idle_no_flash();

	xTaskResumeAll();
}

void set_state_low_power()
{
	// Don't allow other tasks to run while we're changing state
	vTaskSuspendAll();

	CurrentState = LOW_POWER;

	task_resume(BATTERY_CHARGING_TASK); // should never be stopped
	task_suspend(ANTENNA_DEPLOY_TASK);
	task_suspend(IDLE_DATA_TASK);
	task_suspend(FLASH_ACTIVATE_TASK);
	task_resume(TRANSMIT_TASK);
	task_suspend(ATTITUDE_DATA_TASK);
	task_resume(LOW_POWER_DATA_TASK); 
	
	xTaskResumeAll();
}

void set_state_rip()
{
	// Don't allow other tasks to run while we're changing state
	vTaskSuspendAll();

	CurrentState = RIP;

	set_states_of_idle_no_flash();
	
	xTaskResumeAll();
}