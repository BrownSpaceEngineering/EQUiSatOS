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
#include "stacks/Sensor_Structs.h"
#include "stacks/State_Structs.h"
#include "stacks/equistack.h" 
#include "./runnable_configurations/init_rtos_tasks.h"
#include "sensor_drivers/sensor_read_commands.h"
#include "watchdog_task.h"

/************************************************************************/
/* TASK HEADERS                                                         */
/************************************************************************/
// ?
extern void vApplicationStackOverflowHook(TaskHandle_t *pxTask,
signed char *pcTaskName);
extern void vApplicationIdleHook(void); // lowest-priority task... may be used for switching to lower power / other modes
extern void vApplicationTickHook(void);

/************************************************************************************/ 
/* All main satellite tasks
	NOTE:
	If you add/remove a task, there are several things you must change:
	1. Actually implement the task (in this .h and the .c)
	2. Add the task to the tasks enum (above) AND update NUM_TASKS
	3. Specify a STACK_SIZE and PRIORITY for the task (above)
	4. Specify a FREQ in rtos_task_frequencies.h FOR ALL relevant data_type
	5. (Maybe) Create new struct to store all data and add data arrays for all data
	6. (Maybe) Create new EQUIstack to store these structs							*/
/************************************************************************************/ 

// Action tasks 
void watchdog_task(void *pvParameters);
void antenna_deploy_task(void *pvParameters);
void battery_charging_task(void *pvParameters);
void transmit_task(void *pvParameters);
void flash_activate_task(void *pvParameters);

// Data read tasks
void current_data_task(void *pvParameters);
void transmit_data_task(void *pvParameters);
void flash_data_task(void *pvParameters);
void attitude_data_task(void *pvParameters);

/******************************************************************************/
/* Global static memory allocated for tasks; stack and data structure holding */
/******************************************************************************/
StaticTask_t watchdog_task_buffer;
StackType_t watchdog_task_stack					[TASK_WATCHDOG_STACK_SIZE];
StaticTask_t antenna_deploy_task_buffer;
StackType_t antenna_deploy_task_stack			[TASK_ANTENNA_DEPLOY_STACK_SIZE];
StaticTask_t battery_charging_task_buffer;
StackType_t battery_charging_task_stack			[TASK_BATTERY_CHARGING_STACK_SIZE];
StaticTask_t transmit_task_buffer;
StackType_t transmit_task_stack					[TASK_TRANSMIT_STACK_SIZE];
StaticTask_t flash_activate_task_buffer;
StackType_t flash_activate_task_stack			[TASK_FLASH_ACTIVATE_STACK_SIZE];
StaticTask_t current_data_task_buffer;
StackType_t current_data_task_stack				[TASK_CURRENT_DATA_RD_STACK_SIZE];
StaticTask_t flash_data_task_buffer;
StackType_t flash_data_task_stack				[TASK_FLASH_DATA_RD_STACK_SIZE];
StaticTask_t attitude_data_task_buffer;
StackType_t attitude_data_task_stack			[TASK_ATTITUDE_DATA_RD_STACK_SIZE];

/************************************************************************/
/* Equistack definitions                                                */
/************************************************************************/
equistack last_reading_type_equistack; // of msg_data_type_t
equistack idle_readings_equistack; // of idle_data_t
equistack attitude_readings_equistack; // of attitude_data_t
equistack flash_readings_equistack; // of flash_data_t
equistack flash_cmp_readings_equistack; // of flash_cmp_t

/* Global (but don't use them!) arrays used in equistack (put here as an alternative to mallocing) */
msg_data_type_t _last_reading_equistack_arr[LAST_READING_TYPE_STACK_MAX];
idle_data_t _idle_equistack_arr[IDLE_STACK_MAX];
flash_data_t _flash_equistack_arr[FLASH_STACK_MAX];
flash_cmp_t _flash_cmp_equistack_arr[FLASH_CMP_STACK_MAX];
attitude_data_t _attitude_equistack_arr[ATTITUDE_STACK_MAX];

/* Global (but don't use them!) mutex data and mutex handles used inside equistacks (alt. to malloc) */
StaticSemaphore_t _last_reading_type_equistack_mutex_d;
SemaphoreHandle_t _last_reading_type_equistack_mutex;
StaticSemaphore_t _idle_equistack_mutex_d;
SemaphoreHandle_t _idle_equistack_mutex;
StaticSemaphore_t _attitude_equistack_mutex_d;
SemaphoreHandle_t _attitude_equistack_mutex;
StaticSemaphore_t _flash_equistack_mutex_d;
SemaphoreHandle_t _flash_equistack_mutex;
StaticSemaphore_t _flash_cmp_equistack_mutex_d;
SemaphoreHandle_t _flash_cmp_equistack_mutex;

/************************************************************************/
/* TASK STATE MANAGEMENT                                               */
/************************************************************************/
/* List of task handles, indexed by the task id enum value 
   (essentially a map) */
TaskHandle_t task_handles[NUM_TASKS];

/* Task handles for starting and stopping */
TaskHandle_t watchdog_task_handle; // Should we have this?
TaskHandle_t antenna_deploy_task_handle;
TaskHandle_t battery_charging_task_handle; // Should we have this?
TaskHandle_t flash_activate_task_handle;
TaskHandle_t transmit_task_handle;
TaskHandle_t current_data_task_handle;
TaskHandle_t flash_data_task_handle;
TaskHandle_t attitude_data_task_handle;

/* List (series of bits) of whether a given task is RESUMING from suspension.
 * NOTE: a bit does NOT indicate the task is suspended. The time sequence is this:
 * 
 *				|---suspended----|       | task reports unsuspended
 * task bit:  0   0   0   0   0    1   1   0   0   0   ...
 * 
 * This was put here to allow tasks to immediately write old data to their
 * equistacks when resuming from suspend.
 * NOTE: We WILL merge this with the watchdog check in / check out indicators.
 */
uint8_t task_suspended_states; // no tasks suspended

/************************************************************************/
/* Task Control Functions                                               */
/************************************************************************/
void pre_init_rtos_tasks(void); // MUST be called on startup to setup assign various constants
void task_suspend(task_type_t task_id);
void task_resume_if_suspended(task_type_t taskId);
bool check_if_suspended_and_update(task_type_t task_id); /* Checks and returns whether this task was suspended, AND report that it is resuming from suspend */

/************************************************************************/
/* Helper Functions                                                     */
/************************************************************************/
void increment_data_type(uint16_t data_type, int *data_array_tails, int *loops_since_last_log);
uint32_t get_current_timestamp(void);
void increment_all(uint8_t* int_arr, uint8_t length);
void set_all(uint8_t* int_arr, uint8_t length, int value);

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
