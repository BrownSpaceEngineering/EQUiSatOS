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

// wrapper for reading from MRAM; error checking
bool storage_read_bytes_unsafe(uint8_t *data, int num_bytes, uint16_t address) {
	uint8_t data_1[num_bytes];
	bool success1 = log_if_error(ELOC_MRAM1_READ,
		mram_read_bytes(&spi_master_instance, &mram1_slave, data_1, num_bytes, address),
		true); // priority

	bool success2 = log_if_error(ELOC_MRAM2_READ,
		mram_read_bytes(&spi_master_instance, &mram2_slave, data, num_bytes, address),
		true); // priority

	// if both failed, we're quite screwed
	// (note errors would've been logged)
	if (!success1 && !success2) {
		return false;
	}
	// if ONLY one MRAM failed (XOR), take the other one's data
	else if (success1 ^ success2) {
		if (success1) { // implies not success2
			// if #2 was the one that failed, use the data from
			// #1 (otherwise the data from #2 would already be there)
			memcpy(data, data_1, num_bytes);
		}
		return true; // things are theoretically okay

	} else {
		// if everything went okay, check that the data matches
		if (memcmp(data_1, data, num_bytes) != 0) {
			// if it fails, for now just take the data from #2 (in data now)
			return true;
		}
		return true;
	}
}

// wrapper for writing to MRAM; error checking
bool storage_write_bytes_unsafe(uint8_t *data, int num_bytes, uint16_t address) {
	bool success1 = log_if_error(ELOC_MRAM1_WRITE,
		mram_write_bytes(&spi_master_instance, &mram1_slave, data, num_bytes, address),
		true); // priority
	return success1 && log_if_error(ELOC_MRAM2_WRITE,
		mram_write_bytes(&spi_master_instance, &mram2_slave, data, num_bytes, address),
		true); // priority
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
		#else
			storage_read_bytes_unsafe(1, (uint8_t*) &cached_state.secs_since_launch,	4,		STORAGE_SECS_SINCE_LAUNCH_ADDR);
			storage_read_bytes_unsafe(1, &cached_state.reboot_count,					1,		STORAGE_REBOOT_CNT_ADDR);
			storage_read_bytes_unsafe(1, (uint8_t*) &cached_state.sat_state,			1,		STORAGE_SAT_STATE_ADDR);
			storage_read_bytes_unsafe(1, (uint8_t*) &cached_state.sat_event_history,	1,		STORAGE_SAT_EVENT_HIST_ADDR);
		#endif
	} else {
		log_error(ELOC_MRAM_READ, ECODE_SPI_MUTEX_TIMEOUT, false);
	}
	xSemaphoreGive(mram_spi_mutex);
}

void increment_reboot_count(void) {
	cached_state.reboot_count++;
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
	return result;
}

/* must be called with cache mutex locked to be accurate, not throw errors, etc.
 (allows us not to need recursive mutexes when called in this file)
 NOTE: this does protect the SPI lines (takes that mutex) - it's NOT unsafe in that sense */
