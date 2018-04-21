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

// globals for use in external-facing functions
uint32_t prev_wake_time_s = 0; 
bool waiting_between_flashes = false;

/* because we need to average the burst values, we keep a struct of the sums (which needs to be larger data types),
   and then divide the corresponding values to calculate the final comparison struct */
struct flash_burst_data_sums
{
	uint16_t led_temps_data_sums			[4]; // uint16_t led_temps_batch
	uint16_t lifepo_bank_temps_data_sums	[2]; // uint16_t lifepo_bank_temps_batch
	uint16_t led_current_data_sums			[4]; // uint16_t led_current_data_batch
	uint16_t lifepo_current_data_sums		[4]; // uint16_t lifepo_current_data_batch
	uint16_t lifepo_volts_data_sums			[4]; // uint16_t lifepo_volts_data_batch
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
		return prev_wake_time_s + FLASH_ACTIVATE_TASK_FREQ / 1000;
	}
	return (uint32_t) -1;
}

// function that can be called from another task to trigger an immediate flash,
// if we're not already in one (this is mainly done in the radio task)
// Returns whether a flash was triggered (whether we weren't already flashing)
bool flash_now(void) {
	if (waiting_between_flashes) {
		xTaskAbortDelay(*task_handles[FLASH_ACTIVATE_TASK]);
		return true;
	}
	return false;
}

// Returns whether a call to flash_now() right now would immediately
// flash (caution: this is only valid at the moment of calling,
// it could change right after)
bool would_flash_now(void) {
	return (get_sat_state() == IDLE_FLASH) && waiting_between_flashes;
}

//check LED_SNS to make sure each LED turned on, log error otherwise
void validate_LEDSNS_readings(flash_data_t* cur_burst) {	
	uint16_t max_LED_current[4] = {0, 0, 0, 0};
	uint16_t max_LF_current[4] = {0, 0, 0, 0};
	for (int i = 0; i < FLASH_DATA_ARR_LEN; i++) {		
		for (int j = 0; j < 4; j++) {
			max_LED_current[j] = max(cur_burst->led_current_data[i][j], max_LED_current[j]);
			max_LF_current[j] = max(cur_burst->lifepo_current_data[i][j], max_LF_current[j]);
		}
	}
	for (int i = 0; i < 4; i++) {
		uint8_t led_eloc;
		uint8_t lf_eloc;
		sig_id_t lf_sig;
		switch(i) {
			case 0:
				led_eloc = ELOC_LED1SNS;
				lf_eloc = ELOC_LFB1SNS;
				lf_sig = S_LF_SNS_FLASH_BATCH;
				break;
			case 1:
				led_eloc = ELOC_LED2SNS;
				lf_eloc = ELOC_LFB1OSNS;
				lf_sig = S_LF_OSNS_FLASH_BATCH;
				break;
			case 2:
				led_eloc = ELOC_LED3SNS;
				lf_eloc = ELOC_LFB2SNS;
				lf_sig = S_LF_SNS_FLASH_BATCH;
				break;
			case 3:
				led_eloc = ELOC_LED4SNS;
				lf_eloc = ELOC_LFB2OSNS;
				lf_sig = S_LF_OSNS_FLASH_BATCH;
				break;
			default:
				return;
		}
		
		log_if_out_of_bounds(untruncate(max_LED_current[i], S_LED_SNS_FLASH_BATCH), S_LED_SNS_FLASH_BATCH, led_eloc, true);
		log_if_out_of_bounds(untruncate(max_LF_current[i], lf_sig), lf_sig, lf_eloc, true);
	}
}

