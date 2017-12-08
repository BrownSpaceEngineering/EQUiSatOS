/*
 * persistent_storage.h
 *
 * Created: 11/16/2017 23:39:44
 *  Author: mcken
 */ 

#ifndef PERSISTENT_STORAGE_H
#define PERSISTENT_STORAGE_H

#include <global.h>

#define ORBITAL_PERIOD_S	5580 // s; 93mins

typedef struct satellite_history_batch
{
	bool lion_1_charged : 1;
	bool lion_2_charged : 1;
	bool lifepo_b1_charged : 1;
	bool lifepo_b2_charged : 1;
	bool first_flash : 1;
} satellite_history_batch;

uint32_t get_current_timestamp(void);
uint32_t get_time_since_launch(void);
uint16_t get_orbits_since_launch(void);
uint8_t get_reboot_count(void);
satellite_history_batch* get_satellite_history(void);

#endif