/*
 * rtos_tasks.c
 *
 * Created: 9/27/2016 8:21:58 PM
 *  Author: rj16
 */ 

#include "rtos_tasks.h"
#include "processor_drivers\USART_Commands.h"

/************************************************************************/
/* TASK CONTROL FUNCTIONS                                               */
/************************************************************************/
static void init_task_handles(void) {
	//task_handles[WATCHDOG_TASK] = &watchdog_task_handle;
	task_handles[ANTENNA_DEPLOY_TASK] =			&antenna_deploy_task_handle;
	task_handles[BATTERY_CHARGING_TASK] =		&battery_charging_task_handle;
	task_handles[FLASH_ACTIVATE_TASK] =			&flash_activate_task_handle;
	task_handles[TRANSMIT_TASK] =				&transmit_task_handle;
	task_handles[IDLE_DATA_TASK] =				&idle_data_task_handle;
	task_handles[LOW_POWER_DATA_TASK] =			&low_power_data_task_handle;
	task_handles[FLASH_DATA_TASK] =				&flash_data_task_handle;
	task_handles[ATTITUDE_DATA_TASK] =			&attitude_data_task_handle;
	task_handles[LOW_POWER_DATA_TASK] =			&low_power_data_task_handle;
}

void pre_init_rtos_tasks(void) {
	task_suspended_states = 0; // no suspended tasks
	init_task_handles();
}

// NOTE: task_id must be the id of the current task if suspend_current_task is true
void task_suspend(task_type_t task_id) {
	TaskHandle_t* task_handle = task_handles[task_id];
		
	if (task_handle == NULL || *task_handle != NULL) { // the latter would suspend THIS task
		// TODO: ERROR! - maybe even watchdog reset!
		return;
	}
		
	if (eTaskGetState(*task_handle) != eSuspended) { 
		check_out_task(task_id); // check out of watchdog
		vTaskSuspend(*task_handle); // actually suspend using handle
	}
}

void task_resume(task_type_t task_id)
{
	TaskHandle_t* task_handle = task_handles[task_id];
	if (task_handle == NULL) {
		// TODO: ERROR! - maybe even watchdog reset!
		return;
		// this is also good in the case the task_handle is NULL and the task isn't started
		// here, because it will trigger a watchdog reset
	}
		
	if (eTaskGetState(*task_handle) == eSuspended)
	{
		check_in_task(task_id); // check in for watchdog whenever called (in case it wasn't)
		// actually resume task (will be graceful if task_handle task is not actually suspended)
		vTaskResume(*task_handle); 
		task_suspended_states |= (1 << task_id); // note we WERE suspended
	}	
}

/* Checks and returns whether this task was suspended, AND report that it is not suspended */
bool check_if_suspended_and_update(task_type_t task_id) {
	bool val = task_suspended_states & (1 << task_id); // check the state (>0 if was suspended)
	task_suspended_states &= ~(1 << task_id); // set our suspended bit to 0
	return val;
}

/************************************************************************/
/* Helper Functions														*/
/************************************************************************/
void increment_data_type(uint16_t data_type, uint8_t *data_array_tails, uint8_t *loops_since_last_log)
{
	// increment array tail marker and reset reads-per-log counter
	data_array_tails[data_type] = data_array_tails[data_type] + 1;
	loops_since_last_log[data_type] = 0;
}

void increment_all(uint8_t* int_arr, uint8_t length)
{
	for(uint8_t i = 0; i < length; i++)
	{
		int_arr[i] = int_arr[i] + 1;
	}
}

void set_all(uint8_t* int_arr, uint8_t length, int value)
{
	for(uint8_t i = 0; i < length; i++)
	{
		int_arr[i] = value;
	}
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName) {
	while (1) {}; // will hang here on stack overflow
}

/************************************************************************/
/* Required functions for FreeRTOS 9 static allocation					*/
/* Copied from http://www.freertos.org/a00110.html						*/
/************************************************************************/

/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
	/* If the buffers to be provided to the Idle task are declared inside this
	function then they must be declared static - otherwise they will be allocated on
	the stack and so not exists after this function exits. */
	static StaticTask_t xIdleTaskTCB;
	static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize )
{
	/* If the buffers to be provided to the Timer task are declared inside this
	function then they must be declared static - otherwise they will be allocated on
	the stack and so not exists after this function exits. */
	static StaticTask_t xTimerTaskTCB;
	static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
    task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
