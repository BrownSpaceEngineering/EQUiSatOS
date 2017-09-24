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

#include "runnable_configurations/init_rtos_tasks.h"
#include "stacks/Sensor_Structs.h"
#include "stacks/State_Structs.h"
#include "stacks/equistack.h" 
#include "sensor_drivers/sensor_read_commands.h"

/************************************************************************/
/* Task Properties - see rtos_task_frequencies.h for frequencies		*/
/************************************************************************/
#define TASK_BATTERY_CHARGING_STACK_SIZE			(1024)/sizeof(portSTACK_TYPE)
#define TASK_BATTERY_CHARGING_PRIORITY				(tskIDLE_PRIORITY)

#define TASK_ANTENNA_DEPLOY_STACK_SIZE				(1024/sizeof(portSTACK_TYPE))
#define TASK_ANTENNA_DEPLOY_PRIORITY				(tskIDLE_PRIORITY)

#define TASK_WATCHDOG_STACK_SIZE					(1024/sizeof(portSTACK_TYPE))
#define TASK_WATCHDOG_STACK_PRIORITY				(tskIDLE_PRIORITY)

#define TASK_FLASH_ACTIVATE_STACK_SIZE				(1024/sizeof(portSTACK_TYPE))
#define TASK_FLASH_ACTIVATE_PRIORITY				(tskIDLE_PRIORITY)

#define TASK_TRANSMIT_STACK_SIZE					(1024/sizeof(portSTACK_TYPE))
#define TASK_TRANSMIT_PRIORITY						(tskIDLE_PRIORITY)

#define TASK_CURRENT_DATA_RD_STACK_SIZE				(1024/sizeof(portSTACK_TYPE))
#define TASK_CURRENT_DATA_RD_PRIORITY				(tskIDLE_PRIORITY)

#define TASK_FLASH_DATA_RD_STACK_SIZE				(1024/sizeof(portSTACK_TYPE))
#define TASK_FLASH_DATA_RD_PRIORITY					(tskIDLE_PRIORITY)

#define TASK_TRANSMIT_DATA_RD_STACK_SIZE			(1024/sizeof(portSTACK_TYPE))
#define TASK_TRANSMIT_DATA_RD_PRIORITY				(tskIDLE_PRIORITY)

#define TASK_ATTITUDE_DATA_RD_STACK_SIZE			(1024/sizeof(portSTACK_TYPE))
#define TASK_ATTITUDE_DATA_DATA_RD_PRIORITY			(tskIDLE_PRIORITY)

/********************************************************************************/
/* Data reading task stack sizes - how many they can store before overwriting	*/
/********************************************************************************/
// it doesn't make sense of this to be greater than the sum of the other _MAXs
#define LAST_READING_TYPE_STACK_MAX		32
#define IDLE_STACK_MAX					2 // one stored (available for transmission), one staged (TODO: Isn't the staged one stored anyways?)
#define FLASH_STACK_MAX					10  
#define TRANSMIT_STACK_MAX				10
#define ATTITUDE_STACK_MAX				10

/************************************************************************/
/* Enum for states that represent changes in which tasks are running	*/
/************************************************************************/
typedef enum 
{
	HELLO_WORLD,
	IDLE,
	LOW_POWER
} global_state_t;

/************************************************************************/
/*  Enum for all types of collected sensor readings						*/
/* (for consistency across sensor read functions)						*/
/* Based off: https://docs.google.com/a/brown.edu/spreadsheets/d/1sHQNTC5f5sg6j5DD4OKjuQykpIM3z16uetWT9YuB9PQ/edit?usp=sharing
/*	NOTE:																*/
/*	If you add/remove a type of collected data, there are several		*/
/*	things you must change:												*/
/*		- Create a batch type definition								*/
/*		- Create the required frequencies								*/
/*		- Add a new array of data to ALL of the relevant state structs  */
/*		- Add an add_*_batch_if_ready function						    */
/* NOTE: To move this somewhere, use this regex: (\w*)_DATA, --> $1		*/
/************************************************************************/
typedef enum
{
	LION_VOLTS_DATA,
	LION_CURRENT_DATA,
	LED_TEMPS_DATA,
	LIFEPO_CURRENT_DATA,
	LIFEPO_VOLTS_DATA,
	IR_DATA,
	DIODE_DATA,
	BAT_TEMP_DATA,
	IR_TEMPS_DATA,
	RADIO_TEMP_DATA,
	IMU_DATA,
	MAGNETOMETER_DATA,
	LED_CURRENT_DATA,
	RADIO_VOLTS_DATA,
	BAT_CHARGE_VOLTS_DATA,
	BAT_CHARGE_DIG_SIGS_DATA,
	DIGITAL_OUT_DATA,
	NUM_DATA_TYPES //= DIGITAL_OUT_D + 1
} sensor_type_t;

