/*
 * errors.h
 *
 * Created: 9/10/2017 2:48:18 PM
 *  Author: jleiken
 *
 * Errors should be constructed by combining the location and the code
 * i.e. if TEMP_5 was observing a dangerous temperature, the code would be: 16,27
 * where each number is one byte. It would then be transmitted as 00010000 00011011
 *
 * If you're adding new errors, follow the naming conventions:
 *   - Location byte definitions should start with ELOC_ (error location)
 *   - Error code byte definitions should start with ECODE_ (error definition)
 *   - All caps, words separated by _, preferably tab to the right indentation
 *   - Location numbers can't go past 255 so they stay in one byte
 *	 - Code numbers can't go past 127 so they stay in 7 bits
 */


#ifndef ERRORS_H_
#define ERRORS_H_

#include <global.h>
#include "data_handling/equistack.h"

/******************** LOCATIONS ********************/
/* Error code that signified there is no error - "null error"; used in transmission */
enum error_locations {
	ELOC_NO_ERROR =						0,

	ELOC_IR_1 = 						1,
	ELOC_IR_2 = 						2,
	ELOC_IR_3 = 						3,
	ELOC_IR_4 = 						4,
	ELOC_IR_5 = 						5,
	ELOC_IR_6 = 						6,

	ELOC_PD_1 = 						7,
	ELOC_PD_2 = 						8,
	ELOC_PD_3 = 						9,
	ELOC_PD_4 = 						10,
	ELOC_PD_5 = 						11,
	ELOC_PD_6 = 						12,

	ELOC_TEMP_1 = 						13,
	ELOC_TEMP_2 = 						14,
	ELOC_TEMP_3 = 						15,
	ELOC_TEMP_4 = 						16,
	ELOC_TEMP_5 = 						17,
	ELOC_TEMP_6 = 						18,
	ELOC_TEMP_7 = 						19,
	ELOC_TEMP_8 = 						20,

	ELOC_RADIO_TEMP = 					21,

	ELOC_IMU_ACC = 						22,
	ELOC_IMU_GYRO = 					23,
	ELOC_IMU_MAG = 						24,

	ELOC_LED1SNS = 						25,
	ELOC_LED2SNS = 						26,
	ELOC_LED3SNS = 						27,
	ELOC_LED4SNS = 						28,
	ELOC_LFB1OSNS = 					29,
	ELOC_LFB1SNS = 						30,
	ELOC_LFB2OSNS = 					31,
	ELOC_LFB2SNS = 						32,
	ELOC_LF1REF = 						33,
	ELOC_LF2REF = 						34,
	ELOC_LF3REF = 						35,
	ELOC_LF4REF = 						36,
	ELOC_L1_REF = 						37,
	ELOC_L2_REF = 						38,

	ELOC_DET_RTN = 						39,
	ELOC_RADIO = 						40,

	ELOC_AD7991_0 = 					41,
	ELOC_AD7991_0_0 = 					42,
	ELOC_AD7991_0_1 = 					43,
	ELOC_AD7991_0_2 = 					44,
	ELOC_AD7991_0_3 = 					45,
	ELOC_AD7991_1 = 					46,
	ELOC_AD7991_1_0 = 					47,
	ELOC_AD7991_1_1 = 					48,
	ELOC_AD7991_1_2 = 					49,
	ELOC_AD7991_1_3 = 					50,

	ELOC_TCA = 							51,
	ELOC_CACHED_PERSISTENT_STATE = 		52,
	ELOC_MRAM_READ = 					53,
	ELOC_MRAM_WRITE = 					54,
	ELOC_5V_REF = 						55,
	ELOC_STATE_HANDLING = 				56,
	ELOC_BAT_CHARGING =					57,
	ELOC_ANTENNA_DEPLOY	=				58,
	ELOC_ERROR_STACK =					59,
	ELOC_WATCHDOG =						60,
	ELOC_PROC_TEMP =					61,
	ELOC_VERIFY_REGS =					62,

	ELOC_IDLE_DATA =					63,
	ELOC_ATTITUDE_DATA =				64,
	ELOC_FLASH =						65, // both flash and flash_cmp (for now)
	ELOC_LOW_POWER_DATA =				66,
	ELOC_EQUISTACK_GET =				67,
	ELOC_EQUISTACK_PUT =				68

};

