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

/* because we need to average the burst values, we keep a struct of the sums (which needs to be larger data types),
   and then divide the corresponding values to calculate the final comparison struct */
struct flash_burst_data_sums
{
	uint16_t led_temps_data_sums[4];
	uint16_t lifepo_bank_temps_data_sums[2];
	uint16_t led_current_data_sums[4];
	uint16_t lifepo_current_data_sums[4];
	uint16_t lifepo_volts_data_sums[4];
};

// averaging helpers
void sum_piecewise(uint16_t* arr, uint8_t* to_add, int len)
{
	for (int i = 0; i < len; i++)
	{
		arr[i] += to_add[i];
	}
}

void average_piecewise(uint8_t* results, uint16_t* sums, uint16_t size, int len)
{
	for (int i = 0; i < len; i++)
	{
		results[i] = sums[i] / size;
	}
}

void read_flash_data_batch(flash_data_t* burst_struct, uint8_t* data_arrays_tail, 
							struct flash_burst_data_sums* sums_struct)
{
	
	// for data type in the flash batch, read the data, add it to the burst array, and 
	// also add it to the average
	// (note below that we can't assign to arrays, so we have to use pointers to the arrays)
	led_temps_batch* led_temps = &burst_struct->led_temps_data[*data_arrays_tail];
	read_led_temps_batch(*led_temps);
	sum_piecewise(sums_struct->led_temps_data_sums, *led_temps, 4);
	
	lifepo_bank_temps_batch* lifepo_bank_temps = &burst_struct->lifepo_bank_temps_data[*data_arrays_tail];
	read_lifepo_temps_batch(*lifepo_bank_temps);
	sum_piecewise(sums_struct->lifepo_bank_temps_data_sums, *lifepo_bank_temps, 2);
		
	lifepo_current_batch* lifepo_current = &burst_struct->lifepo_current_data[*data_arrays_tail];
	read_lifepo_current_batch(*lifepo_current);
	sum_piecewise(sums_struct->led_current_data_sums, *lifepo_current, 4);

	lifepo_volts_batch* lifepo_volts = &burst_struct->lifepo_volts_data[*data_arrays_tail];
	read_lifepo_volts_batch(*lifepo_volts);
	sum_piecewise(sums_struct->lifepo_current_data_sums, *lifepo_volts, 4);

	led_current_batch* led_current = &burst_struct->led_current_data[*data_arrays_tail];
	read_led_current_batch(*led_current);
	sum_piecewise(sums_struct->lifepo_volts_data_sums, *led_current, 4);
		
	(*data_arrays_tail)++;
}

void read_flash_data_batches(flash_data_t* current_struct, uint8_t* data_arrays_tail, 
								struct flash_burst_data_sums* sums_struct,
								uint8_t num, TickType_t* prev_data_read_time) 
{
	for (uint8_t i = 0; i < num; i++)
	{
		read_flash_data_batch(current_struct, data_arrays_tail, sums_struct);
		// delay until FLASH_DATA_READ_FREQ (in ticks) after our previous read time.
		// if the read took a long time, this will return immediately (not on time), but
		// otherwise it will ensure the frequency of data reading is constant
		vTaskDelayUntil(prev_data_read_time, FLASH_DATA_READ_FREQ / portTICK_PERIOD_MS);		
		
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
	uint8_t data_arrays_tail;
	flash_data_t *current_burst_struct = (flash_data_t*) equistack_Initial_Stage(&flash_readings_equistack);
	struct flash_burst_data_sums current_sums_struct;
	flash_cmp_data_t *current_cmp_struct = (flash_cmp_data_t*) equistack_Initial_Stage(&flash_cmp_readings_equistack);
	
	init_task_state(FLASH_ACTIVATE_TASK); // suspend or run on boot
	
	for ( ;; )
	{	
		vTaskDelayUntil( &prev_wake_time, FLASH_ACTIVATE_TASK_FREQ / portTICK_PERIOD_MS);
		
		// report to watchdog
		report_task_running(FLASH_ACTIVATE_TASK);
		
		// actually flash leds
		for (int i = 0; i < NUM_FLASHES; i++) {
			// start taking data and set start timestamp
			uint32_t cur_timestamp = get_rtc_count();
			current_burst_struct->timestamp = cur_timestamp;
			current_cmp_struct->timestamp = cur_timestamp;
					
			read_flash_data_batches(current_burst_struct, &data_arrays_tail, &current_sums_struct, 
									BATCH_READS_BEFORE, &prev_data_read_time);
					
			vTaskSuspendAll(); // make sure RTOS is suspended for critical signals
			flash_leds(); // note contains a delay of approx. 1 ms
			xTaskResumeAll();
			
			// read data during the flash and after
			read_flash_data_batches(current_burst_struct, &data_arrays_tail, &current_sums_struct,
									BATCH_READS_DURING + BATCH_READS_AFTER, &prev_data_read_time);
			
			assert (data_arrays_tail < FLASH_DATA_ARR_LEN);
			
			// using the sums, compute and populate the flash compare struct corresponding to this burst
			average_piecewise(current_cmp_struct->led_temps_avg_data, current_sums_struct.led_current_data_sums, 
								FLASH_DATA_ARR_LEN, 4);
			average_piecewise(current_cmp_struct->lifepo_bank_temps_avg_data, current_sums_struct.lifepo_bank_temps_data_sums,
								FLASH_DATA_ARR_LEN, 2);
			average_piecewise(current_cmp_struct->lifepo_current_avg_data, current_sums_struct.lifepo_current_data_sums,
								FLASH_DATA_ARR_LEN, 4);
			average_piecewise(current_cmp_struct->lifepo_volts_avg_data, current_sums_struct.lifepo_volts_data_sums,
								FLASH_DATA_ARR_LEN, 4);
			average_piecewise(current_cmp_struct->led_current_avg_data, current_sums_struct.led_current_data_sums,
								FLASH_DATA_ARR_LEN, 4);			
			
			// stop taking data and store in struct
			current_burst_struct = (flash_data_t*) equistack_Stage(&flash_readings_equistack);
			// reset data array tails so we're writing at the start
			data_arrays_tail = 0;
			
			vTaskDelay(TIME_BTWN_FLASHES / portTICK_PERIOD_MS); // delay on last iteration as well is OK
		}
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}