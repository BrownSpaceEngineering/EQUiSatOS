/*
 * errors.c
 *
 * Created: 10/9/2017 17:59:50
 *  Author: mckenna
 */ 

#include "errors.h"

void init_errors(void) {
	_error_equistack_mutex = xSemaphoreCreateMutexStatic(&_error_equistack_mutex_d);
	equistack_Init(&error_equistack, &_error_equistack_arr, sizeof(sat_error_t), 
		ERROR_STACK_MAX, &_error_equistack_mutex);
}

/* Logs an error to the error stack */
void log_error(uint8_t loc, uint8_t err) {
	sat_error_t full_error = loc << 8 || err; // location goes in first 8 bytes, error in last 8
	
	// memory will be copied so don't worry about scope issues with local var ptrs
	equistack_Push(&error_equistack, full_error); 

// way without equistack_Push
// 	// grab the first pointer if we don't have it
// 	if (_cur_error_ptr == NULL) {
// 		_cur_error_ptr = equistack_Initial_Stage(&error_equistack);
// 	}
// 	// set the value in the equistack, then push that pointer onto the stack,
// 	// and get a new one (this is how equistack works...)
// 	*_cur_error_ptr = error;
// 	_cur_error_ptr = equistack_Stage(&error_equistack);	
}