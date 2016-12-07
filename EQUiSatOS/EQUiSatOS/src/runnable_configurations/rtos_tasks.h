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

#include "stacks/idle_Stack.h"
#include "stacks/flash_Stack.h"
#include "stacks/boot_Stack.h"
#include "stacks/low_power_Stack.h"
#include "../stacks/Data_Heap.h"

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
	NUM_TASKS
} task_type;

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
   https://docs.google.com/spreadsheets/d/1EGv9MapHfgPCEahHojSbnT92tt32k0kPWfgaWkicNCE/edit
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
	REG_VOLT_DATA,
	NUM_DATA_TYPES //= REG_VOLT_DATA + 1
} data_types;

/* Task headers */

// ?
extern void vApplicationStackOverflowHook(TaskHandle_t *pxTask,
	signed char *pcTaskName);
extern void vApplicationIdleHook(void); // lowest-priority task... may be used for switching to lower power / other modes
extern void vApplicationTickHook(void);

// Action tasks
void task_antenna_deploy(void *pvParameters); // Will this be a task?
void task_led(void *pvParameters);
void task_radio_transmit(void *pvParameters);

// Data read tasks
void task_data_read_idle(void *pvParameters);
void task_data_read_flash(void *pvParameters);
void task_data_read_boot(void *pvParameters);
void task_data_read_low_power(void *pvParameters);

/* Queue definitions */
uint8_t *last_state_read_equistack; // TODO: should these be pointers to num-stacks, idle_sturct_stacks etc.
idle_data_t *idle_readings_equistack; 
flash_data_t *flash_readings_equistack;
/*static boot_data_t *boot_readings_equistack;*/
/*static low_power_data_t *low_power_readings_equistack;*/

data_t_heap *idle_data_t_heap;
// data_t_heap *flash_data_t_heap;
// data_t_heap *boot_data_t_heap;
// data_t_heap *low_power_data_t_heap;

/* Individual sensor helpers for data reading tasks */
void add_ir_batch_if_ready(ir_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);
void add_temp_batch_if_ready(temp_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);
void add_diode_batch_if_ready(diode_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);
void add_led_current_batch_if_ready(led_current_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);
void add_gyro_batch_if_ready(gyro_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);
void add_magnetometer_batch_if_ready(magnetometer_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);
void add_charging_batch_if_ready(charging_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);
void add_radio_temp_batch_if_ready(radio_temp_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);
void add_battery_voltages_batch_if_ready(battery_voltages_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);
void add_regulator_voltages_batch_if_ready(regulator_voltages_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);

/* Helper Functions */
uint32_t get_current_timestamp(void);
void increment_all(int* int_arr, int length);

#endif