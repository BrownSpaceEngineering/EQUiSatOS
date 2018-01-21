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
#include "../processor_drivers/MRAM_Commands.h"

/* addressing constants */
#define STORAGE_SECS_SINCE_LAUNCH_ADDR		0x0010
#define STORAGE_REBOOT_CNT_ADDR				0x0014
#define STORAGE_SAT_STATE_ADDR				0x0015
#define STORAGE_SAT_EVENT_HIST_ADDR			0x0016
#define STORAGE_BOOTLOADER_HASH_ADDR		0x0017
#define STORAGE_PROG_MEMORY_HASH_ADDR		0x0080
#define STORAGE_PRIORITY_ERR_NUM_ADDR		0x0117
#define STORAGE_PRIORITY_LIST_ADDR			0x0118
#define STORAGE_NORMAL_ERR_NUM_ADDR			0x0136
#define STORAGE_NORMAL_LIST_ADDR			0x0137

// note: this is the NUMBER of stored errors; the bytes taken up is this times sizeof(sat_error_t)
#define MAX_STORED_ERRORS					PRIORITY_ERROR_STACK_MAX + NORMAL_ERROR_STACK_MAX // TODO: would be nice if we could store more than equstack size
#define ORBITAL_PERIOD_S					5580 // s; 93 mins
#define CACHE_MUTEX_WAIT_TIME_TICKS			((TickType_t) 1000) // ms

/* helper structs */
struct persistent_data {
	uint32_t secs_since_launch; // note: most recent one stored in MRAM, not current timestamp
	uint8_t reboot_count;
	sat_state_t sat_state; // most recent known state
	satellite_history_batch sat_event_history;
	
} cached_state;

// variable to be updated on each data read so we know how current the MRAM
// data is (only for computing timestamps) 
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

void write_custom_state(void);

#endif