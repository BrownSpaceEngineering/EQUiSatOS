/*
 * persistent_storage.c
 *
 * Created: 12/3/2017 23:42:50
 *  Author: mcken
 */

#include "persistent_storage.h"

/* mutex for locking SPI lines and MRAM drivers */
StaticSemaphore_t _mram_spi_mutex_d;
SemaphoreHandle_t mram_spi_mutex;

// super simple mutex used to sync the two operations required
// to modify the cached data responsible for computing get_current_timestamp()
// (true if fields are being modified, false otherwise)
// note: this only works because it's only being written by one thread at a time
// (except in rare cases), and because read/stores are atomic in single-processor ARM
bool cache_time_fields_minimutex = false;

/* SPI master and slave handles */
struct spi_module spi_master_instance;
struct spi_slave_inst mram1_slave;
struct spi_slave_inst mram2_slave;

void write_state_to_storage(void);
uint32_t get_current_timestamp_safety(bool safe);
uint32_t cache_get_secs_since_launch_safety(bool safe);

/************************************************************************/
/* memory interface / init functions									*/
/************************************************************************/

void init_persistent_storage(void) {
	memset(&cached_state, 0, sizeof(cached_state)); // to avoid undefined behavior if someone accidentally uses it

	mram_spi_mutex = xSemaphoreCreateMutexStatic(&_mram_spi_mutex_d);

	mram_initialize_master(&spi_master_instance, MRAM_SPI_BAUD);
	mram_initialize_slave(&mram1_slave, P_MRAM1_CS);
	mram_initialize_slave(&mram2_slave, P_MRAM2_CS);
}

// wrapper for reading a field from MRAM
// handles RAIDing, error checking and correction, and field duplication
// returns whether accurate data should be expected in data (whether error checks worked out)
bool storage_read_field_unsafe(uint8_t *data, int num_bytes, uint32_t address) {
	// data is used as mram1_data1
	uint8_t mram1_data2[num_bytes];
	uint8_t mram2_data1[num_bytes];
	uint8_t mram2_data2[num_bytes];
	// TODO: ^^^^^ can we find a way to use just two of these 
	
	// read both duplicates from MRAM1
	bool success_mram1_data1 = !log_if_error(ELOC_MRAM1_READ,
		mram_read_bytes(&spi_master_instance, &mram1_slave, data, num_bytes, address),
		true); // priority
	bool success_mram1_data2 = !log_if_error(ELOC_MRAM1_READ,
		mram_read_bytes(&spi_master_instance, &mram1_slave, mram1_data2, num_bytes, 
		address + num_bytes), true); // priority

	bool success_mram2_data1 = !log_if_error(ELOC_MRAM2_READ,
		mram_read_bytes(&spi_master_instance, &mram2_slave, mram2_data1, num_bytes, address),
		true); // priority
	bool success_mram2_data2 = !log_if_error(ELOC_MRAM2_READ,
		mram_read_bytes(&spi_master_instance, &mram2_slave, mram2_data2, num_bytes, 
		address + num_bytes), true); // priority
		
	// helpful constants
	bool success_mram1 = success_mram1_data1 && success_mram1_data2;
	bool success_mram2 = success_mram2_data1 && success_mram2_data2;
	bool mram1_data_matches = memcmp(data,		  mram1_data2, num_bytes) == 0;
	bool mram2_data_matches = memcmp(mram2_data1, mram2_data2, num_bytes) == 0;
	
	// TODO: incorporate status codes BECAUE THEY'LL LIKELY MATCH IF ALL 0xff's!!!!

	/* if only one of the two sets of data matches, return the other one (but log error) 
	   (also require that the status codes from that MRAM were okay, because 0xff's from
	   a bad MRAM or SPI driver would match!) */
	if (mram1_data_matches && !mram2_data_matches && success_mram1) {
		log_error(ELOC_MRAM2_READ, ECODE_INCONSISTENT_DATA, true);
		return success_mram1;
	}
	if (!mram1_data_matches && mram2_data_matches && success_mram2) {
		log_error(ELOC_MRAM1_READ, ECODE_INCONSISTENT_DATA, true);
		// need to copy over (which data # shouldn't matter)
		memcpy(data, mram2_data1, num_bytes);
		return success_mram2;
	}
	
	/* if neither set of data matches, check for a cross-match and use it if available 
	 (also require that status codes are good for reason above) */
	/*
	1_1 _ 2_1
	    X  
	1_2 _ 2_2
	*/
	if (!mram1_data_matches && !mram2_data_matches) {
		log_error(ELOC_MRAM1_READ, ECODE_INCONSISTENT_DATA, true);
		log_error(ELOC_MRAM2_READ, ECODE_INCONSISTENT_DATA, true);
		
		if (memcmp(data, mram2_data1, num_bytes) == 0 
			&& success_mram1_data1 && success_mram2_data1) {
			// return data in data
			return true;
		}
		else if (memcmp(data, mram2_data2, num_bytes) == 0
			&& success_mram1_data1 && success_mram2_data2) {
			// return data in data
			return true;
		}
		else if (memcmp(mram1_data2, mram2_data1, num_bytes) == 0
			&& success_mram1_data2 && success_mram2_data1) {
			memcpy(data, mram1_data2, num_bytes);
			return true;
		}
		else if (memcmp(mram1_data2, mram2_data2, num_bytes) == 0
			&& success_mram1_data2 && success_mram2_data2) {
			memcpy(data, mram1_data2, num_bytes);
			return true;
		} else {
			// we could try and compare data without caring about status codes, 
			// but this case is so unlikely and hard to recover from we determined
			// it's not worth it 
			return false;
		}
	}
	
	/* if both sets of data match, do an additional comparison between them to determine our confidence */
	if (mram1_data_matches && mram2_data_matches) {
		bool mrams_match = memcmp(data, mram2_data1, num_bytes) == 0;
		if (mrams_match) {
			// return data in data
			return true;
		} else {
			log_error(ELOC_MRAM_READ, ECODE_INCONSISTENT_DATA, true);
			// if they aren't matched, check status codes and take the one most likely to be right
			// based on those
			if (success_mram1_data2) {
				memcpy(data, mram1_data2, num_bytes);
			} else if (success_mram2_data1) {
				memcpy(data, mram1_data2, num_bytes);
			} else if (success_mram2_data1) {
				memcpy(data, mram1_data2, num_bytes);
			}
			// otherwise, we'll take the data in data (mram1_data1) no matter what... 
			return false;
		}
	}
}

