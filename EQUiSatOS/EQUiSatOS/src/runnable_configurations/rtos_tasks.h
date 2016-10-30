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

#include "processor_drivers/I2C_Commands.h"
#include "processor_drivers/SPI_Commands.h"
#include "processor_drivers/ADC_Commands.h"
#include "processor_drivers/Direct_Pin_Commands.h"

#include "sensor_drivers/HMC5883L_Magnetometer_Commands.h"
#include "sensor_drivers/MLX90614_IR_Sensor.h"
#include "sensor_drivers/TEMD6200_Commands.h"
#include "sensor_drivers/switching_commands.h"

#include "radio/Stacks/Sensor_Structs.h"
#include "radio/Stacks/State_Structs.h"

/* Task Properties - see rtos_task_frequencies.h for frequencies */

#define TASK_LED_STACK_SIZE						(1024/sizeof(portSTACK_TYPE))
#define TASK_LED_PRIORITY						(tskIDLE_PRIORITY)

#define TASK_RADIO_TRANSMIT_STACK_SIZE			(1024/sizeof(portSTACK_TYPE))
#define TASK_RADIO_TRANSMIT_PRIORITY			(tskIDLE_PRIORITY)

#define TASK_SENS_RD_IDLE_STACK_SIZE			(1024/sizeof(portSTACK_TYPE))
#define TASK_SENS_RD_IDLE_PRIORITY				(tskIDLE_PRIORITY)

#define TASK_SENS_RD_FLASH_STACK_SIZE			(1024/sizeof(portSTACK_TYPE))
#define TASK_SENS_RD_FLASH_PRIORITY				(tskIDLE_PRIORITY)

#define TASK_SENS_RD_BOOT_STACK_SIZE			(1024/sizeof(portSTACK_TYPE))
#define TASK_SENS_RD_BOOT_PRIORITY				(tskIDLE_PRIORITY)

#define TASK_SENS_RD_LOW_POWER_STACK_SIZE		(1024/sizeof(portSTACK_TYPE))
#define TASK_SENS_RD_LOW_POWER_PRIORITY			(tskIDLE_PRIORITY)

/* Enum for all tasks (to allow for array-wise referencing for freq, etc.) */
typedef enum
{
	LED_TASK,
	RADIO_TRANSMIT_TASK,
	SENSOR_READ_IDLE,
	SENSOR_READ_FLASH,
	SENSOR_READ_BOOT,
	SENSOR_READ_LOW_POWER,
} task_type;

#define NUM_TASKS		6

/* Enum for all states
   NOTE:
   If you add/remove a state, there are several things you must change:
	   1. Actually implement the task (in this .h and the .c)
	   2. Add the task to the tasks enum (above) AND update NUM_TASKS
	   3. Specify a STACK_SIZE and PRIORITY for the task (above)
	   4. Specify a FREQ in rtos_task_frequencies.h FOR ALL relevant data_type
	   5. Create new struct to store all data and add data arrays for all data
	   6. Create new EQUIstack to store these structs
	   7. Create copied versions of all stack functions for that stack type
*/
typedef enum 
{
	IDLE,
	FLASH,
	BOOT,
	LOW_POWER
} state_type;

/* Enum for all types of collected data (for consistency across sensor read functions) 
   Based off: https://docs.google.com/spreadsheets/d/1sHQNTC5f5sg6j5DD4OKjuQykpIM3z16uetWT9YuB9PQ/edit 
   NOTE:
	If you add/remove a type of collected data, there are several things you must change:
		1. Add the task to the tasks data_types (below) AND update NUM_DATA_TYPES
		2. Specify a frequency of logging via a _READS_PER_LOG in rtos_task_frequencies.h
			FOR ALL satellite states (be sure to update MAX_READS_PER_LOG if relevant)
		3. Create a batch type definition
		4. Add a new array of data to ALL of the relevant state structs
		5. Add collection/storing code to radio transmit for all states
 */
typedef enum
{
	IR_DATA,
	TEMP_DATA,
	DIODE_DATA,
	LED_CUR_DATA,
	GYRO_DATA,
	MAGNETOMETER_DATA,
	CHARGING_DATA,
	RADIO_TEMP_DATA,
	BAT_VOLT_DATA,
	REG_VOLT_DATA
} data_types;

#define NUM_DATA_TYPES		10

/* Task headers */

// ?
extern void vApplicationStackOverflowHook(TaskHandle_t *pxTask,
	signed char *pcTaskName);
extern void vApplicationIdleHook(void); // lowest-priority task... may be used for switching to lower power / other modes
extern void vApplicationTickHook(void);

// Action tasks
static void task_antenna_deploy(void *pvParameters); // Will this be a task?
static void task_led(void *pvParameters);
static void task_radio_transmit(void *pvParameters);

// NEW
// have queues for each of these, and inside each of these read at HIGHEST frequency
// but write at different frequencies depending on constants like READ_PER_LOG for each sensor
//, and finally add all the data from these into a big struct corresponding to each struct instead of 
// each sensor

// Sensor read tasks
static void task_data_read_idle(void *pvParameters);
static void task_data_read_flash(void *pvParameters);
static void task_data_read_boot(void *pvParameters);
static void task_data_read_low_power(void *pvParameters);

/* Queue definitions */
static idle_data_t *idle_readings_equistack;
/*static flash_data_t *flash_readings_equistack;*/
/*static boot_data_t *boot_readings_equistack;*/
/*static low_power_data_t *low_power_readings_equistack;*/


/* Helper Functions */
static void increment_all(int* int_arr, int length);

#endif