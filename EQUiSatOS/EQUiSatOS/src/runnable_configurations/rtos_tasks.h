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
#define TASK_TRANSMIT_DATA_RD_PRIORITY			(tskIDLE_PRIORITY)

#define TASK_ATTITUDE_DATA_RD_STACK_SIZE			(1024/sizeof(portSTACK_TYPE))
#define TASK_ATTITUDE_DATA_DATA_RD_PRIORITY			(tskIDLE_PRIORITY)

/********************************************************************************/
/* Data reading task stack sizes - how many they can store before overwriting	*/
/********************************************************************************/
#define IDLE_STACK_MAX			2 // one stored (available for transmission), one staged
#define FLASH_STACK_MAX			10  
#define TRANSMIT_STACK_MAX		10
#define ATTITUDE_STACK_MAX		10

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
/*	NOTE:
/*	If you add/remove a type of collected data, there are several		*/
/*	things you must change:												*/
/*		- Create a batch type definition								*/
/*		- Add a new array of data to ALL of the relevant state structs	*/
/************************************************************************/
typedef enum
{
	IR_DATA,
	TEMP_DATA,
	DIODE_DATA,
	LED_CUR_DATA,
	IMU_DATA,
	MAGNETOMETER_DATA,
	CHARGING_DATA,
	RADIO_TEMP_DATA,
	BAT_VOLT_DATA,
	REG_VOLT_DATA,
	NUM_DATA_TYPES //= REG_VOLT_DATA + 1
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

/* Queue definitions */
equistack* last_reading_type_equistack; // of msg_data_type_t
equistack* idle_readings_equistack; // of idle_data_t
equistack* flash_readings_equistack; // of flash_data_t
equistack* transmit_readings_equistack; // of transmit_data_t
equistack* attitude_readings_equistack; // of attitude_data_t

/* Individual sensor helpers for data reading tasks */
void add_ir_batch_if_ready(ir_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);
void add_temp_batch_if_ready(temp_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);
void add_diode_batch_if_ready(diode_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);
void add_led_current_batch_if_ready(led_current_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);
void add_imu_batch_if_ready(imu_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);
void add_magnetometer_batch_if_ready(magnetometer_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);
void add_charging_batch_if_ready(charging_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);
void add_radio_temp_batch_if_ready(radio_temp_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);
void add_battery_voltages_batch_if_ready(battery_voltages_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);
void add_regulator_voltages_batch_if_ready(regulator_voltages_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);

/* Helper Functions */
uint32_t get_current_timestamp(void);
void increment_all(int* int_arr, int length);
void set_all(int* int_arr, int length, int value);

#endif