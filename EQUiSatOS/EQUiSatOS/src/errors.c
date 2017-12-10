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

/*
 * returns true if sc is not OK, VALID_DATA, or NO_CHANGE
 */
bool is_error(enum status_code sc) {
	return !(sc == STATUS_OK || sc == STATUS_VALID_DATA || sc == STATUS_NO_CHANGE || sc == STATUS_BUSY);
}

void print_error(enum status_code code){
	switch(code){
		
		case STATUS_OK:
		print("Status OK\n");
		break;
		
		case STATUS_ERR_BAD_ADDRESS:
		print("Status error, bad address\n");
		break;
		
		case STATUS_ERR_BAD_DATA:
		print("Status error, bad data\n");
		break;
		
		case STATUS_ERR_BAD_FORMAT:
		print("Status error, bad format\n");
		break;
		
		case STATUS_BUSY:
		print("Status busy\n");
		break;
		
		case STATUS_ERR_PACKET_COLLISION:
		print("Status error, packet collision\n");
		break;
		
		default:
		print("Status unknown error \n");
	}
	
}

void log_if_error(uint8_t loc, enum status_code sc, bool priority) {
	if (is_error(sc)) {
		print_error(sc);
		log_error(loc, atmel_to_equi_error(sc), priority);
	}
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