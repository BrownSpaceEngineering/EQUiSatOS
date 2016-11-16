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
		ir_batch batch = read_ir_batch();
			
		// log sensor data to the appropriate array within the big struct
		batch_list[data_array_tails[IR_DATA]] = batch;
			
		// increment array tail marker and reset reads-per-log counter
		data_array_tails[IR_DATA] = data_array_tails[IR_DATA] + 1;
		reads_since_last_log[IR_DATA] = 0;
	}
}

void add_temp_batch_if_ready(temp_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log)
{
	if (reads_since_last_log[TEMP_DATA] >= reads_per_log)
	{
		// read sensor and compile into batch
		temp_batch batch = read_temp_batch();
		
		// log sensor data to the appropriate array within the big struct
		batch_list[data_array_tails[TEMP_DATA]] = batch;
		
		// increment array tail marker and reset reads-per-log counter
		data_array_tails[TEMP_DATA] = data_array_tails[TEMP_DATA] + 1;
		reads_since_last_log[TEMP_DATA] = 0;
	}
}
void add_diode_batch_if_ready(diode_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log)
{
	if (reads_since_last_log[DIODE_DATA] >= reads_per_log)
	{
		// read sensor and compile into batch
		diode_batch batch = read_diode_batch();
		
		// log sensor data to the appropriate array within the big struct
		batch_list[data_array_tails[DIODE_DATA]] = batch;
		
		// increment array tail marker and reset reads-per-log counter
		data_array_tails[DIODE_DATA] = data_array_tails[DIODE_DATA] + 1;
		reads_since_last_log[DIODE_DATA] = 0;
	}
}

void add_led_current_batch_if_ready(led_current_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log)
{
	if (reads_since_last_log[LED_CUR_DATA] >= reads_per_log)
	{
		// read sensor and compile into batch
		led_current_batch batch = read_led_current_batch();
		
		// log sensor data to the appropriate array within the big struct
		batch_list[data_array_tails[LED_CUR_DATA]] = batch;
		
		// increment array tail marker and reset reads-per-log counter
		data_array_tails[LED_CUR_DATA] = data_array_tails[LED_CUR_DATA] + 1;
		reads_since_last_log[LED_CUR_DATA] = 0;
	}
}

void add_gyro_batch_if_ready(gyro_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log)
{
	if (reads_since_last_log[GYRO_DATA] >= reads_per_log)
	{
		// read sensor and compile into batch
		gyro_batch batch = read_gyro_batch();
		
		// log sensor data to the appropriate array within the big struct
		batch_list[data_array_tails[GYRO_DATA]] = batch;
		
		// increment array tail marker and reset reads-per-log counter
		data_array_tails[GYRO_DATA] = data_array_tails[GYRO_DATA] + 1;
		reads_since_last_log[GYRO_DATA] = 0;
	}
}

void add_magnetometer_batch_if_ready(magnetometer_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log)
{
	if (reads_since_last_log[MAGNETOMETER_DATA] >= reads_per_log)
	{
		// read sensor and compile into batch
		magnetometer_batch batch = read_magnetometer_batch();
		
		// log sensor data to the appropriate array within the big struct
		batch_list[data_array_tails[MAGNETOMETER_DATA]] = batch;
		
		// increment array tail marker and reset reads-per-log counter
		data_array_tails[MAGNETOMETER_DATA] = data_array_tails[MAGNETOMETER_DATA] + 1;
		reads_since_last_log[MAGNETOMETER_DATA] = 0;
	}
}

void add_charging_batch_if_ready(charging_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log)
{
	if (reads_since_last_log[CHARGING_DATA] >= reads_per_log)
	{
		// read sensor and compile into batch
		charging_batch batch = read_charging_batch();
		
		// log sensor data to the appropriate array within the big struct
		batch_list[data_array_tails[CHARGING_DATA]] = batch;
		
		// increment array tail marker and reset reads-per-log counter
		data_array_tails[CHARGING_DATA] = data_array_tails[CHARGING_DATA] + 1;
		reads_since_last_log[CHARGING_DATA] = 0;
	}
}

void add_radio_temp_batch_if_ready(radio_temp_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log)
{
	if (reads_since_last_log[RADIO_TEMP_DATA] >= reads_per_log)
	{
		// read sensor and compile into batch
		radio_temp_batch batch = read_radio_temp_batch();
		
		// log sensor data to the appropriate array within the big struct
		batch_list[data_array_tails[RADIO_TEMP_DATA]] = batch;
		
		// increment array tail marker and reset reads-per-log counter
		data_array_tails[RADIO_TEMP_DATA] = data_array_tails[RADIO_TEMP_DATA] + 1;
		reads_since_last_log[RADIO_TEMP_DATA] = 0;
	}
}

void add_battery_voltages_batch_if_ready(battery_voltages_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log)
{
	if (reads_since_last_log[BAT_VOLT_DATA] >= reads_per_log)
	{
		// read sensor and compile into batch
		battery_voltages_batch batch = read_battery_voltages_batch();
		
		// log sensor data to the appropriate array within the big struct
		batch_list[data_array_tails[BAT_VOLT_DATA]] = batch;
		
		// increment array tail marker and reset reads-per-log counter
		data_array_tails[BAT_VOLT_DATA] = data_array_tails[BAT_VOLT_DATA] + 1;
		reads_since_last_log[BAT_VOLT_DATA] = 0;
	}
}

void add_regulator_voltages_batch_if_ready(regulator_voltages_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log)
{
	if (reads_since_last_log[REG_VOLT_DATA] >= reads_per_log)
	{
		// read sensor and compile into batch
		regulator_voltages_batch batch = read_regulator_voltages_batch();
		
		// log sensor data to the appropriate array within the big struct
		batch_list[data_array_tails[REG_VOLT_DATA]] = batch;
		
		// increment array tail marker and reset reads-per-log counter
		data_array_tails[REG_VOLT_DATA] = data_array_tails[REG_VOLT_DATA] + 1;
		reads_since_last_log[REG_VOLT_DATA] = 0;
	}
}

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
		
		bool validDataTransmitted = false; // we're cynical
		do
		{
			// read the next state to transmit (pop something off state queue) 
			int nextState = IDLE; // num_Stack_Top(last_state_read_equistack); 
		
			// based on what state we're in, compile a different message
			switch(nextState)
			{
				case IDLE: 
					//idle_data_t* idle_data_to_trans = idle_Stack_Top(idle_readings_equistack);
					
					//if (idle_data_to_trans != NULL) 
					//{ 
					//	validDataTransmitted = true;
					//}
					break;
				case FLASH:
					break;
				case BOOT:
					break;
				case LOW_POWER: 
					break;
				default:
					validDataTransmitted = true; // if the state equistack is empty, we have no data, so avoid looping (potentially infinitely) until we get some				
			};
		} while (!validDataTransmitted);
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
			// push to list of transmissions, including adding a corresponding state
			idle_Stack_Push(idle_readings_equistack, current_struct);
			//num_Stack_Push(last_state_read_equistack, IDLE);
			
			// reinitialize data struct
			current_struct = pvPortMalloc(sizeof(idle_data_t)); // TODO: Maybe only malloc at the beginning and just store this somewhere in a massive array
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
