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
	
	// variable for keeping track of our current progress through an orbit
	// (= numerator of (x / IDLE_DATA_LOGS_PER_ORBIT) of an orbit)
	// we set this to the max because we want it to think we've wrapped around
	// an orbit on boot (log immediately)
	uint8_t prev_orbit_fraction = IDLE_DATA_LOGS_PER_ORBIT; 

	init_task_state(IDLE_DATA_TASK); // suspend or run on boot

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
		en_and_read_lion_temps_batch(	current_struct->lion_temps_data);
		read_ad7991_batbrd(				current_struct->lion_current_data, current_struct->panelref_lref_data);
		read_bat_charge_dig_sigs_batch(	&(current_struct->bat_charge_dig_sigs_data));
		read_proc_temp_batch(			&(current_struct->proc_temp_data)); // TODO
		read_radio_temp_batch(			&(current_struct->radio_temp_data));
		read_ir_ambient_temps_batch(	current_struct->ir_amb_temps_data);
		
		// TODO: DO CHECKS FOR ERRORS (TO GENERATE ERRORS) HERE
		verify_regulators();
		verify_flash_readings(false); // not flashing (function is thread-safe)

		// once we've collected all the data we need to into the current struct, add the whole thing
		// if we took too long between the start of this packet and here, 
		// DON'T add it and go on to rewrite the current one
		TickType_t data_read_time = (xTaskGetTickCount() / portTICK_PERIOD_MS) - time_before_data_read;
		if (data_read_time <= IDLE_DATA_MAX_READ_TIME) {
			if (passed_orbit_fraction(&prev_orbit_fraction, IDLE_DATA_LOGS_PER_ORBIT)) {
				// validate previous stored value in stack, getting back the next staged address we can start adding to
				current_struct = (idle_data_t*) equistack_Stage(&idle_readings_equistack);
			}
		} else {
			// log error if the data read took too long
			log_error(ELOC_IDLE_DATA, ECODE_EXCESSIVE_SUSPENSION, false);
		}
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}