/************************************************************************/
/* enum for all types of data that can be read							*/
/* (all types that will be in the 'data' section of a message packet)   */
/************************************************************************/
typedef enum
{
	ATTITUDE_DATA,
	TRANSMIT_DATA,
	FLASH_DATA
} msg_data_type_t;

/************************************************************************/
/* Enum for all tasks (for array-wise referencing for last_state, etc.) */
/************************************************************************/
 typedef enum
 {
	ANTENNA_DEPLOY_TASK,
	BATTERY_CHARGING_TASK,
	TRANSMIT_TASK,
	FLASH_ACTIVATE_TASK,
	CURRENT_DATA_TASK,
	ATTITUDE_DATA_TASK,
	TRANSMIT_DATA_TASK,
	FLASH_DATA_TASK
 } task_type_t;

/* Task headers */

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
StaticTask_t transmit_data_task_buffer;
StackType_t transmit_data_task_stack			[TASK_TRANSMIT_DATA_RD_STACK_SIZE];
StaticTask_t flash_data_task_buffer;
StackType_t flash_data_task_stack				[TASK_FLASH_DATA_RD_STACK_SIZE];
StaticTask_t attitude_data_task_buffer;
StackType_t attitude_data_task_stack			[TASK_ATTITUDE_DATA_RD_STACK_SIZE];

/************************************************************************/
/* Equistack definitions                                                */
/************************************************************************/
equistack last_reading_type_equistack; // of msg_data_type_t
equistack idle_readings_equistack; // of idle_data_t
equistack flash_readings_equistack; // of flash_data_t
equistack transmit_readings_equistack; // of transmit_data_t
equistack attitude_readings_equistack; // of attitude_data_t

/* Global (but don't use them!) arrays used in equistack (put here as an alternative to mallocing) */
msg_data_type_t _last_reading_equistack_arr[LAST_READING_TYPE_STACK_MAX];
idle_data_t _idle_equistack_arr[IDLE_STACK_MAX];
flash_data_t _flash_equistack_arr[FLASH_STACK_MAX];
transmit_data_t _transmit_equistack_arr[TRANSMIT_STACK_MAX];
attitude_data_t _attitude_equistack_arr[ATTITUDE_STACK_MAX];

/* Global (but don't use them!) mutex data and mutex handles used inside equistacks (alt. to malloc) */
StaticSemaphore_t _last_reading_type_equistack_mutex_d;
SemaphoreHandle_t _last_reading_type_equistack_mutex;
StaticSemaphore_t _idle_equistack_mutex_d;
SemaphoreHandle_t _idle_equistack_mutex;
StaticSemaphore_t _flash_equistack_mutex_d;
SemaphoreHandle_t _flash_equistack_mutex;
StaticSemaphore_t _transmit_equistack_mutex_d;
SemaphoreHandle_t _transmit_equistack_mutex;
StaticSemaphore_t _attitude_equistack_mutex_d;
SemaphoreHandle_t _attitude_equistack_mutex;

/* Helper Functions */
void taskResumeIfSuspended(TaskHandle_t task_handle, task_type_t taskId);
bool checkIfSuspendedAndUpdate(task_type_t taskId); /* Checks and returns whether this task was suspended, AND report that it is not suspended */

void increment_data_type(uint16_t data_type, int *data_array_tails, int *loops_since_last_log);
uint32_t get_current_timestamp(void);
void increment_all(uint8_t* int_arr, uint8_t length);
void set_all(uint8_t* int_arr, uint8_t length, int value);

#endif
