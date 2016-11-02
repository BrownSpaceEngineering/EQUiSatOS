/*
 * rtos_tasks.c
 *
 * Created: 9/27/2016 8:21:58 PM
 *  Author: rj16
 */ 

#include "rtos_tasks.h"
#include "init_rtos_tasks.h"

/* Helper Functions */
static void increment_all(int* int_arr, int length)
{
	for(int i = 0; i < length; i++)
	{
		int_arr[i] = int_arr[i] + 1;
	}
}

/* Action Tasks */
static void task_radio_transmit(void *pvParameters)
{
	// initialize xNextWakeTime once
	TickType_t xNextWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		// block for a time based on this task's globally-set frequency
		// (Note: changes to the frequency can be delayed in taking effect by as much as the past frequency...)
		vTaskDelayUntil( &xNextWakeTime, taskFrequencies[RADIO_TRANSMIT_TASK] / portTICK_PERIOD_MS);
		
		// read the most recent state 
		int lastState = NULL; // pop something off state queue ;
		
		// based on what state we're in, compile a different message
		switch(lastState)
		{
			case IDLE: PASS;
			case FLASH: PASS;
			case BOOT: PASS;
			case LOW_POWER: PASS;
		};
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}

/* Sensor Read Tasks */

// basic example sensor read task --------DEPRECATED---------
/*
static void task_basic(void *pvParameters)
{
	// initialize xNextWakeTime once
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	// local variable for determining whether to add given data to queue
	int dataReadSinceLastLog = 0;
	
	for( ;; )
	{
		// block for a time based on this task's globally-set frequency 
		// (Note: changes to the frequency can be delayed in taking effect by as much as the past frequency...)
		vTaskDelayUntil( &xNextWakeTime, taskFrequencies[ task enum ID from tasks enum ] / portTICK_PERIOD_MS);

		dataReadSinceLastLog++;
		// get data from sensor
		[particular data struct] valueToSend = [compile particular data struct];

		// TODO: DO DATA SCIENCE: Maybe ignore data if we deem it "not interesting"?? i.e. if it's the same as last time
		
		// only add to queue if we've read enough times, as indicated by constants of the type READS_PER_LOG
		if (dataReadSinceLastLog >= sensorTasksReadsPerLog[ task enum ID from tasks enum ])
		{
			// send to the queue, blocking INDEFINITELY (portMAX_DELAY) until we are able to add
			// TODO: Should we wait forever???
			// NOTE: THIS COULD BE OUR OWN DESIGNED queue/stack
			xQueueSend( [this sensor's data queue], &valueToSend, portMAX_DELAY );
			
			dataReadSinceLastLog = 0;
		}
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}
*/

static void task_data_read_idle(void *pvParameters)
{
	// initialize xNextWakeTime once
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	// tracking arrays
	int reads_since_last_log[NUM_DATA_TYPES];
	int data_array_tails[NUM_DATA_TYPES];
	
	// initialize first struct
	idle_data_t *current_struct;
		
	for( ;; )
	{	
		// block for a time based on this task's globally-set frequency
		// (Note: changes to the frequency can be delayed in taking effect by as much as the past frequency...)
		vTaskDelayUntil( &xNextWakeTime, IDLE_RD_TASK_FREQ / portTICK_PERIOD_MS);
		
		if (reads_since_last_log[IR_DATA] >= idle_IR_READS_PER_LOG)
		{
			// read sensor and compile into batch
			ir_batch batch = {.timestamp = 1, .values = {1, 2, 3, 4, 5, 6}}; //read_ir_batch();
			
			// log sensor data to the appropriate array within the big struct
			current_struct->ir_data[data_array_tails[IR_DATA]] = batch; /* DATA! */
			
			// increment array tail marker and reset reads-per-log counter
			data_array_tails[IR_DATA] = data_array_tails[IR_DATA] + 1;
			reads_since_last_log[IR_DATA] = 0;
		}
		
		// once we've collected all the data we need to into the current struct, add the whole thing
		// (all data is collected once the lowest frequency sensor has just logged)
		if (reads_since_last_log[idle_LOWEST_FREQ_SENSOR] >= idle_MAX_READS_PER_LOG)
		{
			//idle_readings_equistack.add(current_struct)
			
			// reinitialize data struct
			//idle_data_t blank_struct;
			//memcpy(current_struct, blank_struct, sizeof(blank_struct));	
		}
		
		// increment reads in reads_since_last_log
		increment_all(reads_since_last_log, NUM_DATA_TYPES);		
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}

// static void task_data_read_flash(void *pvParameters);
// static void task_data_read_boot(void *pvParameters);
// static void task_data_read_low_power(void *pvParameters);

