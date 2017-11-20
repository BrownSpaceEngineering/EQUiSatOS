/*
 * low_power_data_task.c
 *
 * Created: 9/21/2017 20:42:19
 *  Author: mcken
 */ 

#include "rtos_tasks.h"

void low_power_data_task(void *pvParameters)
{
	// initialize xNextWakeTime onces
	TickType_t prev_wake_time = xTaskGetTickCount();										
	
	// initialize first struct
	low_power_data_t *current_struct = (low_power_data_t*) equistack_Initial_Stage(&low_power_readings_equistack);
	assert(current_struct != NULL); // TESTING
	current_struct->timestamp = get_rtc_count();
	
	init_task_state(LOW_POWER_DATA_TASK); // suspend or run on boot
	
	for( ;; )
	{	
		vTaskDelayUntil( &prev_wake_time, LOW_POWER_DATA_TASK_FREQ / portTICK_PERIOD_MS);
		
		// report to watchdog
		report_task_running(LOW_POWER_DATA_TASK);
		
		// clear any previous suspend flags
		check_if_suspended_and_update(LOW_POWER_DATA_TASK);
		
		// set start timestamp
		current_struct->timestamp = get_rtc_count();
		
		// add all sensors to batch
		
		// TODO: read satellite history somehow
		
		//read_lion_volts_batch(current_struct->lion_volts_data);
		//read_lion_current_batch(current_struct->lion_current_data);
		read_lion_temps_batch(current_struct->lion_temps_data);
		read_bat_charge_volts_batch(current_struct->bat_charge_volts_data);
		read_bat_charge_dig_sigs_batch(&(current_struct->bat_charge_dig_sigs_data));
		read_ir_object_temps_batch(current_struct->ir_obj_temps_data);
		read_gyro_batch(current_struct->gyro_data);
		
		// TODO: DO CHECKS FOR ERRORS (TO GENERATE ERRORS) HERE
		
		// once we've collected all the data we need to into the current struct, add the whole thing
		// if we were suspended in some period between start of this packet and here, DON'T add it
		// and go on to rewrite the current one
		if (!check_if_suspended_and_update(LOW_POWER_DATA_TASK)) {
			// validate previous stored value in stack, getting back the next staged address we can start adding to
			current_struct = (low_power_data_t*) equistack_Stage(&low_power_readings_equistack);
		}
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}