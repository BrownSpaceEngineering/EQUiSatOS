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
#define BATCH_READS_DURING		5 // = 100ms / FLASH_DATA_READ_FREQ
#define BATCH_READS_AFTER		1

uint32_t prev_wake_time_s = 0; // global so we can reference it outside the task function

/* because we need to average the burst values, we keep a struct of the sums (which needs to be larger data types),
   and then divide the corresponding values to calculate the final comparison struct */
struct flash_burst_data_sums
{
	uint16_t led_temps_data_sums			[4];
	uint16_t lifepo_bank_temps_data_sums	[2];
	uint16_t led_current_data_sums			[4];
	uint16_t lifepo_current_data_sums		[4];
	uint16_t lifepo_volts_data_sums			[4];
};

void sum_piecewise_uint8(uint16_t* arr, uint8_t* to_add, int len);
void average_piecewise_uint8(uint8_t* results, uint16_t* sums, uint16_t size, int len);
void read_flash_data_batch(flash_data_t* burst_struct, uint8_t* data_arrays_tail,
							struct flash_burst_data_sums* sums_struct, bool add_to_sum);
void read_flash_data_batches(flash_data_t* current_struct, uint8_t* data_arrays_tail,
							struct flash_burst_data_sums* sums_struct,
							uint8_t num, TickType_t* prev_data_read_time, bool add_to_sum);

// returns the time to next flash (referring to the next flash if we're currently flashing),
// -1 = all 1s if the flash task is currently suspended, i.e. we're not flashing.
uint32_t get_time_of_next_flash(void) {
	if (eTaskGetState(task_handles[FLASH_ACTIVATE_TASK]) != eSuspended) {
		// previous wake time is the last time (using get_current_timestamp()) that vTaskDelayUntil resumed in this task,
		// so that plus the frequency gives us an approximate time of next flash
		return prev_wake_time_s + FLASH_ACTIVATE_TASK_FREQ / 1000;	// TODO: what if RTOS goes to something higher priority?
	}
	return (uint32_t) -1;
}