// wrapper for writing a field to MRAM
// handles RAIDing, error checking, and field duplication
bool storage_write_field_unsafe(uint8_t *data, int num_bytes, uint32_t address) {
	bool success1 = !log_if_error(ELOC_MRAM1_WRITE,
		mram_write_bytes(&spi_master_instance, &mram1_slave, data, num_bytes, address),
			true); // priority
	bool success2 = !log_if_error(ELOC_MRAM1_WRITE,
		mram_write_bytes(&spi_master_instance, &mram1_slave, data, num_bytes, 
			address + num_bytes), true); // priority	
	
	bool success3 = !log_if_error(ELOC_MRAM2_WRITE,
		mram_write_bytes(&spi_master_instance, &mram2_slave, data, num_bytes, address),
			true); // priority
	return success1 && success2 && success3 && !log_if_error(ELOC_MRAM2_WRITE,
		mram_write_bytes(&spi_master_instance, &mram2_slave, data, num_bytes, 
			address + num_bytes), true); // priority
}

/* read state from storage into cache */
void read_state_from_storage(void) {
	if (xSemaphoreTake(mram_spi_mutex, MRAM_SPI_MUTEX_WAIT_TIME_TICKS))
	{
		#ifdef XPLAINED
			// defaults when no MRAM available
			cached_state.secs_since_launch = 0;
			cached_state.sat_state = INITIAL; // signifies initial boot
			cached_state.reboot_count = 0;
			cached_state.sat_event_history;
			cached_state.prog_mem_rewritten = false;
			cached_state.radio_revive_timestamp = 0;
		#else
			storage_read_field_unsafe((uint8_t*) &cached_state.secs_since_launch,	4,		STORAGE_SECS_SINCE_LAUNCH_ADDR);
			storage_read_field_unsafe(&cached_state.reboot_count,					1,		STORAGE_REBOOT_CNT_ADDR);
			storage_read_field_unsafe((uint8_t*) &cached_state.sat_state,			1,		STORAGE_SAT_STATE_ADDR);
			storage_read_field_unsafe((uint8_t*) &cached_state.sat_event_history,	1,		STORAGE_SAT_EVENT_HIST_ADDR);
			storage_read_field_unsafe((uint8_t*) &cached_state.prog_mem_rewritten,	1,		STORAGE_PROG_MEM_REWRITTEN_ADDR);
			storage_read_field_unsafe((uint8_t*) &cached_state.radio_revive_timestamp,	4,	STORAGE_RADIO_REVIVE_TIMESTAMP_ADDR);
		#endif
		
		xSemaphoreGive(mram_spi_mutex);
	} else {
		log_error(ELOC_CACHED_PERSISTENT_STATE, ECODE_SPI_MUTEX_TIMEOUT, true);
	}
}

