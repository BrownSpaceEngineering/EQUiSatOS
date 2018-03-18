/*
 * rtos_tasks.c
 *
 * Created: 9/27/2016 8:21:58 PM
 *  Author: rj16
 */ 

#include "rtos_tasks.h"
#include "processor_drivers\USART_Commands.h"
#include "data_handling/Sensor_Structs.h"
#include "data_handling/equistack.h"
#include "sensor_drivers/sensor_read_commands.h"

/************************************************************************/
/* TASK CONTROL FUNCTIONS                                               */
/************************************************************************/
static void init_task_handles(void) {
	task_handles[WATCHDOG_TASK] =				&watchdog_task_handle;
	task_handles[STATE_HANDLING_TASK] =			&state_handling_task_handle;
	task_handles[ANTENNA_DEPLOY_TASK] =			&antenna_deploy_task_handle;
	task_handles[BATTERY_CHARGING_TASK] =		&battery_charging_task_handle;
	task_handles[FLASH_ACTIVATE_TASK] =			&flash_activate_task_handle;
	task_handles[TRANSMIT_TASK] =				&transmit_task_handle;
	task_handles[IDLE_DATA_TASK] =				&idle_data_task_handle;
	task_handles[LOW_POWER_DATA_TASK] =			&low_power_data_task_handle;
	task_handles[ATTITUDE_DATA_TASK] =			&attitude_data_task_handle;
	task_handles[LOW_POWER_DATA_TASK] =			&low_power_data_task_handle;
	task_handles[PERSISTENT_DATA_BACKUP_TASK] = &persistent_data_backup_task_handle;
}

void pre_init_rtos_tasks(void) {
	init_task_handles();
}

/************************************************************************/
/* Helper Functions														*/
/************************************************************************/

/* returns the equistack associated with the given message type */
equistack* get_msg_type_equistack(msg_data_type_t msg_type) {
	switch (msg_type) {
		case IDLE_DATA:
			return &idle_readings_equistack;
		case ATTITUDE_DATA:
			return &attitude_readings_equistack;
		case FLASH_DATA:
			return &flash_readings_equistack;
		case FLASH_CMP_DATA:
			return &flash_cmp_readings_equistack;
		case LOW_POWER_DATA:
			return &low_power_readings_equistack;
		default:
			return NULL;
	}
}

/************************************************************************/
/* RTOS HOOKS															*/
/************************************************************************/

void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName) {
	configASSERT(false); // MIGHT hang here on stack overflow
	// if this happens for real, log error, write persistent state, and reset processor
	log_error(ELOC_RTOS, ECODE_STACK_OVERFLOW, true);
	write_state_to_storage_emergency(false); // NOT from ISR; do this to minimize stack usage
	print("STACK OVERFLOW RESET -- RESETTING SATELLITE");
	#ifdef WATCHDOG_RESET_ACTIVE
		system_reset();
	#endif
}

/**
 * The "idle" hook for FreeRTOS - this is is code run in the idle task of RTOS, which
 * runs whenever something else is NOT. It should NOT call hanging RTOS functions
 * or take up much computational power in general.
 * See http://www.freertos.org/RTOS-idle-task.html for more details.
 */
//void vApplicationIdleHook(void) {
	// FOR TESTING
//}

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