void write_state_to_storage(void) {
	// keep track of the old timestamp value in case the write fails and we have to reset
	uint32_t prev_cached_secs_since_launch = cached_state.secs_since_launch;
	cached_state.secs_since_launch = get_current_timestamp();
	cached_state.sat_state = get_sat_state();
	// reboot count is only incremented on startup and is written through cache
	// sat_event_history is written through when changed

	// (variables for read results)
	uint32_t temp_secs_since_launch;
	uint8_t temp_reboot_count, temp_sat_state;
	satellite_history_batch temp_sat_event_history;

	// set write time right before writing
	uint32_t prev_last_data_write_ms = last_data_write_ms;
	last_data_write_ms = xTaskGetTickCount() / portTICK_PERIOD_MS;

	if (xSemaphoreTake(mram_spi_mutex, MRAM_SPI_MUTEX_WAIT_TIME_TICKS))
	{
		storage_write_bytes_unsafe(1, (uint8_t*) &cached_state.secs_since_launch,	4,		STORAGE_SECS_SINCE_LAUNCH_ADDR);
		storage_write_bytes_unsafe(1, (uint8_t*) &cached_state.reboot_count,		1,		STORAGE_REBOOT_CNT_ADDR);
		storage_write_bytes_unsafe(1, (uint8_t*) &cached_state.sat_state,			1,		STORAGE_SAT_STATE_ADDR);
		storage_write_bytes_unsafe(1, (uint8_t*) &cached_state.sat_event_history,	1,		STORAGE_SAT_EVENT_HIST_ADDR);
		// NOTE: we don't write out the bootloader or program memory hash TODO: do we REALLY not want to write it out?

		// read it right back to confirm validity
		storage_read_bytes_unsafe(1, (uint8_t*) &temp_secs_since_launch,	4,		STORAGE_SECS_SINCE_LAUNCH_ADDR);
		storage_read_bytes_unsafe(1, &temp_reboot_count,					1,		STORAGE_REBOOT_CNT_ADDR);
		storage_read_bytes_unsafe(1, &temp_sat_state,						1,		STORAGE_SAT_STATE_ADDR);
		storage_read_bytes_unsafe(1, (uint8_t*) &temp_sat_event_history,	1,		STORAGE_SAT_EVENT_HIST_ADDR);
	} else {
		log_error(ELOC_MRAM_WRITE, ECODE_SPI_MUTEX_TIMEOUT, false);
	}
	xSemaphoreGive(mram_spi_mutex);

	// log error if the stored data was not consistent with what was just written
	// note we have the mutex so no one should be able to write to these
	// while we were reading / are comparing them
	if (temp_secs_since_launch != cached_state.secs_since_launch ||
		temp_reboot_count != cached_state.reboot_count ||
		temp_sat_state != cached_state.sat_state ||
		!compare_sat_event_history(&temp_sat_event_history, &cached_state.sat_event_history)) {

		log_error(ELOC_CACHED_PERSISTENT_STATE, ECODE_INCONSISTENT_DATA, true);

		// in particular, if it was the secs_since_launch that was inconsistent,
		// go off the old value in the MRAM
		if (temp_secs_since_launch != cached_state.secs_since_launch &&
		temp_secs_since_launch < cached_state.secs_since_launch) {
			last_data_write_ms = prev_last_data_write_ms;
			cached_state.secs_since_launch = prev_cached_secs_since_launch;
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
								uint8_t first_flash) {

	if (antenna_deployed)
		cached_state.sat_event_history.antenna_deployed = antenna_deployed;
	if (lion_1_charged)
		cached_state.sat_event_history.lion_1_charged = lion_1_charged;
	if (lion_2_charged)
		cached_state.sat_event_history.lion_2_charged = lion_2_charged;
	if (lifepo_b1_charged)
		cached_state.sat_event_history.lifepo_b1_charged = lifepo_b1_charged;
	if (lifepo_b2_charged)
		cached_state.sat_event_history.lifepo_b2_charged = lifepo_b2_charged;
	if (first_flash)
		cached_state.sat_event_history.first_flash = first_flash;

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
	return ((xTaskGetTickCount() / portTICK_PERIOD_MS - last_data_write_ms) / 1000)
		 + cache_get_secs_since_launch();
}

/* Current timestamp in ms since boot, with the above described (low) accuracy */
uint64_t get_current_timestamp_ms(void) {
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

/************************************************************************/
/* functions which require reading from MRAM (bypass cache)				*/
/************************************************************************/
void populate_error_stacks(equistack* priority_errors, equistack* normal_errors) {
	if (xSemaphoreTake(mram_spi_mutex, MRAM_SPI_MUTEX_WAIT_TIME_TICKS))
	{
		// read in errors from MRAM
		uint8_t num_stored_priority_errors;
		uint8_t num_stored_normal_errors;
		sat_error_t priority_error_buf[PRIORITY_ERROR_STACK_MAX];
		sat_error_t normal_error_buf[NORMAL_ERROR_STACK_MAX];
		storage_read_bytes_unsafe(1, &num_stored_priority_errors,	1, STORAGE_PRIORITY_ERR_NUM_ADDR);
		storage_read_bytes_unsafe(1, &num_stored_normal_errors,	1, STORAGE_NORMAL_ERR_NUM_ADDR);
		storage_read_bytes_unsafe(1, (uint8_t*) priority_error_buf,
			PRIORITY_ERROR_STACK_MAX * sizeof(sat_error_t), STORAGE_PRIORITY_LIST_ADDR);
		storage_read_bytes_unsafe(1, (uint8_t*) normal_error_buf,
			NORMAL_ERROR_STACK_MAX * sizeof(sat_error_t), STORAGE_NORMAL_LIST_ADDR);

		// read all errors that we have stored in MRAM in
		for (int i = 0; i < num_stored_priority_errors; i++) {
			equistack_Push(priority_errors, &(priority_error_buf[i]));
		}
		for (int i = 0; i < num_stored_normal_errors; i++) {
			equistack_Push(normal_errors, &(normal_error_buf[i]));
		}
	} else {
		log_error(ELOC_MRAM_READ, ECODE_SPI_MUTEX_TIMEOUT, false);
	}
	xSemaphoreGive(mram_spi_mutex);
}

/************************************************************************/
/* Utilities for setting MRAM; used to write initial state              */
/************************************************************************/
void write_custom_state(void) {
	/*** CONFIG ***/
	uint32_t secs_since_launch =				0;
	uint8_t reboot_count =						0;
	sat_state_t sat_state =						INITIAL;
	satellite_history_batch sat_event_history;
	sat_event_history.antenna_deployed =		false;
	sat_event_history.first_flash =				false;
	sat_event_history.lifepo_b1_charged =		false;
	sat_event_history.lifepo_b2_charged =		false;
	sat_event_history.lion_1_charged =			false;
	sat_event_history.lion_2_charged =			false;

	#define NUM_PRIORITY_ERRS	0
	#define NUM_NORMAL_ERRS		0
	const uint8_t num_priority_errs = NUM_PRIORITY_ERRS;
	const uint8_t num_normal_errs = NUM_NORMAL_ERRS;
	sat_error_t priority_errors[NUM_PRIORITY_ERRS];
	sat_error_t normal_errors[NUM_NORMAL_ERRS];
// 	sat_error_t priority_errors[NUM_PRIORITY_ERRS] = {
// 		{10, 12, 13}
// 	};
// 	sat_error_t normal_errors[NUM_NORMAL_ERRS] = {
// 		{10, 20, 40},
// 		{11, 120, 247},
// 		{1, 2, 3},
// 	};

	/*** WRITING ***/

	// set write time right before writing
	last_data_write_ms = xTaskGetTickCount() / portTICK_PERIOD_MS;

	storage_write_bytes_unsafe(1, (uint8_t*) &secs_since_launch,	4,		STORAGE_SECS_SINCE_LAUNCH_ADDR);
	storage_write_bytes_unsafe(1, (uint8_t*) &reboot_count,		1,		STORAGE_REBOOT_CNT_ADDR);
	storage_write_bytes_unsafe(1, (uint8_t*) &sat_state,			1,		STORAGE_SAT_STATE_ADDR);
	storage_write_bytes_unsafe(1, (uint8_t*) &sat_event_history,	1,		STORAGE_SAT_EVENT_HIST_ADDR);
	// TODO: bootloader / program memory hashes

	// write errors
	storage_write_bytes_unsafe(1, (uint8_t*) &num_priority_errs,	1, STORAGE_PRIORITY_ERR_NUM_ADDR);
	storage_write_bytes_unsafe(1, (uint8_t*) &num_normal_errs,		1, STORAGE_NORMAL_ERR_NUM_ADDR);
	if (num_priority_errs > 0)
		storage_write_bytes_unsafe(1, (uint8_t*) priority_errors,
			num_priority_errs * sizeof(sat_error_t), STORAGE_PRIORITY_LIST_ADDR);
	if (num_normal_errs > 0)
		storage_write_bytes_unsafe(1, (uint8_t*) normal_errors,
			num_normal_errs * sizeof(sat_error_t), STORAGE_NORMAL_LIST_ADDR);

	/*** read it right back to confirm validity ***/
	uint32_t temp_secs_since_launch;
	uint8_t temp_reboot_count;
	sat_state_t temp_sat_state;
	satellite_history_batch temp_sat_event_history; // fits in one

	uint8_t temp_num_priority_errs;
	uint8_t temp_num_normal_errs;
	sat_error_t temp_priority_errors[num_priority_errs];
	sat_error_t temp_normal_errors[num_normal_errs];

	storage_read_bytes_unsafe(1, (uint8_t*) &temp_secs_since_launch,	4,		STORAGE_SECS_SINCE_LAUNCH_ADDR);
	storage_read_bytes_unsafe(1, (uint8_t*) &temp_reboot_count,		1,		STORAGE_REBOOT_CNT_ADDR);
	storage_read_bytes_unsafe(1, (uint8_t*) &temp_sat_state,			1,		STORAGE_SAT_STATE_ADDR);
	storage_read_bytes_unsafe(1, (uint8_t*) &temp_sat_event_history,	1,		STORAGE_SAT_EVENT_HIST_ADDR);
	// TODO: bootloader / program memory hashes

	storage_read_bytes_unsafe(1, (uint8_t*) &temp_num_priority_errs,	1, STORAGE_PRIORITY_ERR_NUM_ADDR);
	storage_read_bytes_unsafe(1, (uint8_t*) &temp_num_normal_errs,		1, STORAGE_NORMAL_ERR_NUM_ADDR);

	configASSERT(temp_num_priority_errs == num_priority_errs);
	configASSERT(temp_num_normal_errs == num_normal_errs);

	if (num_priority_errs > 0)
		storage_read_bytes_unsafe(1, (uint8_t*) temp_priority_errors,
			num_priority_errs * sizeof(sat_error_t), STORAGE_PRIORITY_LIST_ADDR);
	if (num_normal_errs > 0)
		storage_read_bytes_unsafe(1, (uint8_t*) temp_normal_errors,
			num_normal_errs * sizeof(sat_error_t), STORAGE_NORMAL_LIST_ADDR);

	/*** CHECKS ***/
	configASSERT(temp_secs_since_launch == secs_since_launch);
	configASSERT(temp_reboot_count == reboot_count);
	configASSERT(temp_sat_state == sat_state);
	configASSERT(compare_sat_event_history(&temp_sat_event_history, &sat_event_history));

	configASSERT(memcmp(priority_errors, temp_priority_errors, num_priority_errs * sizeof(sat_error_t)) == 0);
	configASSERT(memcmp(normal_errors, temp_normal_errors, num_normal_errs  * sizeof(sat_error_t)) == 0);
}
