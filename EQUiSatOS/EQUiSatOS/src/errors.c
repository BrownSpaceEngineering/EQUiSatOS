/*
 * errors.c
 *
 * Created: 10/9/2017 17:59:50
 *  Author: mckenna
 */ 

#include "errors.h"

void log_error_stack_error(uint8_t ecode);
void add_error_to_equistack(equistack* stack, sat_error_t* error);
void check_for_bad_errors(sat_error_t* full_error);

void init_errors(void) {
	_error_equistack_mutex = xSemaphoreCreateMutexStatic(&_error_equistack_mutex_d);
	equistack_Init(&error_equistack, &_error_equistack_arr, sizeof(sat_error_t),
		ERROR_STACK_MAX, _error_equistack_mutex);
	configASSERT(sizeof(sat_eloc) == 1);
	configASSERT(sizeof(sat_ecode) == 1);
}

/*
 * returns true if sc is not OK, VALID_DATA, or NO_CHANGE
 */
bool is_error(enum status_code sc) {
	return !(sc == STATUS_OK || sc == STATUS_VALID_DATA || sc == STATUS_NO_CHANGE || sc == STATUS_BUSY);
}

// returns whether the given error's priority bit is set
inline bool is_priority_error(sat_error_t err) {
	return err.ecode >> 7 & 0x1;
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

// logs an error if the given atmel status code is one, and returns whether it was an error
bool log_if_error(sat_eloc loc, enum status_code sc, bool priority) {
	if (is_error(sc)) {
		log_error(loc, atmel_to_equi_error(sc), priority);
		return true;
	}
	return false;
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
void log_error(sat_eloc loc, sat_ecode err, bool priority) {
	configASSERT(err <= 127); // only 7 bits	

	sat_error_t full_error;
	full_error.timestamp = get_current_timestamp(); // time is now
	full_error.eloc = loc;
	full_error.ecode = priority << 7 | (0b01111111 & err); // priority bit at MSB
	
	add_error_to_equistack(&error_equistack, &full_error); 
	
	#ifdef PRINT_NEW_ERRORS
		print_sat_error(&full_error, 0);
	#endif
	
	check_for_bad_errors(&full_error);
}

/* Logs an error to the error stack, noting its timestamp (ISR safe) */
void log_error_from_isr(sat_eloc loc, sat_ecode err, bool priority) {
	configASSERT(err <= 127); // only 7 bits

	sat_error_t full_error;
	full_error.timestamp = get_current_timestamp(); // time is now
	full_error.eloc = loc;
	full_error.ecode = priority << 7 | (0b01111111 & err); // priority bit at MSB

	// in this case, just push it right onto the stack and ignore conventions...
	// don't want to spend to much time on it (or write more ISR alternative functions :P)
	equistack_Push_from_isr(&error_equistack, &full_error);
	
	// don't respond to errors in ISRs
}

/* adds the given error to the given error equistack, in such a way 
   that only two errors will ever be stored during a "period of errors":
   one to mark the start and one to mark the most recent occurrence.
   Dependent on starting from an empty stack. */
void add_error_to_equistack(equistack* stack, sat_error_t* new_error) {
	
	// Note: taking the mutex around this logic both ensures the equistack
	// is in a consistent state, and ensures that the mutex grabs in
	// the "safe" equistack functions don't result in infinite recursion 
	// by logging an error if they can't get a mutex
	bool got_mutex = xSemaphoreTake(stack->mutex, (TickType_t) EQUISTACK_MUTEX_WAIT_TIME_TICKS);
	// would log error if we didn't get it, but we can't!
	{
		sat_error_t* newest_same_error = NULL;
		int num_same_errors = 0;
		for (uint8_t i = 0; i < stack->cur_size; i++) {
			sat_error_t* err = (sat_error_t*) equistack_Get_Unsafe(stack, i);
		
			if (err != NULL &&
				err->eloc == new_error->eloc &&
				err->ecode == new_error->ecode) { // includes priority bit comparison
				num_same_errors++;
			
				// find newest error for later use (there should only be a max of two)
				// (use ">" to favor first (newest) one found)
				if (newest_same_error == NULL ||
					err->timestamp > newest_same_error->timestamp) { 
					newest_same_error = err;
				}
			}
		}
	
		if (num_same_errors == 0 || num_same_errors == 1) {
			// if there are no similar errors in the stack, or only one
			// (indicating the (known) start of the period of this error),
			// then we just add the new error to the stack
			// (it will be either the known start or the known "most recent", respectively)
		
			// HOWEVER, we add an additional (unrelated) condition to this:
			// If a normal error being added will overwrite a priority error, only do so if
			// that priority error is past the "importance timeout" (too old to matter)
			// If a priority error is being added, always add it
			if (stack->cur_size == stack->max_size) {
				// we only need to check to determine whether to overwrite with a normal error
				// when the stack is full. We'll only overwrite if we're overwriting a normal
				// error or we're overwriting an OLD priority error.
				sat_error_t* to_overwrite = (sat_error_t*) equistack_Get_From_Bottom_Unsafe(stack, 0);
				if (to_overwrite != NULL
					&& (!is_priority_error(*to_overwrite)  ||
						get_current_timestamp() - to_overwrite->timestamp >= PRIORITY_ERROR_IMPORTANCE_TIMEOUT_S)) {
					// need to have mutex to add, otherwise things may get screwed up
					if (got_mutex) equistack_Push_Unsafe(stack, new_error);
				}
			} else {
				// need to have mutex to add, otherwise things may get screwed up
				if (got_mutex) equistack_Push_Unsafe(stack, new_error);
			}
		
		} else if(num_same_errors == 2) {
			configASSERT(newest_same_error != NULL);
			// if two errors already exist, we just update the timestamp of the newest one to
			// match the error being added
			newest_same_error->timestamp = new_error->timestamp;
		
		} else {
			print("ERROR EQUISTACK NOT FORMATTED AS EXPECTED"); // TODO
			// otherwise, the stack is not formatted as it should be 
			// (though this may be due to an emergency reboot) 
			#ifdef USE_STRICT_ASSERTIONS
				configASSERT(false);
			#endif
		}
	}
	if (got_mutex) xSemaphoreGive(stack->mutex);
}

// removes priority bit from error code
sat_ecode get_ecode(sat_error_t* err) {
	return err->ecode & 0b01111111;
}

// Returns whether the given error code was an i2c device
bool eloc_category_i2c(sat_eloc eloc) {
	switch (eloc) {
		// slide
		case ELOC_IR_POS_Y:
		case ELOC_IR_NEG_X:
		case ELOC_IR_NEG_Y:
		case ELOC_IR_POS_X:
		case ELOC_IR_NEG_Z:
		case ELOC_IR_POS_Z:
		case ELOC_PD_POS_Y:
		case ELOC_PD_NEG_X:
		case ELOC_PD_NEG_Y:
		case ELOC_PD_POS_X:
		case ELOC_PD_NEG_Z:
		case ELOC_PD_POS_Z:
		case ELOC_TEMP_LF_1:
		case ELOC_TEMP_LF_2:
		case ELOC_TEMP_L_1:
		case ELOC_TEMP_L_2:
		case ELOC_TEMP_LED_1:
		case ELOC_TEMP_LED_2:
		case ELOC_TEMP_LED_3:
		case ELOC_TEMP_LED_4:
		case ELOC_IMU_ACC:
		case ELOC_IMU_GYRO:
		case ELOC_IMU_MAG:
		case ELOC_AD7991_BBRD:
		case ELOC_AD7991_BBRD_L2_SNS:
		case ELOC_AD7991_BBRD_L1_SNS:
		case ELOC_AD7991_BBRD_L_REF:
		case ELOC_AD7991_BBRD_PANEL_REF:
		case ELOC_AD7991_CBRD:
		case ELOC_AD7991_CBRD_3V6_REF:
		case ELOC_AD7991_CBRD_3V6_SNS:
		case ELOC_AD7991_CBRD_5V_REF:
		case ELOC_AD7991_CBRD_3V3_REF:
		case ELOC_TCA:
			return true;
		default:
			return false;
	}
}


/* responds to certain errors that may require action */
void check_for_bad_errors(sat_error_t* full_error) {
	/************************************************************************/
	/* TESTING                                                              */
	/************************************************************************/
	enum error_codes actual_code = full_error->ecode & 0b01111111;
	#ifdef USE_STRICT_ASSERTIONS
		configASSERT(actual_code < ECODE_CRIT_ACTION_MUTEX_TIMEOUT || actual_code > ECODE_ALL_MUTEX_TIMEOUT);
		configASSERT(actual_code != ECODE_EXCESSIVE_SUSPENSION);
		configASSERT(actual_code != ECODE_CORRUPTED);
	#endif
	static int a = 1;
	if (actual_code >= ECODE_CRIT_ACTION_MUTEX_TIMEOUT && actual_code <= ECODE_ALL_MUTEX_TIMEOUT) {
		a++;
	}
	if (actual_code == ECODE_READING_HIGH || actual_code == ECODE_READING_LOW) {
		a++;
	}
	if (actual_code == ECODE_BAD_ADDRESS) {
		a++;
	}
	if (actual_code == ECODE_TIMEOUT || actual_code == ECODE_BAD_ADDRESS) {
		//i2c_send_stop();
	}
	
	if (actual_code == ECODE_EXCESSIVE_SUSPENSION) {
		a++;
	}
	if (actual_code == ECODE_INCONSISTENT_STATE) {
		a++;
	}
};