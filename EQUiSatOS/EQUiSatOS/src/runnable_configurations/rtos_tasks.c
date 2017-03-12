/*
 * rtos_tasks.c
 *
 * Created: 9/27/2016 8:21:58 PM
 *  Author: rj16
 */ 

#include "rtos_tasks.h"

/* Individual sensor helpers for data reading tasks */
void add_ir_batch_if_ready(ir_batch *batch_list, int *data_array_tails, int *loops_since_last_log, int loops_per_log)
{
	if (loops_since_last_log[IR_DATA] >= loops_per_log)
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
		assert(batch_list[data_array_tails[IR_DATA]].values[1] == batch.values[1]); // TODO: is this task independent:
		assert(batch_list[data_array_tails[IR_DATA]].values[idle_IR_DATA_ARR_LEN - 1] == batch.values[idle_IR_DATA_ARR_LEN - 1]);
		
		// increment array tail marker and reset reads-per-log counter
		data_array_tails[IR_DATA] = data_array_tails[IR_DATA] + 1;
		loops_since_last_log[IR_DATA] = 0;
	}
}

void add_temp_batch_if_ready(temp_batch *batch_list, int *data_array_tails, int *loops_since_last_log, int loops_per_log)
{
	if (loops_since_last_log[TEMP_DATA] >= loops_per_log)
	{
		// read sensor and compile into batch
		temp_batch batch = read_temp_batch();
		
		// log sensor data to the appropriate array within the big struct
		batch_list[data_array_tails[TEMP_DATA]] = batch;
		
		// increment array tail marker and reset reads-per-log counter
		data_array_tails[TEMP_DATA] = data_array_tails[TEMP_DATA] + 1;
		loops_since_last_log[TEMP_DATA] = 0;
	}
}

void add_diode_batch_if_ready(diode_batch *batch_list, int *data_array_tails, int *loops_since_last_log, int loops_per_log)
{
	if (loops_since_last_log[DIODE_DATA] >= loops_per_log)
	{
		// read sensor and compile into batch
		diode_batch batch = read_diode_batch();
		
		// log sensor data to the appropriate array within the big struct
		batch_list[data_array_tails[DIODE_DATA]] = batch;
		
		// increment array tail marker and reset reads-per-log counter
		data_array_tails[DIODE_DATA] = data_array_tails[DIODE_DATA] + 1;
		loops_since_last_log[DIODE_DATA] = 0;
	}
}

void add_led_current_batch_if_ready(led_current_batch *batch_list, int *data_array_tails, int *loops_since_last_log, int loops_per_log)
{
	if (loops_since_last_log[LED_CUR_DATA] >= loops_per_log)
	{
		// read sensor and compile into batch
		led_current_batch batch = read_led_current_batch();
		
		// log sensor data to the appropriate array within the big struct
		batch_list[data_array_tails[LED_CUR_DATA]] = batch;
		
		// increment array tail marker and reset reads-per-log counter
		data_array_tails[LED_CUR_DATA] = data_array_tails[LED_CUR_DATA] + 1;
		loops_since_last_log[LED_CUR_DATA] = 0;
	}
}

void add_imu_batch_if_ready(imu_batch *batch_list, int *data_array_tails, int *loops_since_last_log, int loops_per_log)
{
	if (loops_since_last_log[IMU_DATA] >= loops_per_log)
	{
		// read sensor and compile into batch
		imu_batch batch = read_imu_batch();
		
		// log sensor data to the appropriate array within the big struct
		batch_list[data_array_tails[IMU_DATA]] = batch;
		
		// increment array tail marker and reset reads-per-log counter
		data_array_tails[IMU_DATA] = data_array_tails[IMU_DATA] + 1;
		loops_since_last_log[IMU_DATA] = 0;
	}
}

void add_magnetometer_batch_if_ready(magnetometer_batch *batch_list, int *data_array_tails, int *loops_since_last_log, int loops_per_log)
{
	if (loops_since_last_log[MAGNETOMETER_DATA] >= loops_per_log)
	{
		// read sensor and compile into batch
		magnetometer_batch batch = read_magnetometer_batch();
		
		// log sensor data to the appropriate array within the big struct
		batch_list[data_array_tails[MAGNETOMETER_DATA]] = batch;
		
		// increment array tail marker and reset reads-per-log counter
		data_array_tails[MAGNETOMETER_DATA] = data_array_tails[MAGNETOMETER_DATA] + 1;
		loops_since_last_log[MAGNETOMETER_DATA] = 0;
	}
}