void increment_reboot_count(void) {
	cached_state.reboot_count++;
	write_state_to_storage();
}

void update_radio_revive_timestamp(uint16_t radio_revive_timestamp) {
	cached_state.radio_revive_timestamp = radio_revive_timestamp;
	write_state_to_storage();
}

// deep comparison of structs because thier bit organization may differ
bool compare_sat_event_history(satellite_history_batch* history1, satellite_history_batch* history2) {
	bool result = true;
	result = result && history1->antenna_deployed == history2->antenna_deployed;
	result = result && history1->first_flash == history2->first_flash;
	result = result && history1->lifepo_b1_charged == history2->lifepo_b1_charged;
	result = result && history1->lifepo_b2_charged == history2->lifepo_b2_charged;
	result = result && history1->lion_1_charged == history2->lion_1_charged;
	result = result && history1->lion_2_charged == history2->lion_2_charged;
	result = result && history1->prog_mem_rewritten == history2->prog_mem_rewritten;
	return result;
}

// writes error stack data to mram, and confirms it was written correctly if told to
bool storage_write_check_errors_unsafe(equistack* stack, bool confirm) {
	uint8_t num_errors = stack->cur_size;
	sat_error_t error_buf[num_errors];
	
	bool got_mutex = true;
	if (!xSemaphoreTake(stack->mutex, (TickType_t) EQUISTACK_MUTEX_WAIT_TIME_TICKS)) {
		// log error, but continue on because we're just reading
		log_error(ELOC_CACHED_PERSISTENT_STATE, ECODE_EQUISTACK_MUTEX_TIMEOUT, true);
		got_mutex = false;
	}
	{
		// populate buffer with current errors (thread-safely)
		for (int i = 0; i < num_errors; i++) {
			sat_error_t* err = (sat_error_t*) equistack_Get_Unsafe(stack, i);
			error_buf[i] = *err;
		}
	}
	if (got_mutex) xSemaphoreGive(stack->mutex);
	
	// write size and error data to storage
	storage_write_field_unsafe(&num_errors,	1, STORAGE_ERR_NUM_ADDR);
	if (num_errors > 0) {
		storage_write_field_unsafe((uint8_t*) error_buf,
			num_errors * sizeof(sat_error_t), STORAGE_ERR_LIST_ADDR);
	}
	
	if (confirm) {
		// check if stored # of errors matches
		uint8_t temp_num_errors;
		storage_read_field_unsafe(&temp_num_errors,	1, STORAGE_ERR_NUM_ADDR);
		if (temp_num_errors != num_errors) {
			return false;
		}
	
		// check if actual stored errors match (if necessary)
		if (num_errors > 0) {
			sat_error_t temp_error_buf[num_errors];
			storage_read_field_unsafe((uint8_t*) temp_error_buf,
				num_errors * sizeof(sat_error_t), STORAGE_ERR_LIST_ADDR);
			if (memcmp(error_buf, temp_error_buf, num_errors * sizeof(sat_error_t)) != 0) {
				return false;
			}
		}
	}
	return true;
}

/* writes cached state to MRAM
 NOTE: this does protect the SPI lines (takes that mutex) - it's NOT unsafe in that sense */
