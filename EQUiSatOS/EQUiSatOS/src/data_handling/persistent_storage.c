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

uint8_t get_reboot_count(void) {
	return 0;
}

satellite_history_batch* get_satellite_history() {
	return NULL;
}
