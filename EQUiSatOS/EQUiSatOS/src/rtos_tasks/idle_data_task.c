/*
 * idle_data_task.c
 *
 * Created: 9/21/2017 20:42:19
 *  Author: mcken
 */

#include "rtos_tasks.h"

void idle_data_task(void *pvParameters)
{
	// initialize xNextWakeTime onces
	TickType_t prev_wake_time = xTaskGetTickCount();

	// initialize first struct
	idle_data_t *current_struct = (idle_data_t*) equistack_Initial_Stage(&idle_readings_equistack);
	assert(current_struct != NULL); // TESTING
	current_struct->timestamp = get_rtc_count();

	init_task_state(IDLE_DATA_TASK); // suspend or run on boot

	for( ;; )
	{
		vTaskDelayUntil( &prev_wake_time, IDLE_DATA_TASK_FREQ / portTICK_PERIOD_MS);

		// report to watchdog
		report_task_running(IDLE_DATA_TASK);

		// clear any previous suspend flags
		check_if_suspended_and_update(IDLE_DATA_TASK);

		// set start timestamp
		current_struct->timestamp = get_rtc_count();

		// add all sensors to batch
		// read radio temp first because it takes a while to write & reset the radio
		read_radio_temp_batch(			&(current_struct->radio_temp_data));
		
		// TODO: read satellite history from somewhere
		
		// TODO: read reboot count from memory
	
		//read_lion_volts_batch(			current_struct->lion_volts_data);
		//read_lion_current_batch(		current_struct->lion_current_data);
		read_lion_temps_batch(			current_struct->lion_temps_data);
		read_bat_charge_volts_batch(	current_struct->bat_charge_volts_data);
		read_bat_charge_dig_sigs_batch(	&(current_struct->bat_charge_dig_sigs_data));
		read_rail_5v_batch(				&(current_struct->rail_5v_data));
		read_radio_temp_batch(			&(current_struct->radio_temp_data));
		read_radio_volts_batch(			&(current_struct->radio_volts_data));
		read_radio_current_batch(		&(current_struct->radio_current_data));
		read_proc_temp_batch(			&(current_struct->proc_temp_data));
		read_ir_ambient_temps_batch(	current_struct->ir_amb_temps_data);
		

		// TODO: DO CHECKS FOR ERRORS (TO GENERATE ERRORS) HERE

		// once we've collected all the data we need to into the current struct, add the whole thing
		// if we were suspended in some period between start of this packet and here, DON'T add it
		// and go on to rewrite the current one
		if (!check_if_suspended_and_update(IDLE_DATA_TASK)) {
			// validate previous stored value in stack, getting back the next staged address we can start adding to
			current_struct = (idle_data_t*) equistack_Stage(&idle_readings_equistack);
		}
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}
