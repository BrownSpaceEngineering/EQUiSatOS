/*
 * persistent_storage.h
 *
 * Created: 11/16/2017 23:39:44
 *  Author: mcken
 */ 

#ifndef PERSISTENT_STORAGE_H
#define PERSISTENT_STORAGE_H

#include <global.h>
#include "Sensor_Structs.h"
#include "equistack.h"

#define ORBITAL_PERIOD_S	5580 // s; 93 mins
#define CACHE_MUTEX_WAIT_TIME_TICKS		((TickType_t) 1000) // ms

/* helper structs */
struct persistent_data {
	uint32_t secs_since_launch;
	uint8_t reboot_count;
	sat_state_t sat_state; // most recent known state
	satellite_history_batch sat_event_history;
	
} cached_state;

// variable to be updated on each data read so we know how current the MRAM
// data is (in particular for computing timestamps) 
// (measured relative to start of current RTOS tick count)
uint32_t last_data_write_ms;

/* memory interface / action functions */
void init_persistent_storage(void);
void read_state_from_storage(void);
void write_state_to_storage(void);
void increment_reboot_count(void);
void update_sat_event_history(uint8_t antenna_deployed,
									uint8_t lion_1_charged,
									uint8_t lion_2_charged,
									uint8_t lifepo_b1_charged,
									uint8_t lifepo_b2_charged,
									uint8_t first_flash);

/* functions to get components of cached state */
uint32_t					cache_get_secs_since_launch(bool wait_on_write);
uint8_t						cache_get_reboot_count(bool wait_on_write);
sat_state_t					cache_get_sat_state(bool wait_on_write);
satellite_history_batch*	cache_get_sat_event_history(bool wait_on_write);

/* functions which require reading from MRAM (bypass cache) */
void populate_error_stacks(equistack* priority_errors, equistack* normal_errors);

/* helper functions using cached state */
uint32_t get_current_timestamp(void);
uint64_t get_current_timestamp_ms(void);
uint16_t get_orbits_since_launch(void);
bool at_orbit_fraction(uint8_t* prev_orbit_fraction, uint8_t orbit_fraction_denominator);

#endif