void flash_activate_task(void *pvParameters)
{
	// delay to offset task relative to others, then start
	vTaskDelay(FLASH_ACTIVATE_TASK_FREQ_OFFSET);
	TickType_t prev_wake_time = xTaskGetTickCount(); // good on tick count overflow (only used with RTOS api calls)
	TickType_t prev_data_read_time = xTaskGetTickCount(); // good on tick count overflow (only used with RTOS api calls)
	
	// data storage variables for flash data
	uint8_t data_arrays_tail = 0;
	flash_data_t *current_burst_struct = (flash_data_t*) equistack_Initial_Stage(&flash_readings_equistack);
	struct flash_burst_data_sums current_sums_struct;
	flash_cmp_data_t *current_cmp_struct = (flash_cmp_data_t*) equistack_Initial_Stage(&flash_cmp_readings_equistack);
	
	init_task_state(FLASH_ACTIVATE_TASK); // suspend or run on boot
	
	// variable for keeping track of data logging to distribute over orbit
	uint32_t time_of_last_log_s = 0; // try to log ASAP (on first task start)
	
	for ( ;; )
	{	
		// note that we're in the long wait between flashes so that the flash_now 
		// command won't release the task from one of the other vTaskDelays
		waiting_between_flashes = true;
		vTaskDelayUntil( &prev_wake_time, FLASH_ACTIVATE_TASK_FREQ / portTICK_PERIOD_MS);
		waiting_between_flashes = false;
		prev_wake_time_s = get_current_timestamp();
		
		// report to watchdog
		report_task_running(FLASH_ACTIVATE_TASK);
		
		// read a single magnetometer batch before flash
		read_magnetometer_batch(current_cmp_struct->mag_before_data);

		// clear out our average sums struct
		memset(&current_sums_struct, 0, sizeof(struct flash_burst_data_sums));
		
		print("Starting FLASH sequence");
		
		// actually flash leds (make sure we're not transmitting or deploying antenna while this is going on)
		if (xSemaphoreTake(critical_action_mutex, CRITICAL_MUTEX_WAIT_TIME_TICKS))
		{
			// turn on IR power before we start to use it during flash
			bool got_semaphore = enable_ir_pow_if_necessary();
			for (int i = 0; i < NUM_FLASHES; i++) {
				// start taking data and set start timestamp
				uint32_t cur_timestamp = get_current_timestamp();
				current_burst_struct->timestamp = cur_timestamp;
				current_cmp_struct->timestamp = cur_timestamp;
				
				// obtain i2c_irpow_mutex throughout flash, to speed up sensor reads,
				// and enable 5V regulator throughout as well
				// NOTE: order is intentional!
				bool actually_flashed = false;
				if (xSemaphoreTake(i2c_irpow_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
				{
					if (xSemaphoreTake(processor_adc_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
					{
						_set_5v_enable_unsafe(true);
				
						// enable lifepo output (before first data read to give a time buffer before flashing),
						// and make sure the flash enable pin is high
						flash_arm();

						// delays for time of FLASH_DATA_READ_FREQ, as required by flash_arm
						read_flash_data_batches(current_burst_struct, &data_arrays_tail, &current_sums_struct, 
												BATCH_READS_BEFORE, &prev_data_read_time, false);

					trace_print("Starting flash @ %d ticks", xTaskGetTickCount());

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
				
						// disable sensor regulators and free up mutexes
						_set_5v_enable_unsafe(false);
						xSemaphoreGive(processor_adc_mutex);
					} else {
						log_error(ELOC_FLASH, ECODE_PROC_ADC_MUTEX_TIMEOUT, false);
					}
					xSemaphoreGive(i2c_irpow_mutex);
				} else {
					log_error(ELOC_FLASH, ECODE_I2C_IRPOW_MUTEX_TIMEOUT, false);
				}
				
				// in case any mutex didn't get locked
				if (!actually_flashed) {
					// note critical action mutex is given OUTSIDE this loop
					continue;
				}
			
				// update sat event history if we flashed and it wasn't noted
				if (!cache_get_sat_event_history().first_flash) {
					update_sat_event_history(0, 0, 0, 0, 0, 1, 0);
				}
			
				configASSERT (data_arrays_tail <= FLASH_DATA_ARR_LEN);
					
				validate_LEDSNS_readings(current_burst_struct)							;
			
				// store burst data in equistack
				current_burst_struct = (flash_data_t*) equistack_Stage(&flash_readings_equistack);
				// reset data array tails so we're writing at the start
				data_arrays_tail = 0;
			
				// delay between successive flashes
				vTaskDelay(TIME_BTWN_FLASHES / portTICK_PERIOD_MS); // delay on last iteration as well is OK
			}
			disable_ir_pow_if_necessary(got_semaphore);
			xSemaphoreGive(critical_action_mutex);
		} else {
			log_error(ELOC_FLASH, ECODE_CRIT_ACTION_MUTEX_TIMEOUT, false);
			// skip logging flash cmp on failure
			continue;
		}
		
		// using the sums, compute and populate the flash compare struct corresponding to this burst
		average_piecewise_uint8(current_cmp_struct->led_temps_avg_data, current_sums_struct.led_current_data_sums,
			NUM_FLASHES * BATCH_READS_DURING, 4);
		average_piecewise_uint8(current_cmp_struct->lifepo_bank_temps_avg_data, current_sums_struct.lifepo_bank_temps_data_sums,
			NUM_FLASHES * BATCH_READS_DURING, 2);
		average_piecewise_uint8(current_cmp_struct->lifepo_current_avg_data, current_sums_struct.lifepo_current_data_sums,
			NUM_FLASHES * BATCH_READS_DURING, 4);
		average_piecewise_uint8(current_cmp_struct->lifepo_volts_avg_data, current_sums_struct.lifepo_volts_data_sums,
			NUM_FLASHES * BATCH_READS_DURING, 4);
		average_piecewise_uint8(current_cmp_struct->led_current_avg_data, current_sums_struct.led_current_data_sums,
			NUM_FLASHES * BATCH_READS_DURING, 4);
	
		// store cmp data in flash equistack, but distribute over orbit
		uint32_t time_since_last_log_s = get_current_timestamp() - time_of_last_log_s;
		if (time_since_last_log_s >= FLASH_CMP_DATA_LOG_FREQ_S) {
			current_cmp_struct = (flash_cmp_data_t*) equistack_Stage(&flash_cmp_readings_equistack);
			time_of_last_log_s = get_current_timestamp();
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
		results[i] = (uint8_t) (sums[i] / size);
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
	#ifdef FLASH_ACTIVE
		_read_led_temps_batch_unsafe(*led_temps, true);
	#else
		_read_led_temps_batch_unsafe(*led_temps, false);
	#endif
	if (add_to_sum) sum_piecewise_uint8(sums_struct->led_temps_data_sums, *led_temps, 4);
	
	lifepo_bank_temps_batch* lifepo_bank_temps = &burst_struct->lifepo_bank_temps_data[*data_arrays_tail];
	_read_lifepo_temps_batch_unsafe(*lifepo_bank_temps);
	if (add_to_sum) sum_piecewise_uint8(sums_struct->lifepo_bank_temps_data_sums, *lifepo_bank_temps, 2);
	
	lifepo_current_batch* lifepo_current = &burst_struct->lifepo_current_data[*data_arrays_tail];
	#ifdef FLASH_ACTIVE
		_read_lifepo_current_batch_unsafe(*lifepo_current, true);
	#else
		_read_lifepo_current_batch_unsafe(*lifepo_current, false);
	#endif
	if (add_to_sum) sum_piecewise_uint8(sums_struct->lifepo_current_data_sums, *lifepo_current, 4);

	lifepo_volts_batch* lifepo_volts = &burst_struct->lifepo_volts_data[*data_arrays_tail];
	_read_lifepo_volts_batch_unsafe(*lifepo_volts);
	if (add_to_sum) sum_piecewise_uint8(sums_struct->lifepo_volts_data_sums, *lifepo_volts, 4);

	led_current_batch* led_current = &burst_struct->led_current_data[*data_arrays_tail];
	#ifdef FLASH_ACTIVE
		_read_led_current_batch_unsafe(*led_current, true);
	#else 
		_read_led_current_batch_unsafe(*led_current, false);
	#endif
	if (add_to_sum) sum_piecewise_uint8(sums_struct->led_current_data_sums, *led_current, 4);

	gyro_batch* gyro = &burst_struct->gyro_data[*data_arrays_tail];
	_read_gyro_batch_unsafe(*gyro);
	
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