/******************** PROBLEM CODES ********************/
enum error_codes {
/**** ATMEL DEFINED ****/
	ECODE_OK = 							0,
	ECODE_VALID_DATA = 					1,
	ECODE_NO_CHANGE = 					2,
	ECODE_ABORTED = 					3,
	ECODE_BUSY = 						4,
	ECODE_SUSPEND = 					5,
	ECODE_IO = 							6,
	ECODE_REQ_FLUSHED = 				7,
	ECODE_TIMEOUT = 					8,
	ECODE_BAD_DATA = 					9,
	ECODE_NOT_FOUND = 					10,
	ECODE_UNSUPPORTED_DEV = 			11,
	ECODE_NO_MEMORY = 					12,
	ECODE_INVALID_ARG = 				13,
	ECODE_BAD_ADDRESS = 				14,
	ECODE_BAD_FORMAT = 					15,
	ECODE_BAD_FRQ = 					16,
	ECODE_DENIED = 						17,
	ECODE_ALREADY_INITIALIZED = 		18,
	ECODE_OVERFLOW = 					19,
	ECODE_NOT_INITIALIZED = 			20,
	ECODE_SAMPLERATE_UNAVAILABLE = 		21,
	ECODE_RESOLUTION_UNAVAILABLE = 		22,
	ECODE_BAUDRATE_UNAVAILABLE = 		23,
	ECODE_PACKET_COLLISION = 			24,
	ECODE_PROTOCOL = 					25,
	ECODE_PIN_MUX_INVALID = 			26,

	/**** CUSTOM ****/
	ECODE_READING_HIGH = 				27,
	ECODE_READING_LOW = 				28,
	ECODE_SIGNAL_LOST = 				29,

	ECODE_CONFIRM_TIMEOUT = 			30,
	ECODE_INCONSISTENT_DATA = 			31,
	ECODE_UNEXPECTED_CASE = 			32,
	ECODE_ENTER_RIP = 					33,
	ECODE_UNCERTAIN_RIP = 				34,
	ECODE_WATCHDOG_EARLY_WARNING =		35,
	ECODE_WATCHDOG_RESET =				36,
	ECODE_WATCHDOG_DID_KICK =			37,
	ECODE_EXCESSIVE_SUSPENSION =		38,

	ECODE_CRIT_ACTION_MUTEX_TIMEOUT =	39,
	ECODE_I2C_MUTEX_TIMEOUT =			40,
	ECODE_PROC_ADC_MUTEX_TIMEOUT =		41,
	ECODE_HW_STATE_MUTEX_TIMEOUT =		42,
	ECODE_USART_MUTEX_TIMEOUT =			43,
	ECODE_SPI_MUTEX_TIMEOUT =			44,
	ECODE_BAT_CHARGING_MUTEX_TIMEOUT =  45,
	ECODE_WATCHDOG_MUTEX_TIMEOUT =		46,
	ECODE_EQUISTACK_MUTEX_TIMEOUT =		47
};

/************************************************************************/
/* ERROR STORAGE / INTERFACES                                           */
/************************************************************************/
#define PRIORITY_ERROR_STACK_MAX		30
#define NORMAL_ERROR_STACK_MAX			20

typedef struct {
	uint32_t timestamp;
	uint8_t eloc;
	uint8_t ecode; // top bit is priority of error
} sat_error_t;

equistack priority_error_equistack; // of sat_error_t
equistack normal_error_equistack; // of sat_error_t

// static data used inside error equistack
sat_error_t _priority_error_equistack_arr[PRIORITY_ERROR_STACK_MAX];
StaticSemaphore_t _priority_error_equistack_mutex_d;
SemaphoreHandle_t _priority_error_equistack_mutex;
sat_error_t _normal_error_equistack_arr[NORMAL_ERROR_STACK_MAX];
StaticSemaphore_t _normal_error_equistack_mutex_d;
SemaphoreHandle_t _normal_error_equistack_mutex;

void init_errors(void);
uint8_t atmel_to_equi_error(enum status_code sc);
bool is_error(enum status_code sc);
bool log_if_error(uint8_t loc, enum status_code sc, bool priority);
void log_error(uint8_t loc, uint8_t err, bool priority);
bool is_priority_error(sat_error_t err);
void print_error(enum status_code code);

#endif /* ERRORS_H_ */