void add_charging_batch_if_ready(charging_batch *batch_list, int *data_array_tails, int *loops_since_last_log, int loops_per_log)
{
	if (loops_since_last_log[CHARGING_DATA] >= loops_per_log)
	{
		// read sensor and compile into batch
		charging_batch batch = read_charging_batch();
		
		// log sensor data to the appropriate array within the big struct
		batch_list[data_array_tails[CHARGING_DATA]] = batch;
		
		// increment array tail marker and reset reads-per-log counter
		data_array_tails[CHARGING_DATA] = data_array_tails[CHARGING_DATA] + 1;
		loops_since_last_log[CHARGING_DATA] = 0;
	}
}

void add_radio_temp_batch_if_ready(radio_temp_batch *batch_list, int *data_array_tails, int *loops_since_last_log, int loops_per_log)
{
	if (loops_since_last_log[RADIO_TEMP_DATA] >= loops_per_log)
	{
		// read sensor and compile into batch
		radio_temp_batch batch = read_radio_temp_batch();
		
		// log sensor data to the appropriate array within the big struct
		batch_list[data_array_tails[RADIO_TEMP_DATA]] = batch;
		
		// increment array tail marker and reset reads-per-log counter
		data_array_tails[RADIO_TEMP_DATA] = data_array_tails[RADIO_TEMP_DATA] + 1;
		loops_since_last_log[RADIO_TEMP_DATA] = 0;
	}
}

void add_battery_voltages_batch_if_ready(battery_voltages_batch *batch_list, int *data_array_tails, int *loops_since_last_log, int loops_per_log)
{
	if (loops_since_last_log[BAT_VOLT_DATA] >= loops_per_log)
	{
		// read sensor and compile into batch
		battery_voltages_batch batch = read_battery_voltages_batch();
		
		// log sensor data to the appropriate array within the big struct
		batch_list[data_array_tails[BAT_VOLT_DATA]] = batch;
		
		// increment array tail marker and reset reads-per-log counter
		data_array_tails[BAT_VOLT_DATA] = data_array_tails[BAT_VOLT_DATA] + 1;
		loops_since_last_log[BAT_VOLT_DATA] = 0;
	}
}

void add_regulator_voltages_batch_if_ready(regulator_voltages_batch *batch_list, int *data_array_tails, int *loops_since_last_log, int loops_per_log)
{
	if (loops_since_last_log[REG_VOLT_DATA] >= loops_per_log)
	{
		// read sensor and compile into batch
		regulator_voltages_batch batch = read_regulator_voltages_batch();
		
		// log sensor data to the appropriate array within the big struct
		batch_list[data_array_tails[REG_VOLT_DATA]] = batch;
		
		// increment array tail marker and reset reads-per-log counter
		data_array_tails[REG_VOLT_DATA] = data_array_tails[REG_VOLT_DATA] + 1;
		loops_since_last_log[REG_VOLT_DATA] = 0;
	}
}

