/*
 * rtos_tasks.c
 *
 * Created: 9/27/2016 8:21:58 PM
 *  Author: rj16
 */ 

#include "rtos_tasks.h"

/* Helper Functions */
void increment_all(int* int_arr, int length)
{
	for(int i = 0; i < length; i++)
	{
		int_arr[i] = int_arr[i] + 1;
	}
}

/* Individual sensor helpers for data reading tasks */
void add_ir_batch_if_ready(ir_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log)
{
	if (reads_since_last_log[IR_DATA] >= reads_per_log)
	{
		// read sensor and compile into batch
		ir_batch batch = {.timestamp = 42, .values = {1, 2, 3, 4, 5, 6}}; //read_ir_batch();
			
		// log sensor data to the appropriate array within the big struct
		batch_list[data_array_tails[IR_DATA]] = batch;
			
		// increment array tail marker and reset reads-per-log counter
		data_array_tails[IR_DATA] = data_array_tails[IR_DATA] + 1;
		reads_since_last_log[IR_DATA] = 0;
	}
}

void add_diode_batch_if_ready(diode_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);
void add_led_current_batch_if_ready(led_current_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);
void add_gyro_batch_if_ready(gyro_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);
void add_magnetometer_batch_if_ready(magnetometer_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);
void add_charging_batch_if_ready(charging_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);
void add_radio_temp_batch_if_ready(radio_temp_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);
void add_battery_voltages_batch_if_ready(battery_voltages_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);
void add_regulator_voltages_batch_if_ready(regulator_voltages_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);

/* Action Tasks */
void task_radio_transmit(void *pvParameters)
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

/* Data Read Tasks */
void task_data_read_idle(void *pvParameters)
{
	// initialize xNextWakeTime onces
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	// tracking arrays
	int reads_since_last_log[NUM_DATA_TYPES]; // TODO: what happens if one of the data read tasks never reads
												// one of the sensors, so the value in here keeps growing?
	int data_array_tails[NUM_DATA_TYPES];
	
	// initialize first struct
	idle_data_t *current_struct = pvPortMalloc(sizeof(idle_data_t)); 
		
	for( ;; )
	{	
		// block for a time based on this task's globally-set frequency
		// (Note: changes to the frequency can be delayed in taking effect by as much as the past frequency...)
		vTaskDelayUntil( &xNextWakeTime, IDLE_RD_TASK_FREQ / portTICK_PERIOD_MS);
		
		// once we've collected all the data we need to into the current struct, add the whole thing
		// (all data is collected once some sensor is just about to log past the end of the list -> if one is, all should be)
		if (data_array_tails[IR_DATA] >= idle_MAX_READS_PER_LOG / idle_IR_READS_PER_LOG)
		{
			idle_Stack_Push(idle_readings_equistack, current_struct);
			
			// reinitialize data struct
			current_struct = pvPortMalloc(sizeof(idle_data_t));
		}
		
		// see if each sensor is ready to add a batch, and do so if we need to
		add_ir_batch_if_ready( &(current_struct->ir_data), &data_array_tails, &reads_since_last_log, idle_IR_READS_PER_LOG);
		
		// increment reads in reads_since_last_log
		increment_all(reads_since_last_log, NUM_DATA_TYPES);		
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}

// static void task_data_read_flash(void *pvParameters);
// static void task_data_read_boot(void *pvParameters);
// static void task_data_read_low_power(void *pvParameters);




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