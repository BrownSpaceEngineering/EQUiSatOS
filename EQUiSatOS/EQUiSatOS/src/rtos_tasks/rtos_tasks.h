/*
 * rtos_tasks.h
 *
 * Created: 10/2/2016 1:47:37 PM
 *  Author: mckenna
 */

#ifndef RTOS_TASKS_H_
#define RTOS_TASKS_H_

/* Includes */

#include <inttypes.h>
#include <assert.h>
#include <global.h>
#include "task.h"
#include "semphr.h"

#include "rtos_tasks_config.h"
#include "data_handling/State_Structs.h"
#include "watchdog_task.h"

/************************************************************************/
/* TASK HEADERS                                                         */
/************************************************************************/
// lowest-priority task... may be used for switching to lower power / other modes
// FOR US, it will eventually do nothing, but currently is used for testing (TODO)
void vApplicationStackOverflowHook(TaskHandle_t xTask,
                                    signed char *pcTaskName);

/************************************************************************************/
/* All main satellite tasks															*/
/************************************************************************************/

// Action tasks
void watchdog_task(void *pvParameters);
void state_handling_task(void *pvParameters);
void antenna_deploy_task(void *pvParameters);
void battery_charging_task(void *pvParameters);
void transmit_task(void *pvParameters);
void flash_activate_task(void *pvParameters);
void persistent_data_backup_task(void *pvParameters);

// Data read tasks
void idle_data_task(void *pvParameters);
void transmit_data_task(void *pvParameters);
void attitude_data_task(void *pvParameters);
void low_power_data_task(void *pvParameters);

/******************************************************************************/
/* Global static memory allocated for tasks; stack and data structure holding */
/******************************************************************************/
StaticTask_t init_task_buffer;
StackType_t init_task_stack						[TASK_INIT_STACK_SIZE];
StaticTask_t watchdog_task_buffer;
StackType_t watchdog_task_stack					[TASK_WATCHDOG_STACK_SIZE];
StaticTask_t state_handling_task_buffer;
StackType_t state_handling_task_stack			[TASK_STATE_HANDLING_STACK_SIZE];
StaticTask_t antenna_deploy_task_buffer;
StackType_t antenna_deploy_task_stack			[TASK_ANTENNA_DEPLOY_STACK_SIZE];
StaticTask_t battery_charging_task_buffer;
StackType_t battery_charging_task_stack			[TASK_BATTERY_CHARGING_STACK_SIZE];
StaticTask_t transmit_task_buffer;
StackType_t transmit_task_stack					[TASK_TRANSMIT_STACK_SIZE];
StaticTask_t flash_activate_task_buffer;
StackType_t flash_activate_task_stack			[TASK_FLASH_ACTIVATE_STACK_SIZE];
StaticTask_t idle_data_task_buffer;
StackType_t idle_data_task_stack				[TASK_IDLE_DATA_RD_STACK_SIZE];
StaticTask_t attitude_data_task_buffer;
StackType_t attitude_data_task_stack			[TASK_ATTITUDE_DATA_RD_STACK_SIZE];
StaticTask_t low_power_data_task_buffer;
StackType_t low_power_data_task_stack			[TASK_LOW_POWER_DATA_RD_STACK_SIZE];
StaticTask_t persistent_data_backup_task_buffer;
StackType_t persistent_data_backup_task_stack	[TASK_PERSISTENT_DATA_BACKUP_STACK_SIZE];

/************************************************************************/
/* Equistack definitions                                                */
/************************************************************************/
equistack idle_readings_equistack; // of idle_data_t
equistack attitude_readings_equistack; // of attitude_data_t
equistack flash_readings_equistack; // of flash_data_t
equistack flash_cmp_readings_equistack; // of flash_cmp_t
equistack low_power_readings_equistack; // of low_power_data_t

/* Global (but don't use them!) arrays used in equistack (put here as an alternative to mallocing) */
idle_data_t _idle_equistack_arr			[IDLE_STACK_MAX];
attitude_data_t _attitude_equistack_arr	[ATTITUDE_STACK_MAX];
flash_data_t _flash_equistack_arr		[FLASH_STACK_MAX];
flash_cmp_data_t _flash_cmp_equistack_arr	[FLASH_CMP_STACK_MAX];
idle_data_t _low_power_equistack_arr	[LOW_POWER_STACK_MAX];

/* # of mutexes (for sat state handling) */
#if (PRINT_DEBUG == 1 || PRINT_DEBUG == 3) && defined(SAFE_PRINT)
	// to be technically correct with prints
	#define NUM_MUTEXES			14
#else 
	#define NUM_MUTEXES			13
#endif

/* Global (but don't use them!) mutex data and mutex handles used inside equistacks (alt. to malloc) */
StaticSemaphore_t _idle_equistack_mutex_d;
SemaphoreHandle_t _idle_equistack_mutex;
StaticSemaphore_t _attitude_equistack_mutex_d;
SemaphoreHandle_t _attitude_equistack_mutex;
StaticSemaphore_t _flash_equistack_mutex_d;
SemaphoreHandle_t _flash_equistack_mutex;
StaticSemaphore_t _flash_cmp_equistack_mutex_d;
SemaphoreHandle_t _flash_cmp_equistack_mutex;
StaticSemaphore_t _low_power_equistack_mutex_d;
SemaphoreHandle_t _low_power_equistack_mutex;

/************************************************************************/
/* TASK STATE MANAGEMENT                                               */
/************************************************************************/
/* List of task handles, indexed by the task id enum value
   (essentially a map) */
TaskHandle_t* task_handles[NUM_TASKS];

/* Task handles for starting and stopping */
TaskHandle_t watchdog_task_handle;
TaskHandle_t state_handling_task_handle;
TaskHandle_t antenna_deploy_task_handle;
TaskHandle_t battery_charging_task_handle;
TaskHandle_t flash_activate_task_handle;
TaskHandle_t transmit_task_handle;
TaskHandle_t idle_data_task_handle;
TaskHandle_t attitude_data_task_handle;
TaskHandle_t low_power_data_task_handle;
TaskHandle_t persistent_data_backup_task_handle;

/************************************************************************/
/* Task Control Functions                                               */
/************************************************************************/
void pre_init_rtos_tasks(void); // MUST be called on startup to setup assign various constants

/************************************************************************/
/* Helper Functions                                                     */
/************************************************************************/
uint32_t get_time_of_next_flash(void);	// implemented in flash_activate_task
bool flash_now(void);					// implemented in flash_activate_task
bool would_flash_now(void);				// implemented in flash_activate_task
equistack* get_msg_type_equistack(msg_data_type_t msg_type);
bool should_exit_antenna_deploy(void);

/************************************************************************/
/*  Required functions for FreeRTOS 9 static allocation                 */
/************************************************************************/
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
	StackType_t **ppxIdleTaskStackBuffer,
	uint32_t *pulIdleTaskStackSize );
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
	StackType_t **ppxTimerTaskStackBuffer,
	uint32_t *pulTimerTaskStackSize );

#endif
