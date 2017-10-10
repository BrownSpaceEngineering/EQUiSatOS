/*
 * transmit_data_task.c
 *
 * Created: 9/21/2017 20:43:16
 *  Author: mcken
 */ 

#include "rtos_tasks.h"

void transmit_data_task(void *pvParameters)
{
	// initialize xNextWakeTime onces
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	// see current_data_task for extensive comments / testing
	
	// tracking arrays
	uint8_t loops_since_last_log[NUM_DATA_TYPES];
	uint8_t data_array_tails[NUM_DATA_TYPES];
	
	transmit_data_t *current_struct = (transmit_data_t*) equistack_Initial_Stage(&flash_readings_equistack);
	
	for ( ;; )
	{
		vTaskDelayUntil( &xNextWakeTime, TRANSMIT_DATA_TASK_FREQ / portTICK_PERIOD_MS);
		
		// update current_struct if necessary
		if (check_if_suspended_and_update(TRANSMIT_DATA_TASK) || data_array_tails[LION_CURRENT_DATA] >= transmit_LION_VOLTS_DATA_ARR_LEN)
		{
			// validate previous stored value in stack, getting back the next staged address we can start adding to
			current_struct = (transmit_data_t*) equistack_Stage(&flash_readings_equistack);
			current_struct->timestamp = get_current_timestamp();
			
			// log state read
			msg_data_type_t last_reading = TRANSMIT_DATA; // need a stack pointer to memcpy from
			equistack_Push(&last_reading_type_equistack, &last_reading);
			
			// reset data array tails so we're writing at the start // TODO: loops_since_last_log = ...; ???
			set_all(data_array_tails, NUM_DATA_TYPES, 0);
		}
		
		// TODO: DO CHECKS FOR ERRORS (TO GENERATE ERRORS) HERE
		
		
		// see if each sensor is ready to add a batch, and do so if we need to
		if (loops_since_last_log[RADIO_TEMP_DATA] >= transmit_RADIO_TEMP_LOOPS_PER_LOG) {
			current_struct->radio_temp_data[data_array_tails[RADIO_TEMP_DATA]] = read_radio_temp_batch();
			increment_data_type(RADIO_TEMP_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[LION_VOLTS_DATA] >= transmit_LION_VOLTS_LOOPS_PER_LOG) {
			current_struct->lion_volts_data[data_array_tails[LION_VOLTS_DATA]] = read_lion_volts_batch();
			increment_data_type(LION_VOLTS_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[LION_CURRENT_DATA] >= transmit_LION_CURRENT_LOOPS_PER_LOG) {
			current_struct->lion_current_data[data_array_tails[LION_CURRENT_DATA]] = read_lion_current_batch();
			increment_data_type(LION_CURRENT_DATA, data_array_tails, loops_since_last_log);
		}
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}
