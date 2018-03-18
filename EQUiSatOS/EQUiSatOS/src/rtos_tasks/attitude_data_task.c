/*
 * antenna_data_task.c
 *
 * Created: 9/21/2017 20:44:50
 *  Author: mcken
 */ 

#include "rtos_tasks.h"

void attitude_data_task(void *pvParameters)
{
	// delay to offset task relative to others, then start
	vTaskDelay(ATTITUDE_DATA_TASK_FREQ_OFFSET);
	TickType_t prev_wake_time = xTaskGetTickCount();
	
	attitude_data_t *current_struct = (attitude_data_t*) equistack_Initial_Stage(&attitude_readings_equistack);
	
	// variable for timing data reads (which may include task suspensions)
	TickType_t time_before_data_read;
	
	init_task_state(ATTITUDE_DATA_TASK); // suspend or run on boot
	
	// variable for keeping track of data logging to distribute over orbit
	uint32_t time_of_last_log_s = 0; // try to log ASAP (on first task start)
	
	for ( ;; )
	{
		vTaskDelayUntil( &prev_wake_time, ATTITUDE_DATA_TASK_FREQ / portTICK_PERIOD_MS);
		
		// report to watchdog
		report_task_running(ATTITUDE_DATA_TASK);
		
		// set start timestamp
		current_struct->timestamp = get_current_timestamp();

		// time the data reading to make sure it doesn't exceed a maximum
		time_before_data_read = xTaskGetTickCount() / portTICK_PERIOD_MS;
		
		// the next couple readings use IR power on, so turn it on for them
		// to speed up the readings (otherwise they have to wait for it to come on each time)
		bool got_semaphore = enable_ir_pow_if_necessary();
		{
			// read all sensors first
			read_ir_object_temps_batch(	current_struct->ir_obj_temps_data);
			read_pdiode_batch(			&(current_struct->pdiode_data));
			read_gyro_batch(			current_struct->gyro_data);
			read_accel_batch(			current_struct->accelerometer_data	[0]);
			read_magnetometer_batch(	current_struct->magnetometer_data	[0]);
		
			// delay a bit and take a second batch of readings to allow rate measurements
			// TODO: may want to error if this is off (by a tighter bound than ATTITUDE_DATA_MAX_READ_TIME)
			vTaskDelay(ATTITUDE_DATA_SECOND_SAMPLE_DELAY / portTICK_PERIOD_MS);
			read_accel_batch(			current_struct->accelerometer_data	[1]);
			read_magnetometer_batch(	current_struct->magnetometer_data	[1]);
		}
		// try to disable IR power because we're done, but only if we get the mutex (we expect it to be on so no errors)
		disable_ir_pow_if_necessary(got_semaphore);
	
		// if we were suspended in some period between start of this packet and here, DON'T add it
		// and go on to rewrite the current one
		TickType_t data_read_time = (xTaskGetTickCount() / portTICK_PERIOD_MS) - time_before_data_read;
		if (data_read_time <= ATTITUDE_DATA_MAX_READ_TIME) {
			uint32_t time_since_last_log_s = get_current_timestamp() - time_of_last_log_s;
			if (time_since_last_log_s >= ATTITUDE_DATA_LOG_FREQ_S) {
				// validate previous stored value in stack, getting back the next staged address we can start adding to
				current_struct = (attitude_data_t*) equistack_Stage(&attitude_readings_equistack);
				time_of_last_log_s = get_current_timestamp();
			}
		} else {
			// log error if the data read took too long
			log_error(ELOC_ATTITUDE_DATA, ECODE_EXCESSIVE_SUSPENSION, false);
		}
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}