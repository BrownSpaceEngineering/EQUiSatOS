/*
 * persistent_storage.c
 *
 * Created: 12/3/2017 23:42:50
 *  Author: mcken
 */ 

#include "persistent_storage.h"

/* mutex for locking the cache on reads / writes - ensures cache is always synced */
StaticSemaphore_t _cache_mutex_d;
SemaphoreHandle_t cache_mutex;

void write_state_to_storage_unsafe(void);
uint32_t get_current_timestamp_safety(bool safe);

/************************************************************************/
/* memory interface / init functions									*/
/************************************************************************/ 

void init_persistent_storage(void) {
	cache_mutex = xSemaphoreCreateMutexStatic(&_cache_mutex_d);
}

/* read state from storage into cache */
void read_state_from_storage(void) {
	xSemaphoreTake(cache_mutex, CACHE_MUTEX_WAIT_TIME_TICKS);
	
	// DEFAULT: TODO
	cached_state.secs_since_launch = 0;
	cached_state.sat_state = INITIAL; // signifies initial boot
	cached_state.reboot_count = 0;
	cached_state.sat_event_history;
	
	// TODO: read all fields from MRAM into cache
	
	
	xSemaphoreGive(cache_mutex);
}

void increment_reboot_count(void) {
	xSemaphoreTake(cache_mutex, CACHE_MUTEX_WAIT_TIME_TICKS);
	cached_state.reboot_count++;
	write_state_to_storage_unsafe(); // have the mutex so is safe
	xSemaphoreGive(cache_mutex);
}

/* without mutexes (allows us not to need recursive mutexes when called in this file) */
void write_state_to_storage_unsafe(void) {
	cached_state.secs_since_launch = get_current_timestamp_safety(false); // unsafe
	cached_state.sat_state = get_sat_state();
	// reboot count is only incremented on startup and is written through cache
	// sat_event_history is written through when changed
	
	// set time right before writing
	last_data_write_ms = xTaskGetTickCount() / portTICK_PERIOD_MS;
	
	
	// TODO: write state to MRAM
}

/* updates state to current satellite state (any that isn't written through the cache)
   and then writes to storage */
void write_state_to_storage(void) {
	xSemaphoreTake(cache_mutex, CACHE_MUTEX_WAIT_TIME_TICKS);
	write_state_to_storage_unsafe();
	xSemaphoreGive(cache_mutex);
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
										
	xSemaphoreTake(cache_mutex, CACHE_MUTEX_WAIT_TIME_TICKS);
	
	if (lion_1_charged)
		cached_state.sat_event_history.antenna_deployed = antenna_deployed;
	if (lion_1_charged)
		cached_state.sat_event_history.lion_1_charged = lion_1_charged;
	if (lion_2_charged)
		cached_state.sat_event_history.lion_2_charged = lion_2_charged;
	if (lifepo_b1_charged)
		cached_state.sat_event_history.lifepo_b1_charged = lifepo_b1_charged;
	if (lifepo_b1_charged)
		cached_state.sat_event_history.lifepo_b2_charged = lifepo_b2_charged;
	if (first_flash)
		cached_state.sat_event_history.first_flash = first_flash;
	
	write_state_to_storage_unsafe(); // we already have the mutex, so its safe
	
	xSemaphoreGive(cache_mutex);
}

/************************************************************************/
/* helper functions using cached state		                            */
/************************************************************************/

/*
 * Current timestamp in seconds since boot, with an accuracy of +/- the
 * data write frequency (a reboot could happen at any point in that period
 * due to a watchdog reset). Segment since reboot is accurate to ms.
 */
uint32_t get_current_timestamp(void) {
	return get_current_timestamp_safety(true);
}
uint32_t get_current_timestamp_safety(bool wait_on_write) {
	return ((xTaskGetTickCount() / portTICK_PERIOD_MS - last_data_write_ms) / 1000) 
		 + cache_get_secs_since_launch(wait_on_write);
}

/* Current timestamp in ms since boot, with the above described (low) accuracy */
uint64_t get_current_timestamp_ms(void) {
	return (xTaskGetTickCount() / portTICK_PERIOD_MS) - last_data_write_ms 
			+ (1000 * cache_get_secs_since_launch(true));
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
bool at_orbit_fraction(uint8_t* prev_orbit_fraction, uint8_t orbit_fraction_denominator) {
	// first, we scale up by the denominator to bring our integer precision up to the 
	// fractional (bucket) size. Thus, we will truncate all bits that determine how
	// far we are inside a fractional bucket, and it will give us only the current one
	// we're in
	// TODO: Will the calculations stay in the 32 bit registers? I.e. will they overflow?
	// (use 64 bit for now to be safe)
	uint64_t cur_orbit_fraction = (get_current_timestamp() * orbit_fraction_denominator) / 
									(ORBITAL_PERIOD_S * orbit_fraction_denominator);
									
	// strictly greater than because we only want this to return true on 
	// a CHANGE, i.e. when the fraction moves from one "bucket" or 
	// fraction component to the next we set prev_orbit_fraction 
	// so that we wait the fractional amount before returning true again
	if (cur_orbit_fraction > *prev_orbit_fraction) {
		*prev_orbit_fraction = cur_orbit_fraction;
		return true;
	}
	return false;
}

/************************************************************************/
// functions to get components of cached state   
// NOTE: use of mutexes here is not to prevent race conditions, etc., but
// to ensure the final read value is the most recent value and matches
// what is in MRAM, etc.	                   
// The wait_on_write parameter can be used to configure these mutexes, 
// which will essentially halt any reads if we're in the process
// of writing to the MRAM
/************************************************************************/

uint32_t cache_get_secs_since_launch(bool wait_on_write) {
	uint32_t secs_since_launch;
	if (wait_on_write) xSemaphoreTake(cache_mutex, CACHE_MUTEX_WAIT_TIME_TICKS);
	secs_since_launch = cached_state.secs_since_launch;
	if (wait_on_write) xSemaphoreGive(cache_mutex);
	return secs_since_launch;
}

uint8_t cache_get_reboot_count(bool wait_on_write) {
	uint8_t reboot_count;
	if (wait_on_write) xSemaphoreTake(cache_mutex, CACHE_MUTEX_WAIT_TIME_TICKS);
	reboot_count = cached_state.reboot_count;
	if (wait_on_write) xSemaphoreGive(cache_mutex);
	return reboot_count;
}

/* returns satellite state at last reboot */
sat_state_t cache_get_sat_state(bool wait_on_write) {
	sat_state_t sat_state;
	if (wait_on_write) xSemaphoreTake(cache_mutex, CACHE_MUTEX_WAIT_TIME_TICKS);
	sat_state = cached_state.sat_state;
	if (wait_on_write) xSemaphoreGive(cache_mutex);
	return sat_state;
}

satellite_history_batch* cache_get_sat_event_history(bool wait_on_write) {
	satellite_history_batch* sat_event_history;
	xSemaphoreTake(cache_mutex, CACHE_MUTEX_WAIT_TIME_TICKS);
	sat_event_history = &(cached_state.sat_event_history);
	xSemaphoreGive(cache_mutex);
	return sat_event_history;
}

/************************************************************************/
/* functions which require reading from MRAM (bypass cache)				*/
/************************************************************************/
void populate_error_stacks(equistack* priority_errors, equistack* normal_errors) {
	xSemaphoreTake(cache_mutex, CACHE_MUTEX_WAIT_TIME_TICKS);
	// read errors from MRAM until we fill up both stacks (write their MAX size)
	xSemaphoreGive(cache_mutex);
}
