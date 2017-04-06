/*
 * rtos_tasks.h
 *
 * Created: 10/2/2016 1:47:37 PM
 *  Author: mckenna
 */ 

#ifndef RTOS_TASKS_H_
#define RTOS_TASKS_H_

/* Includes */

#include <asf.h>
#include <inttypes.h>
#include "FreeRTOS.h"
#include "task.h"
#include <assert.h>

#include "processor_drivers/I2C_Commands.h"
#include "processor_drivers/SPI_Commands.h"
#include "processor_drivers/ADC_Commands.h"
#include "processor_drivers/Direct_Pin_Commands.h"

#include "sensor_drivers/HMC5883L_Magnetometer_Commands.h"
#include "sensor_drivers/MLX90614_IR_Sensor.h"
#include "sensor_drivers/TEMD6200_Commands.h"
#include "sensor_drivers/switching_commands.h"
#include "sensor_drivers/sensor_read_commands.h"

#include "init_rtos_tasks.h"
#include "stacks/Sensor_Structs.h"
#include "stacks/State_Structs.h"
#include "stacks/equistack.h" 

/************************************************************************/
/* Task Properties - see rtos_task_frequencies.h for frequencies		*/
/************************************************************************/
#define TASK_BATTERY_CHARGE_STACK_SIZE				(1024)/sizeof(portSTACK_TYPE)
#define TASK_BATTERY_CHARGE_PRIORITY				(tskIDLE_PRIORITY)

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
#define LAST_READING_TYPE_STACK_MAX		100
#define IDLE_STACK_MAX					2 // one stored (available for transmission), one staged
#define FLASH_STACK_MAX					10  
#define TRANSMIT_STACK_MAX				10
#define ATTITUDE_STACK_MAX				10

// Don't think we're going to need this due to generally static frequencies
/* Enum for all tasks (to allow for array-wise referencing for freq, etc.) */
// typedef enum
// {
// 	LED_TASK,
// 	RADIO_TRANSMIT_TASK,
// 	SENSOR_READ_IDLE,
// 	SENSOR_READ_FLASH,
// 	SENSOR_READ_BOOT,
// 	SENSOR_READ_LOW_POWER,
// 	NUM_TASKS
// } task_type_t;

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

/* enum for all types of data that can be read 
	(all types that will be in the 'data' section of a message packet) */
typedef enum
{
	ATTITUDE_DATA,
	TRANSMIT_DATA,
	FLASH_DATA
} msg_data_type_t;

/* Task headers */

// ?
extern void vApplicationStackOverflowHook(TaskHandle_t *pxTask,
	signed char *pcTaskName);
extern void vApplicationIdleHook(void); // lowest-priority task... may be used for switching to lower power / other modes
extern void vApplicationTickHook(void);

/* All main satellite tasks
   NOTE:
   If you add/remove a task, there are several things you must change:
	   1. Actually implement the task (in this .h and the .c)
	   2. Add the task to the tasks enum (above) AND update NUM_TASKS
	   3. Specify a STACK_SIZE and PRIORITY for the task (above)
	   4. Specify a FREQ in rtos_task_frequencies.h FOR ALL relevant data_type
	   5. (Maybe) Create new struct to store all data and add data arrays for all data
	   6. (Maybe) Create new EQUIstack to store these structs
*/

// Action tasks 
void watchdog_task(void *pvParameters);
void antenna_deploy_task(void *pvParameters);
void battery_charging_task(void *pvParameters);
void transmit_task(void *pvParameters);
void flash_activate_task(void *pvParameters);

// Data read tasks (some are actually action tasks)
void current_data_task(void *pvParameters);
void transmit_data_task(void *pvParameters);
void flash_data_task(void *pvParameters);
void attitude_data_task(void *pvParameters);

/* Equistack definitions */
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

void increment_data_type(uint16_t data_type, int *data_array_tails, int *loops_since_last_log);

/* Helper Functions */
uint32_t get_current_timestamp(void);
void increment_all(uint8_t* int_arr, uint8_t length);
void set_all(uint8_t* int_arr, uint8_t length, int value);
char* hex_str_of(void* data, int bytes);

#endif