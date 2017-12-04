/*
 * antenna_data_task.c
 *
 * Created: 9/21/2017 20:44:50
 *  Author: mcken
 */ 

#include "rtos_tasks.h"

void attitude_data_task(void *pvParameters)
{
	// initialize xNextWakeTime onces
	TickType_t prev_wake_time = xTaskGetTickCount();
	
	// current_data_task for extensive comments / testing
	
	// tracking arrays
	uint8_t loops_since_last_log[NUM_DATA_TYPES];
	uint8_t data_array_tails[NUM_DATA_TYPES];
	
	attitude_data_t *current_struct = (attitude_data_t*) equistack_Initial_Stage(&attitude_readings_equistack);
	
	init_task_state(ATTITUDE_DATA_TASK); // suspend or run on boot
	
	for ( ;; )
	{
		vTaskDelayUntil( &prev_wake_time, ATTITUDE_DATA_TASK_FREQ / portTICK_PERIOD_MS);
		
		// report to watchdog
		report_task_running(ATTITUDE_DATA_TASK);
		
		// update current_struct if necessary
		if (check_if_suspended_and_update(ATTITUDE_DATA_TASK) || data_array_tails[IR_DATA] >= attitude_IR_DATA_ARR_LEN)
		{
			// FOR TESTING
			attitude_data_t* prev_cur_struct = current_struct;
			
			// validate previous stored value in stack, getting back the next staged address we can start adding to
			current_struct = (attitude_data_t*) equistack_Stage(&attitude_readings_equistack);
			current_struct->timestamp = get_current_timestamp();
			
			// reset data array tails so we're writing at the start // TODO: loops_since_last_log = ...; ???
			set_all_uint8(data_array_tails, NUM_DATA_TYPES, 0);
			
// 			// TESTING
// 			assert(prev_cur_struct != current_struct);
// 			assert(data_array_tails[0] == 0 && data_array_tails[1] == 0 && data_array_tails[2] == 0 && data_array_tails[3] == 0 && data_array_tails[4] == 0 && data_array_tails[NUM_DATA_TYPES-1] == 0);
// 			assert((idle_data_t*) equistack_Get(&idle_readings_equistack, 0) == prev_cur_struct);
		}
		
		// TODO: DO CHECKS FOR ERRORS (TO GENERATE ERRORS) HERE
		
		
		// see if each sensor is ready to add a batch, and do so if we need to
		if (loops_since_last_log[IR_DATA] >= attitude_IR_LOOPS_PER_LOG) {
			read_ir_object_temps_batch(current_struct->ir_obj_temps_data[data_array_tails[IR_DATA]]);
			increment_data_type(IR_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[DIODE_DATA] >= attitude_DIODE_LOOPS_PER_LOG) {
			read_pdiode_batch(current_struct->pdiode_data[data_array_tails[DIODE_DATA]]);
			increment_data_type(DIODE_DATA, data_array_tails, loops_since_last_log);
		}
		// These should only ever happen at the same time
		// i.e. assert(attitude_ACCELEROMETER_LOOPS_PER_LOG == attitude_GYRO_LOOPS_PER_LOG)
		if (loops_since_last_log[ACCELEROMETER_DATA] >= attitude_ACCELEROMETER_LOOPS_PER_LOG && 
			loops_since_last_log[GYRO_DATA] >= attitude_GYRO_LOOPS_PER_LOG) {
			read_accel_batch(current_struct->accelerometer_data[data_array_tails[ACCELEROMETER_DATA]]);
			read_gyro_batch(current_struct->gyro_data[data_array_tails[GYRO_DATA]]);
			increment_data_type(ACCELEROMETER_DATA, data_array_tails, loops_since_last_log);
			increment_data_type(GYRO_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[MAGNETOMETER_DATA] >= attitude_MAGNETOMETER_LOOPS_PER_LOG) {
			read_magnetometer_batch(current_struct->magnetometer_data[data_array_tails[MAGNETOMETER_DATA]]);
			increment_data_type(MAGNETOMETER_DATA, data_array_tails, loops_since_last_log);
		}
		
		/*
		// FOR TESTING
		uint8_t ir_reads_since = loops_since_last_log[0];
		uint8_t last_sens_reads_since = loops_since_last_log[NUM_DATA_TYPES-1];
			
		// increment reads in loops_since_last_log
		increment_all(data_array_tails, NUM_DATA_TYPES);
			
		// TESTING
		assert(loops_since_last_log[0] == ir_reads_since + 1);
		assert(loops_since_last_log[NUM_DATA_TYPES-1] == last_sens_reads_since + 1);
		*/
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}