void write_state_to_storage(void) {
	cached_state.sat_state = get_sat_state();
	// reboot count is only incremented on startup and is written through cache
	// sat_event_history is written through when changed

	// (variables for read results)
	uint32_t temp_secs_since_launch;
	uint8_t temp_reboot_count, temp_sat_state;
	satellite_history_batch temp_sat_event_history;
	uint8_t temp_prog_mem_rewritten;
	uint32_t temp_radio_revive_timestamp;
	bool errors_write_confirmed = false;

	// set write time right before writing
	// (keep track of the old timestamp value in case the write fails and we have to reset)
	uint32_t prev_cached_secs_since_launch = cached_state.secs_since_launch;
	uint32_t prev_last_data_write_ms = last_data_write_ms;
	
	// quickly set time fields mutex while we're doing this so timestamp doesn't jump forward
	uint32_t cur_timestamp = get_current_timestamp(); // take before because "takes" mutex
	cache_time_fields_minimutex = true;
	cached_state.secs_since_launch = cur_timestamp;
	last_data_write_ms = xTaskGetTickCount() / portTICK_PERIOD_MS;
	cache_time_fields_minimutex = false;
	
	bool got_mutex = false;

	if (xSemaphoreTake(mram_spi_mutex, MRAM_SPI_MUTEX_WAIT_TIME_TICKS))
	{
		got_mutex = true;
		storage_write_field_unsafe((uint8_t*) &cached_state.secs_since_launch,	4,		STORAGE_SECS_SINCE_LAUNCH_ADDR);
		storage_write_field_unsafe((uint8_t*) &cached_state.reboot_count,		1,		STORAGE_REBOOT_CNT_ADDR);
		storage_write_field_unsafe((uint8_t*) &cached_state.sat_state,			1,		STORAGE_SAT_STATE_ADDR);
		storage_write_field_unsafe((uint8_t*) &cached_state.sat_event_history,	1,		STORAGE_SAT_EVENT_HIST_ADDR);
		storage_write_field_unsafe((uint8_t*) &cached_state.prog_mem_rewritten,	1,		STORAGE_PROG_MEM_REWRITTEN_ADDR);
		storage_write_field_unsafe((uint8_t*) &cached_state.radio_revive_timestamp,4,	STORAGE_RADIO_REVIVE_TIMESTAMP_ADDR);
		errors_write_confirmed = storage_write_check_errors_unsafe(&error_equistack, true);
		// NOTE: we don't write out the bootloader or program memory hash TODO: do we REALLY not want to write it out?

		// read it right back to confirm validity
		storage_read_field_unsafe((uint8_t*) &temp_secs_since_launch,	4,		STORAGE_SECS_SINCE_LAUNCH_ADDR);
		storage_read_field_unsafe(&temp_reboot_count,					1,		STORAGE_REBOOT_CNT_ADDR);
		storage_read_field_unsafe(&temp_sat_state,						1,		STORAGE_SAT_STATE_ADDR);
		storage_read_field_unsafe((uint8_t*) &temp_sat_event_history,	1,		STORAGE_SAT_EVENT_HIST_ADDR);
		storage_read_field_unsafe(&temp_prog_mem_rewritten,				1,		STORAGE_PROG_MEM_REWRITTEN_ADDR);
		storage_read_field_unsafe((uint8_t*) &temp_radio_revive_timestamp,	4,	STORAGE_RADIO_REVIVE_TIMESTAMP_ADDR); // TODO: wrong size, how do we do this?
		
		xSemaphoreGive(mram_spi_mutex);
	} else {
		log_error(ELOC_CACHED_PERSISTENT_STATE, ECODE_SPI_MUTEX_TIMEOUT, true);
	}
	
	// skip the check if we didn't get the mutex, and reset last data write to old MRAM's
	if (!got_mutex) {
		// grab time field mutex
		cache_time_fields_minimutex = true;
		last_data_write_ms = prev_last_data_write_ms;
		cached_state.secs_since_launch = prev_cached_secs_since_launch;
		cache_time_fields_minimutex = false;
		return;
	}

	// log error if the stored data was not consistent with what was just written
	// note we have the mutex so no one should be able to write to these
	// while we were reading / are comparing them
	if (temp_secs_since_launch != cached_state.secs_since_launch 
		|| temp_reboot_count != cached_state.reboot_count 
		|| temp_sat_state != cached_state.sat_state 
		|| !compare_sat_event_history(&temp_sat_event_history, &cached_state.sat_event_history) 
		|| temp_prog_mem_rewritten != cached_state.prog_mem_rewritten
		|| temp_radio_revive_timestamp != cached_state.radio_revive_timestamp
		|| !errors_write_confirmed) {

		log_error(ELOC_CACHED_PERSISTENT_STATE, ECODE_INCONSISTENT_DATA, true);

		// in particular, if it was the secs_since_launch that was inconsistent,
		// go off the old value in the MRAM
		if (temp_secs_since_launch != cached_state.secs_since_launch &&
		temp_secs_since_launch < cached_state.secs_since_launch) { // TODO: why the greater than condition??
			// grab time field mutex
			cache_time_fields_minimutex = true;
			last_data_write_ms = prev_last_data_write_ms;
			cached_state.secs_since_launch = prev_cached_secs_since_launch;
			cache_time_fields_minimutex = false;
		}
	}
}

