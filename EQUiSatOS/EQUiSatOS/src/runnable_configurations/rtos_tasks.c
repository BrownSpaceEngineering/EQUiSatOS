/*
 * rtos_tasks.c
 *
 * Created: 9/27/2016 8:21:58 PM
 *  Author: rj16
 */ 

#include "rtos_tasks.h"

// basic example task
/*
static void task_basic(void *pvParameters)
{
	// initialize xNextWakeTime once
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	for( ;; )
	{
		// block for a time based on this task's globally-set frequency 
		// (Note: changes to the frequency can be delayed in taking effect by as much as the past frequency)
		vTaskDelayUntil( &xNextWakeTime, taskFrequencies[ task enum ID from tasks enum ] / portTICK_PERIOD_MS);

		// get data from sensor
		[particular data struct] valueToSend = [compile particular data struct];

		// TODO: Maybe ignore data if we deem it "not interesting"?? i.e. if it's the same as last time

		// send to the queue, blocking INDEFINITELY (portMAX_DELAY) until we are able to add
		// TODO: Should we wait forever???
		xQueueSend( [this sensor's data queue], &valueToSend, portMAX_DELAY );
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}
*/

// write this by comparing the current frequencies to the frequencies associated with a certain state
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