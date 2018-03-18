/*
 * idle_data_task.c
 *
 * Created: 9/21/2017 20:42:19
 *  Author: mcken
 */

#include "rtos_tasks.h"

void idle_data_task(void *pvParameters)
{
	// delay to offset task relative to others, then start
	vTaskDelay(IDLE_DATA_TASK_FREQ_OFFSET);
	TickType_t prev_wake_time = xTaskGetTickCount();

	// initialize first struct
	idle_data_t *current_struct = (idle_data_t*) equistack_Initial_Stage(&idle_readings_equistack);
	assert(current_struct != NULL); // TESTING
	
	// variable for timing data reads (which may include task suspensions)
	TickType_t time_before_data_read;
	
	init_task_state(IDLE_DATA_TASK); // suspend or run on boot
	
	// variable for keeping track of data logging to distribute over orbit
	uint32_t time_of_last_log_s = 0; // try to log ASAP (on first task start) (makes time since log large)

	for( ;; )
	{
		vTaskDelayUntil( &prev_wake_time, IDLE_DATA_TASK_FREQ / portTICK_PERIOD_MS);

		// report to watchdog
		report_task_running(IDLE_DATA_TASK);

		// set start timestamp
		current_struct->timestamp = get_current_timestamp();

		// time the data reading to make sure it doesn't exceed a maximum
		time_before_data_read = xTaskGetTickCount() / portTICK_PERIOD_MS;

		// add all sensors to batch
		current_struct->satellite_history = cache_get_sat_event_history();
		read_lion_volts_batch(			current_struct->lion_volts_data);
		// the next couple readings use IR power on, so turn it on for them
		// to speed up the readings (otherwise they have to wait for it to come on each time)
		bool got_semaphore = enable_ir_pow_if_necessary();
		{
			en_and_read_lion_temps_batch(	current_struct->lion_temps_data);
			read_ad7991_batbrd(				current_struct->lion_current_data, current_struct->panelref_lref_data);
			read_bat_charge_dig_sigs_batch(	&(current_struct->bat_charge_dig_sigs_data));
			read_imu_temp_batch(			&(current_struct->imu_temp_data));
			read_radio_temp_batch(			&(current_struct->radio_temp_data));
			read_ir_ambient_temps_batch(	current_struct->ir_amb_temps_data);
		
			// verify readings (without storing) at regular intervals in this task
			verify_regulators();
			verify_flash_readings(false); // not flashing (function is thread-safe)
		}
		// try to disable IR power because we're done, but only if we get the mutex (we expect it to be on so no errors)
		disable_ir_pow_if_necessary(got_semaphore);
		
		// in this task only, manually disable IR power if it's on and on one's using it
		ensure_ir_power_disabled(false);

		// once we've collected all the data we need to into the current struct, add the whole thing
		// if we took too long between the start of this packet and here, 
		// DON'T add it and go on to rewrite the current one
		// NOTE: if timestamp overflows, everything works because tick type is a uint
		TickType_t data_read_time = (xTaskGetTickCount() / portTICK_PERIOD_MS) - time_before_data_read;
		if (data_read_time <= IDLE_DATA_MAX_READ_TIME) {
			uint32_t time_since_last_log_s = get_current_timestamp() - time_of_last_log_s;
			if (time_since_last_log_s >= IDLE_DATA_LOG_FREQ_S) {
				// validate previous stored value in stack, getting back the next staged address we can start adding to
				current_struct = (idle_data_t*) equistack_Stage(&idle_readings_equistack);
				time_of_last_log_s = get_current_timestamp();
			}
		} else {
			// log error if the data read took too long
			log_error(ELOC_IDLE_DATA, ECODE_EXCESSIVE_SUSPENSION, false);
		}
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}
