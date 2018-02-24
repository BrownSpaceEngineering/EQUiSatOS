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
#define STORAGE_SECS_SINCE_LAUNCH_ADDR		20
#define STORAGE_REBOOT_CNT_ADDR				30
#define STORAGE_SAT_STATE_ADDR				34
#define STORAGE_SAT_EVENT_HIST_ADDR			38
#define STORAGE_PROG_MEM_REWRITTEN_ADDR		42
#define STORAGE_RADIO_REVIVE_TIMESTAMP_ADDR	46
#define STORAGE_BOOTLOADER_HASH_ADDR		56
#define STORAGE_PROG_MEMORY_HASH_ADDR		314
#define STORAGE_ERR_NUM_ADDR				572
#define STORAGE_ERR_LIST_ADDR				576
#define STORAGE_PROG_MEMORY_ADDR			938

// note: this is the NUMBER of stored errors; the bytes taken up is this times sizeof(sat_error_t)
#define MAX_STORED_ERRORS					ERROR_STACK_MAX // TODO: would be nice if we could store more than equstack size
#define ORBITAL_PERIOD_S					5580 // s; 93 mins
#define MRAM_SPI_MUTEX_WAIT_TIME_TICKS		((TickType_t) 1000 / portTICK_PERIOD_MS) // ms

// constants used when copying program memory live to MRAM
#define PROG_MEM_START_ADDR					0x6000	// use default 0x0 OR set with .text=<addr> in Linker Memory settings
#define PROG_MEM_SIZE						81240	// find for latest build in "output"
#define PROG_MEM_COPY_BUF_SIZE				5120	// user-settable (currently matching bootloader batch size)

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
	uint16_t radio_revive_timestamp;
	
} cached_state;

// variable to be updated on each data read so we know how current the MRAM
// data is (only for computing timestamps) 
// (measured relative to start of current RTOS tick count)
uint32_t last_data_write_ms;

/* memory interface / action functions */
void init_persistent_storage(void);
void read_state_from_storage(void);
void write_state_to_storage(void);
void write_state_to_storage_emergency(bool from_isr);
void increment_reboot_count(void);
void update_radio_revive_timestamp(uint16_t radio_revive_timestamp);
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
uint16_t					cache_get_radio_revive_timestamp(void);

/* functions which require reading from MRAM (bypass cache) */
void populate_error_stacks(equistack* error_stack);

/* helper functions using cached state */
uint32_t get_current_timestamp(void);
uint64_t get_current_timestamp_ms(void);
uint16_t get_orbits_since_launch(void);
bool passed_orbit_fraction(uint8_t* prev_orbit_fraction, uint8_t orbit_fraction_denominator);

/* maintenance helpers */
void write_custom_state(void);
void write_cur_prog_mem_to_mram(void);

#endif