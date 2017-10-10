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
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	// current_data_task for extensive comments / testing
	
	// tracking arrays
	uint8_t loops_since_last_log[NUM_DATA_TYPES];
	uint8_t data_array_tails[NUM_DATA_TYPES];
	
	attitude_data_t *current_struct = (attitude_data_t*) equistack_Initial_Stage(&attitude_readings_equistack);
	
	for ( ;; )
	{
		vTaskDelayUntil( &xNextWakeTime, ATTITUDE_DATA_TASK_FREQ / portTICK_PERIOD_MS);
		
		// update current_struct if necessary
		if (check_if_suspended_and_update(ATTITUDE_DATA_TASK) || data_array_tails[IR_DATA] >= attitude_IR_DATA_ARR_LEN)
		{
			// validate previous stored value in stack, getting back the next staged address we can start adding to
			current_struct = (attitude_data_t*) equistack_Stage(&attitude_readings_equistack);
			current_struct->timestamp = get_current_timestamp();
			
			// log state read
			msg_data_type_t last_reading = ATTITUDE_DATA; // need a stack pointer to memcpy from
			equistack_Push(&last_reading_type_equistack, &last_reading);
			
			// reset data array tails so we're writing at the start // TODO: loops_since_last_log = ...; ???
			set_all(data_array_tails, NUM_DATA_TYPES, 0);
		}
		
		// TODO: DO CHECKS FOR ERRORS (TO GENERATE ERRORS) HERE
		
		
		// see if each sensor is ready to add a batch, and do so if we need to
		if (loops_since_last_log[IR_DATA] >= attitude_IR_LOOPS_PER_LOG) {
			current_struct->ir_data[data_array_tails[IR_DATA]] = read_ir_batch();
			increment_data_type(IR_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[DIODE_DATA] >= attitude_DIODE_LOOPS_PER_LOG) {
			current_struct->diode_data[data_array_tails[DIODE_DATA]] = read_diode_batch();
			increment_data_type(DIODE_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[IMU_DATA] >= attitude_IMU_LOOPS_PER_LOG) {
			current_struct->imu_data[data_array_tails[IMU_DATA]] = read_imu_batch();
			increment_data_type(IMU_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[MAGNETOMETER_DATA] >= attitude_MAGNETOMETER_LOOPS_PER_LOG) {
			current_struct->magnetometer_data[data_array_tails[MAGNETOMETER_DATA]] = read_magnetometer_batch();
			increment_data_type(MAGNETOMETER_DATA, data_array_tails, loops_since_last_log);
		}
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}