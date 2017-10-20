/*
 * flash_data_task.c
 *
 * Created: 9/21/2017 20:44:03
 *  Author: mcken
 */ 

#include "rtos_tasks.h"

void flash_data_task(void *pvParameters)
{
	// initialize xNextWakeTime onces
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	// current_data_task for extensive comments / testing
	
	// tracking arrays
	uint8_t loops_since_last_log[NUM_DATA_TYPES];
	uint8_t data_array_tails[NUM_DATA_TYPES];
	
	flash_data_t *current_struct = (flash_data_t*) equistack_Initial_Stage(&attitude_readings_equistack);
	
	for ( ;; )
	{
		vTaskDelayUntil( &xNextWakeTime, FLASH_DATA_TASK_FREQ / portTICK_PERIOD_MS);
		
		// update current_struct if necessary
		if (check_if_suspended_and_update(FLASH_DATA_TASK) || data_array_tails[LED_TEMPS_DATA] >= flash_LED_TEMPS_DATA_ARR_LEN)
		{
			// validate previous stored value in stack, getting back the next staged address we can start adding to
			current_struct = (flash_data_t*) equistack_Stage(&attitude_readings_equistack);
			current_struct->timestamp = get_current_timestamp();
			
			// log state read
			msg_data_type_t last_reading = FLASH_DATA; // need a stack pointer to memcpy from
			equistack_Push(&last_reading_type_equistack, &last_reading);
			
			// reset data array tails so we're writing at the start // TODO: loops_since_last_log = ...; ???
			set_all(data_array_tails, NUM_DATA_TYPES, 0);
		}
		
		// TODO: DO CHECKS FOR ERRORS (TO GENERATE ERRORS) HERE
		
		
		// 		// see if each sensor is ready to add a batch, and do so if we need to
		if (loops_since_last_log[LED_TEMPS_DATA] >= flash_LED_TEMPS_LOOPS_PER_LOG) {
			read_led_temps_batch(current_struct->led_temps_data[data_array_tails[LED_TEMPS_DATA]]);
			increment_data_type(LED_TEMPS_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[LIFEPO_VOLTS_DATA] >= flash_LED_TEMPS_LOOPS_PER_LOG) {
			read_lifepo_current_batch(current_struct->lifepo_current_data[data_array_tails[LIFEPO_VOLTS_DATA]]);
			increment_data_type(LIFEPO_VOLTS_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[LIFEPO_CURRENT_DATA] >= flash_LIFEPO_CURRENT_LOOPS_PER_LOG) {
			read_lifepo_volts_batch(current_struct->lifepo_volts_data[data_array_tails[LIFEPO_CURRENT_DATA]]);
			increment_data_type(LIFEPO_CURRENT_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[LED_CURRENT_DATA] >= flash_LED_CURRENT_LOOPS_PER_LOG) {
			read_led_current_batch(current_struct->led_current_data[data_array_tails[LED_CURRENT_DATA]]);
			increment_data_type(LED_CURRENT_DATA, data_array_tails, loops_since_last_log);
		}
		
		// TODO: Do averages too!
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}