/* Writes cached state to MRAM, but doesn't confirm it was correct. 
   Can also be used from an ISR if from_isr is true */
void write_state_to_storage_emergency(bool from_isr) {
	bool got_mutex;
	if (from_isr) {
		got_mutex = xSemaphoreTakeFromISR(mram_spi_mutex, NULL);
	} else {
		got_mutex = xSemaphoreTake(mram_spi_mutex, MRAM_SPI_MUTEX_WAIT_TIME_TICKS);
	}
	
	if (got_mutex)
	{
		uint32_t cur_timestamp = get_current_timestamp(); // take before because "takes" mutex
		cache_time_fields_minimutex = true;
		cached_state.secs_since_launch = cur_timestamp;
		last_data_write_ms = xTaskGetTickCount() / portTICK_PERIOD_MS;
		cache_time_fields_minimutex = false;
		
		storage_write_field_unsafe((uint8_t*) &cached_state.secs_since_launch,	4,		STORAGE_SECS_SINCE_LAUNCH_ADDR);
		storage_write_field_unsafe((uint8_t*) &cached_state.reboot_count,		1,		STORAGE_REBOOT_CNT_ADDR);
		storage_write_field_unsafe((uint8_t*) &cached_state.sat_state,			1,		STORAGE_SAT_STATE_ADDR);
		storage_write_field_unsafe((uint8_t*) &cached_state.sat_event_history,	1,		STORAGE_SAT_EVENT_HIST_ADDR);
		storage_write_field_unsafe((uint8_t*) &cached_state.prog_mem_rewritten,	1,		STORAGE_PROG_MEM_REWRITTEN_ADDR);
		storage_write_field_unsafe((uint8_t*) &cached_state.radio_revive_timestamp,4,	STORAGE_RADIO_REVIVE_TIMESTAMP_ADDR);
		storage_write_check_errors_unsafe(&error_equistack, false);

		if (from_isr) {
			xSemaphoreGiveFromISR(mram_spi_mutex, NULL);
		} else {
			xSemaphoreGive(mram_spi_mutex);
		}
	}
}

/* Updates the sat_event_history if the given value is true, but ONLY
   sets them to TRUE, not to FALSE; if the passed in value is FALSE,
   the original value (TRUE or FALSE) is retained. */
void update_sat_event_history(uint8_t antenna_deployed,
								uint8_t lion_1_charged,
								uint8_t lion_2_charged,
								uint8_t lifepo_b1_charged,
								uint8_t lifepo_b2_charged,
								uint8_t first_flash,
								uint8_t prog_mem_rewritten) {

	if (antenna_deployed)
		cached_state.sat_event_history.antenna_deployed = true;
	if (lion_1_charged)
		cached_state.sat_event_history.lion_1_charged = true;
	if (lion_2_charged)
		cached_state.sat_event_history.lion_2_charged = true;
	if (lifepo_b1_charged)
		cached_state.sat_event_history.lifepo_b1_charged = true;
	if (lifepo_b2_charged)
		cached_state.sat_event_history.lifepo_b2_charged = true;
	if (first_flash)
		cached_state.sat_event_history.first_flash = true;
	if (prog_mem_rewritten)
		cached_state.sat_event_history.prog_mem_rewritten = true;

	write_state_to_storage();
}

