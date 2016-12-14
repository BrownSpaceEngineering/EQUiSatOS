/*
 * rtos_tasks.c
 *
 * Created: 9/27/2016 8:21:58 PM
 *  Author: rj16
 */ 

#include "rtos_tasks.h"

/* Individual sensor helpers for data reading tasks */
void add_ir_batch_if_ready(ir_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log)
{
	if (reads_since_last_log[IR_DATA] >= reads_per_log)
	{
		// read sensor and compile into batch
		ir_batch batch = read_ir_batch();
		
		// log sensor data to the appropriate array within the big struct
		batch_list[data_array_tails[IR_DATA]] = batch;
		
		// TESTING
		assert(batch_list[data_array_tails[IR_DATA]].values[0] == 0 &&
			   batch_list[data_array_tails[IR_DATA]].values[1] == 1 && 
			   batch_list[data_array_tails[IR_DATA]].values[2] == 2 && 
			   batch_list[data_array_tails[IR_DATA]].values[3] == 3 &&
			   batch_list[data_array_tails[IR_DATA]].values[4] == 4); // remove once implemented
		assert(batch_list[data_array_tails[IR_DATA]].values[0] == batch.values[0]);
		assert(batch_list[data_array_tails[IR_DATA]].values[1] == batch.values[1]);
		assert(batch_list[data_array_tails[IR_DATA]].values[idle_MAX_READS_PER_LOG / idle_IR_READS_PER_LOG - 1] == batch.values[idle_MAX_READS_PER_LOG / idle_IR_READS_PER_LOG - 1]);
		
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
				case IDLE: ; // empty statement to allow definition
					idle_data_t* idle_data_to_trans = idle_Stack_Get(idle_readings_equistack, 0);
					
					//if (idle_data_to_trans != NULL) 
					//{ 
					//	validDataTransmitted = true;
					//}
					break;
				case FLASH: ; // empty statement to allow definition
					break;
				case BOOT: ; // empty statement to allow definition
					break;
				case LOW_POWER: ; // empty statement to allow definition
					break;
				default:
					validDataTransmitted = true; // if the state equistack is empty, we have no data, so avoid looping until we get some (potentially infinitely) 				
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
	idle_data_t *current_struct = idle_Stack_Initial_Stage(idle_readings_equistack);
	current_struct->timestamp = get_current_timestamp();
	
	// TESTING
	assert(current_struct != NULL);
		
	for( ;; )
	{	
		// block for a time based on this task's globally-set frequency
		// (Note: changes to the frequency can be delayed in taking effect by as much as the past frequency...)
		vTaskDelayUntil( &xNextWakeTime, IDLE_RD_TASK_FREQ / portTICK_PERIOD_MS);
		
		// once we've collected all the data we need to into the current struct, add the whole thing
		// (all data is collected once some sensor is just about to log past the end of the list -> if one is, all should be)
		if (data_array_tails[IR_DATA] >= idle_MAX_READS_PER_LOG / idle_IR_READS_PER_LOG)
		{
			// FOR TESTING
			idle_data_t* prev_cur_struct = current_struct;
			
			// validate previous stored value in stack, getting back the next staged address we can start adding to
			current_struct = idle_Stack_Stage(idle_readings_equistack);
			current_struct->timestamp = get_current_timestamp();
			
			// log state read
			//num_Stack_Push(last_state_read_equistack, IDLE);
			
			// reset data array tails so we're writing at the start // TODO: reads_since_last_log = ...; ???
			set_all(data_array_tails, NUM_DATA_TYPES, 0);
			
			// TESTING
			assert(prev_cur_struct != current_struct);
			assert(data_array_tails[0] == 0 && data_array_tails[1] == 0 && data_array_tails[2] == 0 && data_array_tails[3] == 0 && data_array_tails[4] == 0 && data_array_tails[NUM_DATA_TYPES-1] == 0);
			assert(idle_Stack_Get(idle_readings_equistack, 0) == prev_cur_struct);
		}
		
		
		// TODO: DO CHECKS FOR ERRORS (TO GENERATE ERRORS) HERE
		
		
		// see if each sensor is ready to add a batch, and do so if we need to
		add_ir_batch_if_ready( &(current_struct->ir_data), data_array_tails, reads_since_last_log, idle_IR_READS_PER_LOG);
// 		add_temp_batch_if_ready(&(current_struct->temp_data), data_array_tails, reads_since_last_log, idle_TEMP_READS_PER_LOG);
// 		add_diode_batch_if_ready(&(current_struct->diode_data), data_array_tails, reads_since_last_log, idle_DIODE_READS_PER_LOG);
// 		add_led_current_batch_if_ready(&(current_struct->led_current_data), data_array_tails, reads_since_last_log, idle_LED_CURRENT_READS_PER_LOG);
// 		add_gyro_batch_if_ready(&(current_struct->gyro_data), data_array_tails, reads_since_last_log, idle_GYRO_READS_PER_LOG);
// 		add_magnetometer_batch_if_ready(&(current_struct->magnetometer_data), data_array_tails, reads_since_last_log, idle_MAGNETOMETER_READS_PER_LOG);
// 		add_charging_batch_if_ready(&(current_struct->charging_data), data_array_tails, reads_since_last_log, idle_CHARGING_DATA_READS_PER_LOG);
// 		add_radio_temp_batch_if_ready(&(current_struct->radio_temp_data), data_array_tails, reads_since_last_log, idle_RADIO_TEMP_READS_PER_LOG);
// 		add_battery_voltages_batch_if_ready(&(current_struct->battery_voltages_data), data_array_tails, reads_since_last_log, idle_BAT_VOLTAGE_READS_PER_LOG);
// 		add_regulator_voltages_batch_if_ready(&(current_struct->regulator_voltages_data), data_array_tails, reads_since_last_log, idle_REG_VOLTAGE_READS_PER_LOG);
		
		// FOR TESTING
		int ir_reads_since = reads_since_last_log[0];
		int last_sens_reads_since = reads_since_last_log[NUM_DATA_TYPES-1];
		
		// increment reads in reads_since_last_log
		increment_all(reads_since_last_log, NUM_DATA_TYPES);		
		
		// TESTING
		assert(reads_since_last_log[0] == ir_reads_since + 1);
		assert(reads_since_last_log[NUM_DATA_TYPES-1] == last_sens_reads_since + 1);
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}

void task_data_read_flash(void *pvParameters)
{
	// initialize xNextWakeTime onces
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	// tracking arrays
	int reads_since_last_log[NUM_DATA_TYPES]; // TODO: what happens if one of the data read tasks never reads
	// one of the sensors, so the value in here keeps growing?
	int data_array_tails[NUM_DATA_TYPES];
	
	// initialize first struct
	flash_data_t *current_struct = flash_Stack_Stage(flash_readings_equistack); 
	current_struct->timestamp = get_current_timestamp();
	
	for( ;; )
	{	
		// block for a time based on this task's globally-set frequency
		// (Note: changes to the frequency can be delayed in taking effect by as much as the past frequency...)
		vTaskDelayUntil( &xNextWakeTime, FLASH_RD_TASK_FREQ / portTICK_PERIOD_MS);
		
		// once we've collected all the data we need to into the current struct, add the whole thing
		// (all data is collected once some sensor is just about to log past the end of the list -> if one is, all should be)
		if (data_array_tails[IR_DATA] >= flash_MAX_READS_PER_LOG / flash_IR_READS_PER_LOG)
		{
			// validate previous stored value in stack, getting back the next staged address we can start adding to
			current_struct = flash_Stack_Stage(flash_readings_equistack);
			current_struct->timestamp = get_current_timestamp();
			
			// log state read
			//num_Stack_Push(last_state_read_equistack, IDLE);
			
			// reset data array tails so we're writing at the start
			set_all(data_array_tails, NUM_DATA_TYPES, 0);			
		}
		
		// see if each sensor is ready to add a batch, and do so if we need to
		add_ir_batch_if_ready( &(current_struct->ir_data), data_array_tails, reads_since_last_log, flash_IR_READS_PER_LOG);
// 		add_temp_batch_if_ready(&(current_struct->temp_data), data_array_tails, reads_since_last_log, flash_TEMP_READS_PER_LOG);
// 		add_diode_batch_if_ready(&(current_struct->diode_data), data_array_tails, reads_since_last_log, flash_DIODE_READS_PER_LOG);
// 		add_led_current_batch_if_ready(&(current_struct->led_current_data), data_array_tails, reads_since_last_log, flash_LED_CURRENT_READS_PER_LOG);
// 		add_gyro_batch_if_ready(&(current_struct->gyro_data), data_array_tails, reads_since_last_log, flash_GYRO_READS_PER_LOG);
// 		add_magnetometer_batch_if_ready(&(current_struct->magnetometer_data), data_array_tails, reads_since_last_log, flash_MAGNETOMETER_READS_PER_LOG);
// 		add_charging_batch_if_ready(&(current_struct->charging_data), data_array_tails, reads_since_last_log, flash_CHARGING_DATA_READS_PER_LOG);
// 		add_radio_temp_batch_if_ready(&(current_struct->radio_temp_data), data_array_tails, reads_since_last_log, flash_RADIO_TEMP_READS_PER_LOG);
// 		add_battery_voltages_batch_if_ready(&(current_struct->battery_voltages_data), data_array_tails, reads_since_last_log, flash_BAT_VOLTAGE_READS_PER_LOG);
// 		add_regulator_voltages_batch_if_ready(&(current_struct->regulator_voltages_data), data_array_tails, reads_since_last_log, flash_REG_VOLTAGE_READS_PER_LOG);
		
		// increment reads in reads_since_last_log
		increment_all(reads_since_last_log, NUM_DATA_TYPES);
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}
// static void task_data_read_boot(void *pvParameters);
// static void task_data_read_low_power(void *pvParameters);


/* Helper Functions */
uint32_t get_current_timestamp()
{
	// TODO: get a more accurate and persistent timestamp (relative to an alive message)
	return xTaskGetTickCount(); // represents the ms from vstartscheduler
}

void increment_all(int* int_arr, int length)
{
	for(int i = 0; i < length; i++)
	{
		int_arr[i] = int_arr[i] + 1;
	}
}

void set_all(int* int_arr, int length, int value)
{
	for(int i = 0; i < length; i++)
	{
		int_arr[i] = value;
	}
}