void flash_activate_task(void *pvParameters)
{
	// delay to offset task relative to others, then start
	vTaskDelay(FLASH_ACTIVATE_TASK_FREQ_OFFSET);
	TickType_t prev_wake_time = xTaskGetTickCount();	
	TickType_t prev_data_read_time = xTaskGetTickCount();
	
	// variable for keeping track of our current progress through an orbit
	// (= numerator of (x / FLASH_CMP_DATA_LOGS_PER_ORBIT) of an orbit)
	// we set this to the max because we want it to think we've wrapped around
	// an orbit on boot (log immediately)
	uint8_t prev_orbit_fraction = FLASH_CMP_DATA_LOGS_PER_ORBIT;

	// data storage variables for flash data
	uint8_t data_arrays_tail = 0;
	flash_data_t *current_burst_struct = (flash_data_t*) equistack_Initial_Stage(&flash_readings_equistack);
	struct flash_burst_data_sums current_sums_struct;
	flash_cmp_data_t *current_cmp_struct = (flash_cmp_data_t*) equistack_Initial_Stage(&flash_cmp_readings_equistack);
	
	init_task_state(FLASH_ACTIVATE_TASK); // suspend or run on boot
	
	for ( ;; )
	{	
		vTaskDelayUntil( &prev_wake_time, FLASH_ACTIVATE_TASK_FREQ / portTICK_PERIOD_MS);
		prev_wake_time_s = get_current_timestamp();
		
		// report to watchdog
		report_task_running(FLASH_ACTIVATE_TASK);
		
		// read a single magnetometer batch before flash
		read_magnetometer_batch(current_cmp_struct->mag_before_data);
		
		// actually flash leds
		for (int i = 0; i < NUM_FLASHES; i++) {
			// start taking data and set start timestamp
			uint32_t cur_timestamp = get_current_timestamp();
			current_burst_struct->timestamp = cur_timestamp;
			current_cmp_struct->timestamp = cur_timestamp;
				
			// obtain i2c_irpow_mutex throughout flash, to speed up sensor reads,
			// and enable 5V regulator throughout as well
			// NOTE: order is intentional!
			bool actually_flashed = false;
			if (xSemaphoreTake(critical_action_mutex, CRITICAL_MUTEX_WAIT_TIME_TICKS))
			{
				if (xSemaphoreTake(i2c_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
				{
					_enable_ir_pow_if_necessary();
					if (xSemaphoreTake(processor_adc_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
					{
						_set_5v_enable(true);
				
						// enable lifepo output (before first data read to give a time buffer before flashing),
						// and make sure the flash enable pin is high
						// If we can't, halt flash (this may happen due to a mutex timeout; very very unlikely)
						if (!flash_arm()) {
							// error would've been logged
							_set_5v_enable(false);
							xSemaphoreGive(processor_adc_mutex);
							xSemaphoreGive(i2c_mutex);
							xSemaphoreGive(critical_action_mutex);
							continue;
						}

					trace_print("Starting flash @ %d ticks", xTaskGetTickCount());
			
						// delays for time of FLASH_DATA_READ_FREQ, as required by flash_arm
						read_flash_data_batches(current_burst_struct, &data_arrays_tail, &current_sums_struct, 
												BATCH_READS_BEFORE, &prev_data_read_time, false);

						// send actual falling edge to flash circuitry to activate it
						flash_activate();
			
						// read data during the flash of 100ms
						read_flash_data_batches(current_burst_struct, &data_arrays_tail, &current_sums_struct,
												BATCH_READS_DURING, &prev_data_read_time, true);
									
					trace_print("Ending flash @ %d ticks", xTaskGetTickCount());
			
						// read data after the flash
						read_flash_data_batches(current_burst_struct, &data_arrays_tail, &current_sums_struct,
												BATCH_READS_AFTER, &prev_data_read_time, false);
									
						// turn off the lifepos after the 100ms of data reading
						// NOTE this does NOT actually stop the flashing - that is hardware controlled
						// (put after last data read to keep from accidentally cutting off the flash)
						flash_disarm();
						
						actually_flashed = true;
				
						xSemaphoreGive(processor_adc_mutex);
					} else {
						log_error(ELOC_FLASH, ECODE_PROC_ADC_MUTEX_TIMEOUT, true);
					}	
					// disable sensor regulators and free up mutexes
					_set_5v_enable(false);
					xSemaphoreGive(i2c_mutex);
				} else {
					log_error(ELOC_FLASH, ECODE_I2C_MUTEX_TIMEOUT, true);
				}
				xSemaphoreGive(critical_action_mutex);
			} else {
				log_error(ELOC_FLASH, ECODE_PROC_ADC_MUTEX_TIMEOUT, true);
			}
			
			// in case any mutex didn't get locked
			if (!actually_flashed) {
				continue;
			}
			
			// update sat event history if we flashed and it wasn't noted
			// TODO: every freaking time?
			if (!cache_get_sat_event_history().first_flash) {
				update_sat_event_history(0, 0, 0, 0, 0, 1, 0);
			}
			
			configASSERT (data_arrays_tail <= FLASH_DATA_ARR_LEN);
			
			
			// store burst data in equistack
			current_burst_struct = (flash_data_t*) equistack_Stage(&flash_readings_equistack);
			// reset data array tails so we're writing at the start
			data_arrays_tail = 0;
			
			// delay between successive flashes
			vTaskDelay(TIME_BTWN_FLASHES / portTICK_PERIOD_MS); // delay on last iteration as well is OK
		}
		
		// using the sums, compute and populate the flash compare struct corresponding to this burst
		average_piecewise_uint8(current_cmp_struct->led_temps_avg_data, current_sums_struct.led_current_data_sums,
			NUM_FLASHES*FLASH_DATA_ARR_LEN, 4);
		average_piecewise_uint8(current_cmp_struct->lifepo_bank_temps_avg_data, current_sums_struct.lifepo_bank_temps_data_sums,
			NUM_FLASHES*FLASH_DATA_ARR_LEN, 2);
		average_piecewise_uint8(current_cmp_struct->lifepo_current_avg_data, current_sums_struct.lifepo_current_data_sums,
			NUM_FLASHES*FLASH_DATA_ARR_LEN, 4);
		average_piecewise_uint8(current_cmp_struct->lifepo_volts_avg_data, current_sums_struct.lifepo_volts_data_sums,
			NUM_FLASHES*FLASH_DATA_ARR_LEN, 4);
		average_piecewise_uint8(current_cmp_struct->led_current_avg_data, current_sums_struct.led_current_data_sums,
			NUM_FLASHES*FLASH_DATA_ARR_LEN, 4);
			
		// store cmp data in flash equistack, but distribute over orbit
		if (passed_orbit_fraction(&prev_orbit_fraction, FLASH_CMP_DATA_LOGS_PER_ORBIT)) {
			current_cmp_struct = (flash_cmp_data_t*) equistack_Stage(&flash_readings_equistack);
		}
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}

// averaging helpers
void sum_piecewise_uint8(uint16_t* arr, uint8_t* to_add, int len)
{
	for (int i = 0; i < len; i++)
	{
		arr[i] += to_add[i];
	}
}

void average_piecewise_uint8(uint8_t* results, uint16_t* sums, uint16_t size, int len)
{
	for (int i = 0; i < len; i++)
	{
		results[i] = sums[i] / size;
	}
}

/* reads a single batch of flash data and adds those to the sums struct for later averaging */
void read_flash_data_batch(flash_data_t* burst_struct, uint8_t* data_arrays_tail,
		struct flash_burst_data_sums* sums_struct, bool add_to_sum)
{
	// for data type in the flash batch, read the data, add it to the burst array, and
	// also add it to the average
	// (note below that we can't assign to arrays, so we have to use pointers to the arrays)
	led_temps_batch* led_temps = &burst_struct->led_temps_data[*data_arrays_tail];
	_read_led_temps_batch_unsafe(*led_temps);
	if (add_to_sum) sum_piecewise_uint8(sums_struct->led_temps_data_sums, *led_temps, 4);
	
	lifepo_bank_temps_batch* lifepo_bank_temps = &burst_struct->lifepo_bank_temps_data[*data_arrays_tail];
	_read_lifepo_temps_batch_unsafe(*lifepo_bank_temps);
	if (add_to_sum) sum_piecewise_uint8(sums_struct->lifepo_bank_temps_data_sums, *lifepo_bank_temps, 2);
	
	lifepo_current_batch* lifepo_current = &burst_struct->lifepo_current_data[*data_arrays_tail];
	_read_lifepo_current_batch_unsafe(*lifepo_current, true);
	if (add_to_sum) sum_piecewise_uint8(sums_struct->lifepo_current_data_sums, *lifepo_current, 4);

	lifepo_volts_batch* lifepo_volts = &burst_struct->lifepo_volts_data[*data_arrays_tail];
	_read_lifepo_volts_batch_unsafe(*lifepo_volts);
	if (add_to_sum) sum_piecewise_uint8(sums_struct->lifepo_volts_data_sums, *lifepo_volts, 4);

	led_current_batch* led_current = &burst_struct->led_current_data[*data_arrays_tail];
	_read_led_current_batch_unsafe(*led_current, true);
	if (add_to_sum) sum_piecewise_uint8(sums_struct->led_current_data_sums, *led_current, 4);

	gyro_batch* gyro = &burst_struct->gyro_data[*data_arrays_tail];
	_read_gyro_batch_unsafe(*gyro);
	if (add_to_sum) sum_piecewise_uint8(sums_struct->led_current_data_sums, *gyro, 4);
	
	(*data_arrays_tail)++;
}

/* reads a set of flash data batches, making sure they're space on their frequency */
void read_flash_data_batches(flash_data_t* current_struct, uint8_t* data_arrays_tail,
								struct flash_burst_data_sums* sums_struct,
								uint8_t num, TickType_t* prev_data_read_time, bool add_to_sum)
{
	for (uint8_t i = 0; i < num; i++)
	{
		read_flash_data_batch(current_struct, data_arrays_tail, sums_struct, add_to_sum);
		// delay until FLASH_DATA_READ_FREQ (in ticks) after our previous read time.
		// if the read took a long time, this will return immediately (not on time), but
		// otherwise it will ensure the frequency of data reading is constant
		vTaskDelayUntil(prev_data_read_time, FLASH_DATA_READ_FREQ / portTICK_PERIOD_MS);
	}
}