/************************************************************************/
/* helper functions using cached state		                            */
/************************************************************************/

/*
 * Current timestamp in seconds since boot, with an accuracy of +/- the
 * data write task frequency (a reboot could happen at any point in that period
 * due to a watchdog reset). Segment since reboot is accurate to ms.
 */
uint32_t get_current_timestamp(void) {
	while (cache_time_fields_minimutex) {}; // wait on time field mutex
	return ((xTaskGetTickCount() / portTICK_PERIOD_MS - last_data_write_ms) / 1000)
		 + cache_get_secs_since_launch();
}

/* Current timestamp in ms since boot, with the above described (low) accuracy */
uint64_t get_current_timestamp_ms(void) {
	while (cache_time_fields_minimutex) {}; // wait on time field mutex
	return (xTaskGetTickCount() / portTICK_PERIOD_MS) - last_data_write_ms
			+ (1000 * cache_get_secs_since_launch());
}

/* returns truncated number or orbits since first boot */
uint16_t get_orbits_since_launch(void) {
	return get_current_timestamp() / ORBITAL_PERIOD_S;
}

/**
 * Returns whether we're currently at or above
 * (*prev_orbit_fraction / orbit_fraction_denominator) percent through an orbit,
 * where prev_orbit_fraction is the last known orbit fraction (set by this function)
 * and 1 / orbit_fraction_denominator is a fraction ("bucket") to divide an orbit by such that
 * this function will return true after each such fraction of orbital time passes.
 * This function is designed specifically to be used to time actions according
 * to fractions of the current orbit, and ensures this function will return true
 * orbit_fraction_denominator times during an orbit as long as it is called at least
 * that many times during the orbit.
 *
 * Test cases (with implicit argument of current orbit percentage):
 *		at_orbit_fraction(0, 2, .1) = false
 *		at_orbit_fraction(0, 2, .1) = false
 */
bool passed_orbit_fraction(uint8_t* prev_orbit_fraction, uint8_t orbit_fraction_denominator) {
	#ifdef TESTING_SPEEDUP
		return true;
	#else
		// TODO: Does this work??
		// first, we scale up by the denominator to bring our integer precision up to the
		// fractional (bucket) size. Thus, we will truncate all bits that determine how
		// far we are inside a fractional bucket, and it will give us only the current one
		// we're in
		// TODO: Will the calculations stay in the 32 bit registers? I.e. will they overflow?
		// (use 64 bit for now to be safe)
		uint64_t cur_orbit_fraction = (get_current_timestamp() * orbit_fraction_denominator) /
										(ORBITAL_PERIOD_S * orbit_fraction_denominator);

		// strictly not equal to (really greater than) because we only want
		// this to return true on a CHANGE,
		// i.e. when the fraction moves from one "bucket" or
		// fraction component to the next we set prev_orbit_fraction
		// so that we wait the fractional amount before returning true again
		if (cur_orbit_fraction != *prev_orbit_fraction) {
			*prev_orbit_fraction = cur_orbit_fraction;
			return true;
		}
		return false;
	#endif
}

/************************************************************************/
// functions to get components of cached state
// NOTE: use of mutexes here is necessary because we're using a single
// MRAM chip which is a single-reader single-writer shared resource.
/************************************************************************/

uint32_t cache_get_secs_since_launch() {
	// note: not necessary to take minimutex because people 
	// don't have access to last_write_time_ms
	return cached_state.secs_since_launch;
}

uint8_t cache_get_reboot_count() {
	return cached_state.reboot_count;
}

/* returns satellite state at last reboot */
sat_state_t cache_get_sat_state() {
	return cached_state.sat_state;
}

satellite_history_batch cache_get_sat_event_history() {
	return cached_state.sat_event_history;
}

bool cache_get_prog_mem_rewritten(void) {
	return cached_state.prog_mem_rewritten;
}

uint16_t cache_get_radio_revive_timestamp(void) {
	return cached_state.radio_revive_timestamp;
}

