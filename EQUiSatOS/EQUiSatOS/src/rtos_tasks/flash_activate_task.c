/*
 * flash_activate_task.c
 *
 * Created: 9/21/2017 20:40:34
 *  Author: mcken
 */ 

#include "../processor_drivers/Flash_Commands.h"
#include "rtos_tasks.h"

#define NUM_FLASHES				3
#define TIME_BTWN_FLASHES		1000 // ms
#define BATCH_READS_BEFORE		1
#define BATCH_READS_DURING		4
#define BATCH_READS_AFTER		1

void read_flash_data_batch(flash_data_t* current_struct, uint8_t* data_array_tails)
{
		read_led_temps_batch(current_struct->led_temps_data[data_array_tails[LED_TEMPS_DATA]]);
		
		read_lifepo_current_batch(current_struct->lifepo_current_data[data_array_tails[LIFEPO_VOLTS_DATA]]);

		read_lifepo_volts_batch(current_struct->lifepo_volts_data[data_array_tails[LIFEPO_CURRENT_DATA]]);

		read_led_current_batch(current_struct->led_current_data[data_array_tails[LED_CURRENT_DATA]]);
		
		increment_all(data_array_tails, NUM_DATA_TYPES);
}

void read_flash_data_batches(flash_data_t* current_struct, uint8_t* data_array_tails, 
							uint8_t num, TickType_t* xPrevDataReadTime) 
{
	for (uint8_t i = 0; i < num; i++)
	{
		read_flash_data_batch(current_struct, data_array_tails);
		// delay until FLASH_DATA_READ_FREQ (in ticks) after our previous read time.
		// if the read took a long time, this will return immediately (not on time), but
		// otherwise it will ensure the frequency of data reading is constant
		vTaskDelayUntil(xPrevDataReadTime, FLASH_DATA_READ_FREQ / portTICK_PERIOD_MS);		
		
		// check if suspended since last data collection (or during collection)
		// if so, we need to dump the current data struct 
// 		if (check_if_suspended_and_update(FLASH_DATA_TASK)) 
// 		{
// 					// TODO: LOG ERROR	
// 		}
	}
}

void flash_activate_task(void *pvParameters)
{
	// initialize xNextWakeTime onces
	TickType_t prev_wake_time = xTaskGetTickCount();
	TickType_t prev_data_read_time = xTaskGetTickCount();

	// data storage variables for flash data
	uint8_t data_array_tails[NUM_DATA_TYPES];
	flash_data_t *current_struct = (flash_data_t*) equistack_Initial_Stage(&flash_readings_equistack);
	
	init_task_state(FLASH_ACTIVATE_TASK); // suspend or run on boot
	
	for ( ;; )
	{	
		vTaskDelayUntil( &prev_wake_time, FLASH_ACTIVATE_TASK_FREQ / portTICK_PERIOD_MS);
		
		// report to watchdog
		report_task_running(FLASH_ACTIVATE_TASK);
		
		// actually flash leds
		for (int i = 0; i < NUM_FLASHES; i++) {
			// start taking data and set start timestamp
			current_struct->timestamp = get_rtc_count();
					
			read_flash_data_batches(current_struct, data_array_tails, BATCH_READS_BEFORE, &prev_wake_time);
					
			vTaskSuspendAll(); // make sure RTOS is suspended for critical signals
			flash_leds(); // note contains a delay of approx. 1 ms
			xTaskResumeAll();
			
			// read data during the flash and after
			read_flash_data_batches(current_struct, data_array_tails, 
									BATCH_READS_DURING + BATCH_READS_AFTER, &prev_wake_time);
			
			assert (data_array_tails[LED_TEMPS_DATA] < FLASH_DATA_ARR_LEN); // same for all
			
			
			// TODO: Compute flash compare packet
			
			
			// stop taking data and store in struct
			current_struct = (flash_data_t*) equistack_Stage(&flash_readings_equistack);
			// reset data array tails so we're writing at the start
			set_all(data_array_tails, NUM_DATA_TYPES, 0);
			
			vTaskDelay(TIME_BTWN_FLASHES / portTICK_PERIOD_MS); // delay on last iteration as well is OK
		}
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}