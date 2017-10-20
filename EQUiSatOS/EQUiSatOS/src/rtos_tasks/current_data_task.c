/*
 * current_data_task.c
 *
 * Created: 9/21/2017 20:42:19
 *  Author: mcken
 */ 

#include "rtos_tasks.h"

/* Data Read Tasks */
void current_data_task(void *pvParameters)
{
	// initialize xNextWakeTime onces
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	// tracking arrays
	uint8_t loops_since_last_log[NUM_DATA_TYPES]; // TODO: what happens if one of the data read tasks never reads
												// one of the sensors, so the value in here keeps growing?
												
	// NOTE: data_array_tails should be this big for all data reading tasks 
	// (this is just done for indexing consistency so that the add_*_batch_if_ready functions are universal) 
	uint8_t data_array_tails[NUM_DATA_TYPES];
	
	// initialize first struct
	idle_data_t *current_struct = (idle_data_t*) equistack_Initial_Stage(&idle_readings_equistack);
	assert(current_struct != NULL); // TESTING
	current_struct->timestamp = get_current_timestamp();
		
	for( ;; )
	{
		// block for a time based on a frequency, determined by whether we're in IDLE or LOW_POWER mode.
		// (Note: changes to the frequency can be delayed in taking effect by as much as the past frequency...)
		if (CurrentState == LOW_POWER) {
			vTaskDelayUntil( &xNextWakeTime, CURRENT_DATA_LOW_POWER_TASK_FREQ / portTICK_PERIOD_MS);
		} else {
			vTaskDelayUntil( &xNextWakeTime, CURRENT_DATA_TASK_FREQ / portTICK_PERIOD_MS);
		}

		// once we've collected all the data we need to into the current struct, add the whole thing
		// (all data is collected once some sensor is just about to log past the end of the list 
		//  (if one is, all should be according to their LOOPS_PER_LOG -> they're all the same freq))
		if (check_if_suspended_and_update(CURRENT_DATA_TASK) || data_array_tails[IR_DATA] >= idle_BAT_TEMP_DATA_ARR_LEN)
		{
			
			// FOR TESTING
			idle_data_t* prev_cur_struct = current_struct;
			
			// validate previous stored value in stack, getting back the next staged address we can start adding to
			current_struct = (idle_data_t*) equistack_Stage(&idle_readings_equistack);
			current_struct->timestamp = get_current_timestamp();
			
			// reset data array tails so we're writing at the start // TODO: loops_since_last_log = ...; ???
			set_all(data_array_tails, NUM_DATA_TYPES, 0);
			
			// TESTING
			assert(prev_cur_struct != current_struct);
			assert(data_array_tails[0] == 0 && data_array_tails[1] == 0 && data_array_tails[2] == 0 && data_array_tails[3] == 0 && data_array_tails[4] == 0 && data_array_tails[NUM_DATA_TYPES-1] == 0);
			assert((idle_data_t*) equistack_Get(&idle_readings_equistack, 0) == prev_cur_struct);
		}
		
		
		// TODO: DO CHECKS FOR ERRORS (TO GENERATE ERRORS) HERE
		
		
		// see if each sensor is ready to add a batch, and do so if we need to
		/**
		 * Helpful regex: (\w*)_DATA, -> 
		 * if (loops_since_last_log[$1_DATA] >= idle_$1_LOOPS_PER_LOG) { \n current_struct->$1_data[data_array_tails[$1_DATA]] = read_$1_batch(); \n increment_data_type($1_DATA, data_array_tails, loops_since_last_log);\n}
		 */
		if (loops_since_last_log[LION_VOLTS_DATA] >= idle_LION_VOLTS_LOOPS_PER_LOG) { 
		 read_lion_volts_batch(current_struct->lion_volts_data[data_array_tails[LION_VOLTS_DATA]]); 
		 increment_data_type(LION_VOLTS_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[LION_CURRENT_DATA] >= idle_LION_CURRENT_LOOPS_PER_LOG) { 
		 read_lion_current_batch(current_struct->lion_current_data[data_array_tails[LION_CURRENT_DATA]]); 
		 increment_data_type(LION_CURRENT_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[BAT_CHARGE_VOLTS_DATA] >= idle_BAT_CHARGE_VOLTS_LOOPS_PER_LOG) {
			read_bat_charge_volts_batch(current_struct->bat_charge_volts_data[data_array_tails[BAT_CHARGE_VOLTS_DATA]]);
			increment_data_type(BAT_CHARGE_VOLTS_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[BAT_CHARGE_DIG_SIGS_DATA] >= idle_BAT_CHARGE_DIG_SIGS_LOOPS_PER_LOG) {
			read_bat_charge_dig_sigs_batch(current_struct->bat_charge_dig_sigs_data[data_array_tails[BAT_CHARGE_DIG_SIGS_DATA]]);
			increment_data_type(BAT_CHARGE_DIG_SIGS_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[DIGITAL_OUT_DATA] >= idle_DIGITAL_OUT_LOOPS_PER_LOG) {
			read_digital_out_batch(current_struct->digital_out_data[data_array_tails[DIGITAL_OUT_DATA]]);
			increment_data_type(DIGITAL_OUT_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[BAT_TEMP_DATA] >= idle_BAT_TEMP_LOOPS_PER_LOG) { 
		 read_bat_temp_batch(current_struct->bat_temp_data[data_array_tails[BAT_TEMP_DATA]]); 
		 increment_data_type(BAT_TEMP_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[RADIO_VOLTS_DATA] >= idle_RADIO_VOLTS_LOOPS_PER_LOG) { 
		 read_radio_volts_batch(current_struct->radio_volts_data[data_array_tails[RADIO_VOLTS_DATA]]); 
		 increment_data_type(RADIO_VOLTS_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[RADIO_TEMP_DATA] >= idle_RADIO_TEMP_LOOPS_PER_LOG) {
			read_radio_temp_batch(current_struct->radio_temp_data[data_array_tails[RADIO_TEMP_DATA]]);
			increment_data_type(RADIO_TEMP_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[IMU_TEMP_DATA] >= idle_IMU_TEMP_LOOPS_PER_LOG) {
			read_ir_temps_batch(current_struct->imu_temp_data[data_array_tails[IMU_TEMP_DATA]]);
			increment_data_type(IMU_TEMP_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[IR_TEMPS_DATA] >= idle_IR_TEMPS_LOOPS_PER_LOG) {
			read_ir_temps_batch(current_struct->ir_temps_data[data_array_tails[IR_TEMPS_DATA]]);
			increment_data_type(IR_TEMPS_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[RAIL_3V_DATA] >= idle_RAIL_3V_DATA_ARR_LEN) {
			read_ir_batch(current_struct->rail_3v_data[data_array_tails[RAIL_3V_DATA]]);
			increment_data_type(RAIL_3V_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[RAIL_5V_DATA] >= idle_RAIL_5V_DATA_ARR_LEN) {
			read_ir_batch(current_struct->rail_5v_data[data_array_tails[RAIL_5V_DATA]]);
			increment_data_type(RAIL_5V_DATA, data_array_tails, loops_since_last_log);
		}
		
		// FOR TESTING
		uint8_t ir_reads_since = loops_since_last_log[0];
		uint8_t last_sens_reads_since = loops_since_last_log[NUM_DATA_TYPES-1];
		
		// increment reads in loops_since_last_log
		increment_all(loops_since_last_log, NUM_DATA_TYPES);		
		
		// TESTING
		assert(loops_since_last_log[0] == ir_reads_since + 1);
		assert(loops_since_last_log[NUM_DATA_TYPES-1] == last_sens_reads_since + 1);
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}