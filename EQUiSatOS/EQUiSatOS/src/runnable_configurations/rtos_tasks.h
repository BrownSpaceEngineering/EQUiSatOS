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

/* NOTE:
 If you add/remove a task, there are several things you must change:
	1. Actually implement the task (in this .h and the .c)
	2. Add the task to the tasks enum (below) AND update NUM_TASKS
	3. Specify a STACK_SIZE and PRIORITY for the task (below)
	4. Specify a FREQ in rtos_task_frequencies.h FOR ALL satellite states
 */


/* Task Properties - see rtos_task_frequencies.h for frequencies */

#define TASK_LED_STACK_SIZE						(1024/sizeof(portSTACK_TYPE))
#define TASK_LED_PRIORITY						(tskIDLE_PRIORITY)

#define TASK_RADIO_TRANSMIT_STACK_SIZE			(1024/sizeof(portSTACK_TYPE))
#define TASK_RADIO_TRANSMIT_PRIORITY			(tskIDLE_PRIORITY)

#define TASK_STATE_RD_STACK_SIZE				(1024/sizeof(portSTACK_TYPE))
#define TASK_STATE_RD_PRIORITY					(tskIDLE_PRIORITY)

#define TASK_IR_RD_STACK_SIZE					(1024/sizeof(portSTACK_TYPE))
#define TASK_IR_RD_PRIORITY						(tskIDLE_PRIORITY)

#define TASK_TEMP_RD_STACK_SIZE					(1024/sizeof(portSTACK_TYPE))
#define TASK_TEMP_RD_PRIORITY					(tskIDLE_PRIORITY)

#define TASK_DIODE_RD_STACK_SIZE				(1024/sizeof(portSTACK_TYPE))
#define TASK_DIODE_RD_PRIORITY					(tskIDLE_PRIORITY)

#define TASK_LED_CURRENT_RD_STACK_SIZE			(1024/sizeof(portSTACK_TYPE))
#define TASK_LED_CURRENT_RD_PRIORITY			(tskIDLE_PRIORITY)

#define TASK_GYRO_RD_STACK_SIZE					(1024/sizeof(portSTACK_TYPE))
#define TASK_GYRO_RD_PRIORITY					(tskIDLE_PRIORITY)

#define TASK_MAGNETOMETER_RD_STACK_SIZE			(1024/sizeof(portSTACK_TYPE))
#define TASK_MAGNETOMETER_RD_PRIORITY			(tskIDLE_PRIORITY)

#define TASK_CHARGING_DATA_RD_STACK_SIZE		(1024/sizeof(portSTACK_TYPE))
#define TASK_CHARGING_DATA_RD_PRIORITY			(tskIDLE_PRIORITY)

#define TASK_RADIO_TEMP_RD_STACK_SIZE			(1024/sizeof(portSTACK_TYPE))
#define TASK_RADIO_TEMP_RD_PRIORITY				(tskIDLE_PRIORITY)

#define TASK_BAT_VOLTAGE_RD_STACK_SIZE			(1024/sizeof(portSTACK_TYPE))
#define TASK_BAT_VOLTAGE_RD_PRIORITY			(tskIDLE_PRIORITY)

#define TASK_REG_VOLTAGE_RD_STACK_SIZE			(1024/sizeof(portSTACK_TYPE))
#define TASK_REG_VOLTAGE_RD_PRIORITY			(tskIDLE_PRIORITY)

/* Enum for all tasks (to allow for array-wise referencing for freq, etc.) */
enum 
{
	LED_TASK,
	RADIO_TRANSMIT_TASK,
	STATE_READ_TASK,
	IR_READ_TASK,
	TEMP_READ_TASK,
	DIODE_READ_TASK,
	LED_CURRENT_READ_TASK,
	GYRO_READ_TASK,
	MAGNETOMETER_READ_TASK,
	CHARGING_DATA_READ_TASK,
	RADIO_TEMP_READ_TASK,
	BAT_VOLTAGE_READ_TASK,
	REG_VOLTAGE_READ_TASK
} tasks;

#define NUM_TASKS		13

/* Task headers */

// ?
extern void vApplicationStackOverflowHook(TaskHandle_t *pxTask,
	signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);

// Action tasks
static void task_antenna_deploy(void *pvParameters); // Will this be a task?
static void task_led(void *pvParameters);
static void task_radio_transmit(void *pvParameters);

// Sensor read tasks
// TODO: Will need queues for each of these
static void task_state_read(void *pvParameters);
static void task_ir_read(void *pvParameters);
static void task_temp_read(void *pvParameters);
static void task_diode_read(void *pvParameters);
static void task_led_current_read(void *pvParameters);
static void task_gyro_read(void *pvParameters);
static void task_magnetometer_read(void *pvParameters);
static void task_charging_data_read(void *pvParameters);
static void task_radio_temp_read(void *pvParameters);
static void task_bat_voltage_read(void *pvParameters);
static void task_reg_voltage_read(void *pvParameters);

// Other 
static void task_spi_read(void *pvParameters);

#endif