/* Action Tasks */
void watchdog_task(void *pvParameters)
{
	// initialize xNextWakeTime onces
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	for ( ;; )
	{
		vTaskDelayUntil( &xNextWakeTime, WATCHDOG_TASK_FREQ / portTICK_PERIOD_MS);
		
		// TODO
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}

void antenna_deploy_task(void *pvParameters)
{
	// initialize xNextWakeTime onces
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	for ( ;; )
	{
		vTaskDelayUntil( &xNextWakeTime, ANTENNA_DEPLOY_TASK_FREQ / portTICK_PERIOD_MS);
		
		// TODO
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}

void battery_charging_task(void *pvParameters)
{
	// initialize xNextWakeTime onces
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	for ( ;; )
	{
		vTaskDelayUntil( &xNextWakeTime, BATTERY_CHARGING_TASK_FREQ / portTICK_PERIOD_MS);
		
		// TODO
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}

/* Data Read Tasks */
void flash_activate_task(void *pvParameters)
{
	// initialize xNextWakeTime onces
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	for ( ;; )
	{
		vTaskDelayUntil( &xNextWakeTime, FLASH_ACTIVATE_TASK_FREQ / portTICK_PERIOD_MS);
		
		// TODO
		
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}

void transmit_task(void *pvParameters)
{
	// initialize xNextWakeTime once
	TickType_t xNextWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		// block for a time based on this task's globally-set frequency
		// (Note: changes to the frequency can be delayed in taking effect by as much as the past frequency...)
		vTaskDelayUntil( &xNextWakeTime, TRANSMIT_TASK_FREQ / portTICK_PERIOD_MS);
		
		bool validDataTransmitted = false; // we're cynical
		do
		{
			// read the next state to transmit (pop something off state queue) 
			int nextState = IDLE; // (int*) equistack_Top(last_state_read_equistack); 
		
			// based on what state we're in, compile a different message
			switch(nextState)
			{
// 				case IDLE: ; // empty statement to allow definition
// 					idle_data_t* idle_data_to_trans = (idle_data_t*) equistack_Get(idle_readings_equistack, 0);
// 					
// 					//if (idle_data_to_trans != NULL) 
// 					//{ 
// 					//	validDataTransmitted = true;
// 					//}
// 					break;
// 				case FLASH: ; // empty statement to allow definition
// 					break;
// 				case BOOT: ; // empty statement to allow definition
// 					break;
// 				case LOW_POWER: ; // empty statement to allow definition
// 					break;
// 				default:
// 					validDataTransmitted = true; // if the state equistack is empty, we have no data, so avoid looping until we get some (potentially infinitely) 				
			};
		} while (!validDataTransmitted);
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}

void current_data_task(void *pvParameters)
{
	// initialize xNextWakeTime onces
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	// tracking arrays
	// TODO: what happens if one of the data read tasks never reads
	// one of the sensors, so the value in here keeps growing?
	uint8_t loops_since_last_idle_log[NUM_DATA_TYPES]; 
	uint8_t loops_since_last_flash_log[NUM_DATA_TYPES]; 
	uint8_t loops_since_last_transmit_log[NUM_DATA_TYPES]; 
	uint8_t loops_since_last_attitude_log[NUM_DATA_TYPES]; 
												
	// NOTE: data_array_tails should be this big for all data reading tasks 
	// (this is just done for indexing consistency so that the add_*_batch_if_ready functions are universal) 
	uint8_t idle_array_tails[NUM_DATA_TYPES];
	uint8_t flash_array_tails[NUM_DATA_TYPES];
	uint8_t transmit_array_tails[NUM_DATA_TYPES];
	uint8_t attitude_array_tails[NUM_DATA_TYPES];
	
	// initialize initial structs
	idle_data_t *cur_idle_struct = (idle_data_t*) equistack_Initial_Stage(idle_readings_equistack);
	flash_data_t *cur_flash_struct = (flash_data_t*) equistack_Initial_Stage(flash_readings_equistack);
	transmit_data_t *cur_transmit_struct = (transmit_data_t*) equistack_Initial_Stage(transmit_readings_equistack);
	attitude_data_t *cur_attitude_struct = (attitude_data_t*) equistack_Initial_Stage(attitude_readings_equistack);
	cur_idle_struct->timestamp = get_current_timestamp();
	cur_flash_struct->timestamp = get_current_timestamp();
	cur_transmit_struct->timestamp = get_current_timestamp();
	cur_attitude_struct->timestamp = get_current_timestamp();
	//	assert(current_struct != NULL); // TESTING
		
	for( ;; )
	{	
		// block for a time based on a frequency, determined by whether we're in IDLE or LOW_POWER mode.
		// (Note: changes to the frequency can be delayed in taking effect by as much as the past frequency...)
		if (CurrentState == LOW_POWER) {
			vTaskDelayUntil( &xNextWakeTime, DATA_LOW_POWER_TASK_FREQ / portTICK_PERIOD_MS);
		} else {
			vTaskDelayUntil( &xNextWakeTime, DATA_TASK_FREQ / portTICK_PERIOD_MS);
		}
		
		// once we've collected all the data we need to into the current struct, add the whole thing
		// (all data is collected once some sensor is just about to log past the end of the list -> if one is, all should be)
		if (data_array_tails[IR_DATA] >= idle_IR_DATA_ARR_LEN)
		{
			// FOR TESTING
//			idle_data_t* prev_cur_struct = current_struct;
			
			// validate previous stored value in stack, getting back the next staged address we can start adding to
			current_struct = (idle_data_t*) equistack_Stage(idle_readings_equistack);
			current_struct->timestamp = get_current_timestamp();
			
			// reset data array tails so we're writing at the start // TODO: loops_since_last_log = ...; ???
			set_all(data_array_tails, NUM_DATA_TYPES, 0);
			
			// TESTING
// 			assert(prev_cur_struct != current_struct);
// 			assert(data_array_tails[0] == 0 && data_array_tails[1] == 0 && data_array_tails[2] == 0 && data_array_tails[3] == 0 && data_array_tails[4] == 0 && data_array_tails[NUM_DATA_TYPES-1] == 0);
// 			assert((idle_data_t*) equistack_Get(idle_readings_equistack, 0) == prev_cur_struct);
		}
		
		
		// TODO: DO CHECKS FOR ERRORS (TO GENERATE ERRORS) HERE
		
		// Try to log idle sensor data
		add_ir_batch_if_ready( &(current_struct->ir_data), data_array_tails, loops_since_last_log, idle_IR_LOOPS_PER_LOG);
// 		add_temp_batch_if_ready(&(current_struct->temp_data), data_array_tails, loops_since_last_log, idle_TEMP_LOOPS_PER_LOG);
// 		add_diode_batch_if_ready(&(current_struct->diode_data), data_array_tails, loops_since_last_log, idle_DIODE_LOOPS_PER_LOG);
// 		add_led_current_batch_if_ready(&(current_struct->led_current_data), data_array_tails, loops_since_last_log, idle_LED_CURRENT_LOOPS_PER_LOG);
// 		add_gyro_batch_if_ready(&(current_struct->gyro_data), data_array_tails, loops_since_last_log, idle_GYRO_LOOPS_PER_LOG);
// 		add_magnetometer_batch_if_ready(&(current_struct->magnetometer_data), data_array_tails, loops_since_last_log, idle_MAGNETOMETER_LOOPS_PER_LOG);
// 		add_charging_batch_if_ready(&(current_struct->charging_data), data_array_tails, loops_since_last_log, idle_CHARGING_DATA_LOOPS_PER_LOG);
// 		add_radio_temp_batch_if_ready(&(current_struct->radio_temp_data), data_array_tails, loops_since_last_log, idle_RADIO_TEMP_LOOPS_PER_LOG);
// 		add_battery_voltages_batch_if_ready(&(current_struct->battery_voltages_data), data_array_tails, loops_since_last_log, idle_BAT_VOLTAGE_LOOPS_PER_LOG);
// 		add_regulator_voltages_batch_if_ready(&(current_struct->regulator_voltages_data), data_array_tails, loops_since_last_log, idle_REG_VOLTAGE_LOOPS_PER_LOG);
		
		// only log additional data if we're not in low power
		if (CurrentState != LOW_POWER) {
			switch (CurrentLoggingState) {
				case FLASH_LOG:
	
			}
		}
		// FOR TESTING
// 		int ir_reads_since = loops_since_last_log[0];
// 		int last_sens_reads_since = loops_since_last_log[NUM_DATA_TYPES-1];
		
		// increment reads in loops_since_last_log
		increment_all(loops_since_last_log, NUM_DATA_TYPES);		
		
		// TESTING
// 		assert(loops_since_last_log[0] == ir_reads_since + 1);
// 		assert(loops_since_last_log[NUM_DATA_TYPES-1] == last_sens_reads_since + 1);
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}

void attitude_data_task(void *pvParameters)
{
	// initialize xNextWakeTime onces
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	// current_data_task for extensive comments / testing
		
	// tracking arrays
	uint8_t loops_since_last_log[NUM_DATA_TYPES];
	uint8_t data_array_tails[NUM_DATA_TYPES];
	
	attitude_data_t *current_struct = (idle_data_t*) equistack_Initial_Stage(attitude_readings_equistack);
	
	for ( ;; )
	{
		vTaskDelayUntil( &xNextWakeTime, ATTITUDE_DATA_TASK_FREQ / portTICK_PERIOD_MS);
		
		// update current_struct if necessary
		if (data_array_tails[IR_DATA] >= attitute_data_IR_DATA_ARR_LEN)
		{
			// validate previous stored value in stack, getting back the next staged address we can start adding to
			current_struct = (attitude_data_t*) equistack_Stage(attitude_readings_equistack);
			current_struct->timestamp = get_current_timestamp();
			
			// TODO: log state read
			//equistack_Stage(last_reading_type_equistack, ATTITUDE_DATA);
			
			// reset data array tails so we're writing at the start // TODO: loops_since_last_log = ...; ???
			set_all(data_array_tails, NUM_DATA_TYPES, 0);
		}
		
		// TODO: DO CHECKS FOR ERRORS (TO GENERATE ERRORS) HERE
		
		
		// see if each sensor is ready to add a batch, and do so if we need to
		add_ir_batch_if_ready( &(current_struct->ir_data), data_array_tails, loops_since_last_log, attitute_data_IR_LOOPS_PER_LOG);
		add_diode_batch_if_ready(&(current_struct->diode_data), data_array_tails, loops_since_last_log, attitute_data_DIODE_LOOPS_PER_LOG);
		add_imu_batch_if_ready(&(current_struct->imu_data), data_array_tails, loops_since_last_log, attitute_data_IMU_LOOPS_PER_LOG);
		add_magnetometer_batch_if_ready(&(current_struct->magnetometer_data), data_array_tails, loops_since_last_log, attitute_data_MAGNETOMETER_LOOPS_PER_LOG);
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}

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