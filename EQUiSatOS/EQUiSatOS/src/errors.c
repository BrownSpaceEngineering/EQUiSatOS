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
void log_error(uint8_t loc, uint8_t err, bool priority_error) {
	sat_error_t full_error = loc << 8 || err; // location goes in first 8 bytes, error in last 8
	
	// memory will be copied so don't worry about scope issues with local var ptrs
	equistack_Push(&error_equistack, &full_error); 

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

uint8_t atmel_to_equi_error(enum status_code sc) {
	// needing this switch statement makes me really sad but here you have it 
	//   -Jacob
	switch (sc)	{
		case STATUS_OK:
			return ECODE_OK;
		case STATUS_VALID_DATA:
			return ECODE_VALID_DATA;
		case STATUS_NO_CHANGE:
			return ECODE_NO_CHANGE;
		case STATUS_ABORTED:
			return ECODE_ABORTED;
		case STATUS_BUSY:
			return ECODE_BUSY;
		case STATUS_SUSPEND:
			return ECODE_SUSPEND;
		case STATUS_ERR_IO:
			return ECODE_IO;
		case STATUS_ERR_REQ_FLUSHED:
			return ECODE_REQ_FLUSHED;
		case STATUS_ERR_TIMEOUT:
			return ECODE_TIMEOUT;
		case STATUS_ERR_BAD_DATA:
			return ECODE_BAD_DATA;
		case STATUS_ERR_NOT_FOUND:
			return ECODE_NOT_FOUND;
		case STATUS_ERR_UNSUPPORTED_DEV:
			return ECODE_UNSUPPORTED_DEV;
		case STATUS_ERR_NO_MEMORY:
			return ECODE_NO_MEMORY;
		case STATUS_ERR_INVALID_ARG:
			return ECODE_INVALID_ARG;
		case STATUS_ERR_BAD_ADDRESS:
			return ECODE_BAD_ADDRESS;
		case STATUS_ERR_BAD_FORMAT:
			return ECODE_BAD_FORMAT;
		case STATUS_ERR_BAD_FRQ:
			return ECODE_BAD_FRQ;
		case STATUS_ERR_DENIED:
			return ECODE_DENIED;
		case STATUS_ERR_ALREADY_INITIALIZED:
			return ECODE_ALREADY_INITIALIZED;
		case STATUS_ERR_OVERFLOW:
			return ECODE_OVERFLOW;
		case STATUS_ERR_NOT_INITIALIZED:
			return ECODE_NOT_INITIALIZED;
		case STATUS_ERR_SAMPLERATE_UNAVAILABLE:
			return ECODE_SAMPLERATE_UNAVAILABLE;
		case STATUS_ERR_RESOLUTION_UNAVAILABLE:
			return ECODE_RESOLUTION_UNAVAILABLE;
		case STATUS_ERR_BAUDRATE_UNAVAILABLE:
			return ECODE_BAUDRATE_UNAVAILABLE;
		case STATUS_ERR_PACKET_COLLISION:
			return ECODE_PACKET_COLLISION;
		case STATUS_ERR_PROTOCOL:
			return ECODE_PROTOCOL;
		case STATUS_ERR_PIN_MUX_INVALID:
			return ECODE_PIN_MUX_INVALID;
		default:
			return -1;
	}
}