/************************************************************************/
/* functions which require reading from MRAM (bypass cache)				*/
/************************************************************************/
void populate_error_stacks(equistack* error_stack) {
	if (xSemaphoreTake(mram_spi_mutex, MRAM_SPI_MUTEX_WAIT_TIME_TICKS))
	{
		// read in errors from MRAM
		uint8_t num_stored_errors;
		sat_error_t error_buf[ERROR_STACK_MAX];
		storage_read_field_unsafe(&num_stored_errors,	1, STORAGE_ERR_NUM_ADDR);
		
		// make sure number of errors is in a reasonable bound (note we're using a uint)
		if (num_stored_errors <= ERROR_STACK_MAX) {
			// special case; we can't read in 0 bytes (invalid arg)
			if (num_stored_errors > 0) {
				storage_read_field_unsafe((uint8_t*) error_buf,
					num_stored_errors * sizeof(sat_error_t), STORAGE_ERR_LIST_ADDR);

				// read all errors that we have stored in MRAM in
				for (int i = 0; i < num_stored_errors; i++) {
					equistack_Push(error_stack, &(error_buf[i]));
				}
			}
		} else {
			log_error(ELOC_MRAM_READ, ECODE_OUT_OF_BOUNDS, false);
		}
		
		xSemaphoreGive(mram_spi_mutex);
	} else {
		log_error(ELOC_CACHED_PERSISTENT_STATE, ECODE_SPI_MUTEX_TIMEOUT, true);
	}
}

/************************************************************************/
/* Utility for setting MRAM; used to write initial state				*/
/************************************************************************/
void write_custom_state(void) {
	/*** CONFIG ***/
	uint32_t secs_since_launch =				0;
	uint8_t reboot_count =						0;
	sat_state_t sat_state =						INITIAL;
	satellite_history_batch sat_event_history;
	uint8_t prog_mem_rewritten =				false;
	uint16_t radio_revive_timestamp =				0;
	sat_event_history.antenna_deployed =		false;
	sat_event_history.first_flash =				false;
	sat_event_history.lifepo_b1_charged =		false;
	sat_event_history.lifepo_b2_charged =		false;
	sat_event_history.lion_1_charged =			false;
	sat_event_history.lion_2_charged =			false;
	sat_event_history.prog_mem_rewritten =		false;

	#define NUM_ERRS	0
	const uint8_t num_errs = NUM_ERRS;
	sat_error_t error_buf[NUM_ERRS];
// 	sat_error_t error_buf[NUM_ERRS] = {
// 		{10, 20, 40},
// 		{11, 120, 247},
// 		{1, 2, 3},
// 	};

	/*** WRITING ***/

	// set write time right before writing
	last_data_write_ms = xTaskGetTickCount() / portTICK_PERIOD_MS;

	storage_write_field_unsafe((uint8_t*) &secs_since_launch,	4,		STORAGE_SECS_SINCE_LAUNCH_ADDR);
	storage_write_field_unsafe((uint8_t*) &reboot_count,		1,		STORAGE_REBOOT_CNT_ADDR);
	storage_write_field_unsafe((uint8_t*) &sat_state,			1,		STORAGE_SAT_STATE_ADDR);
	storage_write_field_unsafe((uint8_t*) &sat_event_history,	1,		STORAGE_SAT_EVENT_HIST_ADDR);
	storage_write_field_unsafe((uint8_t*) &prog_mem_rewritten,	1,		STORAGE_PROG_MEM_REWRITTEN_ADDR);
	storage_write_field_unsafe((uint8_t*) &radio_revive_timestamp, 4,	STORAGE_RADIO_REVIVE_TIMESTAMP_ADDR);
	// TODO: bootloader / program memory hashes

	// write errors
	storage_write_field_unsafe((uint8_t*) &num_errs,		1, STORAGE_ERR_NUM_ADDR);
	if (num_errs > 0)
		storage_write_field_unsafe((uint8_t*) error_buf,
			num_errs * sizeof(sat_error_t), STORAGE_ERR_LIST_ADDR);

	/*** read it right back to confirm validity ***/
	uint32_t temp_secs_since_launch;
	uint8_t temp_reboot_count;
	sat_state_t temp_sat_state;
	satellite_history_batch temp_sat_event_history; // fits in one
	uint8_t temp_prog_mem_rewritten;
	uint16_t temp_radio_revive_timestamp;

	uint8_t temp_num_errs;
	sat_error_t temp_error_buf[num_errs];

	storage_read_field_unsafe((uint8_t*) &temp_secs_since_launch,	4,		STORAGE_SECS_SINCE_LAUNCH_ADDR); // TODO: wrong size, how do we do this?
	storage_read_field_unsafe((uint8_t*) &temp_reboot_count,		1,		STORAGE_REBOOT_CNT_ADDR);
	storage_read_field_unsafe((uint8_t*) &temp_sat_state,			1,		STORAGE_SAT_STATE_ADDR);
	storage_read_field_unsafe((uint8_t*) &temp_sat_event_history,	1,		STORAGE_SAT_EVENT_HIST_ADDR);
	storage_read_field_unsafe((uint8_t*) &temp_prog_mem_rewritten,	1,		STORAGE_PROG_MEM_REWRITTEN_ADDR);
	storage_read_field_unsafe((uint8_t*) &temp_radio_revive_timestamp,	4,	STORAGE_RADIO_REVIVE_TIMESTAMP_ADDR); // TODO: wrong size, how do we do this?
	// TODO: bootloader / program memory hashes

	storage_read_field_unsafe((uint8_t*) &temp_num_errs,	1, STORAGE_ERR_NUM_ADDR);
	configASSERT(temp_num_errs == num_errs);
	if (num_errs > 0)
		storage_read_field_unsafe((uint8_t*) temp_error_buf,
			num_errs * sizeof(sat_error_t), STORAGE_ERR_LIST_ADDR);

	/*** CHECKS ***/
	configASSERT(temp_secs_since_launch == secs_since_launch);
	configASSERT(temp_reboot_count == reboot_count);
	configASSERT(temp_sat_state == sat_state);
	configASSERT(compare_sat_event_history(&temp_sat_event_history, &sat_event_history));
	configASSERT(temp_prog_mem_rewritten == prog_mem_rewritten);
	configASSERT(temp_radio_revive_timestamp == radio_revive_timestamp);

	configASSERT(memcmp(error_buf, temp_error_buf, num_errs * sizeof(sat_error_t)) == 0);
}

