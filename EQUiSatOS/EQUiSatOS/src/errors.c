/*
 * errors.c
 *
 * Created: 10/9/2017 17:59:50
 *  Author: mckenna
 */ 

#include "errors.h"

void init_errors(void) {
	_priority_error_equistack_mutex = xSemaphoreCreateMutexStatic(&_priority_error_equistack_mutex_d);
	equistack_Init(&priority_error_equistack, &_priority_error_equistack_arr, sizeof(sat_error_t),
		PRIORITY_ERROR_STACK_MAX, &_priority_error_equistack_mutex);
	
	_normal_error_equistack_mutex = xSemaphoreCreateMutexStatic(&_normal_error_equistack_mutex_d);
	equistack_Init(&normal_error_equistack, &_normal_error_equistack_arr, sizeof(sat_error_t), 
		NORMAL_ERROR_STACK_MAX, &_normal_error_equistack_mutex);
}

/* Logs an error to the error stack, noting its timestamp */
void log_error(uint8_t loc, uint8_t err, bool priority) {
	configASSERT(err <= 127); // only 7 bits	

	sat_error_t full_error;
	full_error.timestamp = get_rtc_count(); // time is now
	full_error.eloc = loc;
	full_error.ecode = priority << 7 || (0b01111111 & err); // priority bit at MSB
	
	if (priority) {
		// memory will be copied so don't worry about scope issues with local var ptrs
		equistack_Push(&priority_error_equistack, &full_error);
	} else {
		equistack_Push(&normal_error_equistack, &full_error); 
	}
}