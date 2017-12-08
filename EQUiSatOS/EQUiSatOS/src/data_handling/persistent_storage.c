/*
 * persistent_storage.c
 *
 * Created: 12/3/2017 23:42:50
 *  Author: mcken
 */ 

#include "persistent_storage.h"

uint32_t get_current_timestamp(void) {
	return (xTaskGetTickCount() / 1000) + get_time_since_launch();
}

uint32_t get_time_since_launch(void) {
	return 0;
}

/* returns truncated number or orbits since first boot */
uint16_t get_orbits_since_launch(void) {
	return get_time_since_launch() / ORBITAL_PERIOD_S;
}

uint8_t get_reboot_count(void) {
	return 0;
}

satellite_history_batch* get_satellite_history() {
	return NULL;
}