// writes the currently-loaded program memory (live) into the MRAM using a buffered copy
// to use this:
// 1) Make sure you've set up the PROG_MEM_START_ADDR using .text=<addr> in Linker Memory settings
// 2) Set this function to run where you want it (your last change to the code), build it,
//    and copy the size of that binary to PROG_MEM_SIZE (without making any other changes)
// 3) Rebuild and run the code.
void write_cur_prog_mem_to_mram(void) {
	print("Started writing program memory to MRAM...");
	uint8_t confirm_buf[PROG_MEM_COPY_BUF_SIZE];
	
	size_t num_copied = 0;
	uint32_t flash_addr = PROG_MEM_START_ADDR;
	uint32_t mram_addr = STORAGE_PROG_MEM_REWRITTEN_ADDR;

	while (num_copied < PROG_MEM_SIZE) {
		size_t buf_size = min(PROG_MEM_SIZE - num_copied, PROG_MEM_COPY_BUF_SIZE);
		
		// write this buffer section directly from the flash (program memory) address space into both MRAMs
		mram_write_bytes(&spi_master_instance, &mram1_slave, (uint8_t*) flash_addr, buf_size, mram_addr);
		mram_write_bytes(&spi_master_instance, &mram2_slave, (uint8_t*) flash_addr, buf_size, mram_addr);
		
		// checks to confirm it matches program memory
		mram_read_bytes(&spi_master_instance, &mram1_slave, confirm_buf, buf_size, mram_addr);
		configASSERT(memcmp((uint8_t*) flash_addr, confirm_buf, buf_size) == 0);
		mram_write_bytes(&spi_master_instance, &mram2_slave, confirm_buf, buf_size, mram_addr);
		configASSERT(memcmp((uint8_t*) flash_addr, confirm_buf, buf_size) == 0);
		
		num_copied += buf_size;
		mram_addr += buf_size;
		flash_addr += buf_size;
	}
	print("Done writing program memory to MRAM.");
}
