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
#define STORAGE_PROG_MEM_REWRITTEN_ADDR		0x1016 // TODO
#define STORAGE_RADIO_KILL_DURATION_ADDR	0x1017 // TODO
#define STORAGE_BOOTLOADER_HASH_ADDR		0x0017
#define STORAGE_PROG_MEMORY_HASH_ADDR		0x0080
#define STORAGE_PRIORITY_ERR_NUM_ADDR		0x0117
#define STORAGE_PRIORITY_LIST_ADDR			0x0118
#define STORAGE_NORMAL_ERR_NUM_ADDR			0x0136
#define STORAGE_NORMAL_LIST_ADDR			0x0137

// note: this is the NUMBER of stored errors; the bytes taken up is this times sizeof(sat_error_t)
#define MAX_STORED_ERRORS					PRIORITY_ERROR_STACK_MAX + NORMAL_ERROR_STACK_MAX // TODO: would be nice if we could store more than equstack size
#define ORBITAL_PERIOD_S					5580 // s; 93 mins
#define MRAM_SPI_MUTEX_WAIT_TIME_TICKS		((TickType_t) 1000 / portTICK_PERIOD_MS) // ms


/************************************************************************/
/* STATE CACHE                                                          */
/* NOTE: this cached state is configured to match the ACTUAL state that 
 the satellite code EXPECTS is stored in the MRAM. This is a tradeoff;
 it means that this state may not persist if something is going wrong
 with the MRAM, so the satellite (and those on the ground) may be 
 'deceived' by the impression that this state will persist. However,
 the important thing is that they will NOT be deceived about the 
 satellite state. In essence, the cache will represent what has actually
 happened, and we just hope like heck that the MRAM will reflect that,
 instead of holding the MRAM as the ground truth. 
 In sum, this is why we ignore concurrency issues with an updated cache
 being readable before the state could be written, etc.					*/
/************************************************************************/
struct persistent_data {
	uint32_t secs_since_launch; // note: most recent one stored in MRAM, not current timestamp
	uint8_t reboot_count;
	sat_state_t sat_state; // most recent known state
	satellite_history_batch sat_event_history;
	uint8_t prog_mem_rewritten; // actually a bool; only written by bootloader (one in sat event history follows that paradigm)
	uint16_t radio_kill_duration;
	
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
void update_radio_kill_duration(uint16_t radio_kill_duration);
void update_sat_event_history(uint8_t antenna_deployed,
								uint8_t lion_1_charged,
								uint8_t lion_2_charged,
								uint8_t lifepo_b1_charged,
								uint8_t lifepo_b2_charged,
								uint8_t first_flash,
								uint8_t prog_mem_rewritten);

/* functions to get components of cached state */
uint32_t					cache_get_secs_since_launch(void);
uint8_t						cache_get_reboot_count(void);
sat_state_t					cache_get_sat_state(void);
satellite_history_batch		cache_get_sat_event_history(void);
bool						cache_get_prog_mem_rewritten(void);
uint16_t					cache_get_radio_kill_duration(void);

/* functions which require reading from MRAM (bypass cache) */
void populate_error_stacks(equistack* priority_errors, equistack* normal_errors);

/* helper functions using cached state */
uint32_t get_current_timestamp(void);
uint64_t get_current_timestamp_ms(void);
uint16_t get_orbits_since_launch(void);
bool passed_orbit_fraction(uint8_t* prev_orbit_fraction, uint8_t orbit_fraction_denominator);

void write_custom_state(void);

#endif