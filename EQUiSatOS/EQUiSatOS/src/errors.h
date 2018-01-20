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
 *   - Numbers can't go past 255 so they stay in one byte
 */


#ifndef ERRORS_H_
#define ERRORS_H_

#include "global.h"
#include "data_handling/equistack.h"

/******************** LOCATIONS ********************/
/* Error code that signified there is no error - "null error"; used in transmission */
#define ECODE_NO_ERROR					0

#define ELOC_IR_1						1
#define ELOC_IR_2						2
#define ELOC_IR_3						3
#define ELOC_IR_4						4
#define ELOC_IR_5						5
#define ELOC_IR_6						6

#define ELOC_PD_1						7
#define ELOC_PD_2						8
#define ELOC_PD_3						9
#define ELOC_PD_4						10
#define ELOC_PD_5						11
#define ELOC_PD_6						12

#define ELOC_TEMP_1						13
#define ELOC_TEMP_2						14
#define ELOC_TEMP_3						15
#define ELOC_TEMP_4						16
#define ELOC_TEMP_5						17
#define ELOC_TEMP_6						18
#define ELOC_TEMP_7						19
#define ELOC_TEMP_8						20

#define ELOC_RADIO_TEMP					21

#define ELOC_IMU_ACC					22
#define ELOC_IMU_GYRO					23
#define ELOC_IMU_MAG					24

#define ELOC_LED1SNS					25
#define ELOC_LED2SNS					26
#define ELOC_LED3SNS					27
#define ELOC_LED4SNS					28
#define ELOC_LFB1OSNS					29
#define ELOC_LFB1SNS					30
#define ELOC_LFB2OSNS					31
#define ELOC_LFB2SNS					32
#define ELOC_LF1REF						33
#define ELOC_LF2REF						34
#define ELOC_LF3REF						35
#define ELOC_LF4REF						36
#define ELOC_L1_REF						37
#define ELOC_L2_REF						38

#define ELOC_DET_RTN					39
#define ELOC_RADIO						40

#define ELOC_AD7991_0					41
#define ELOC_AD7991_0_0					42
#define ELOC_AD7991_0_1					43
#define ELOC_AD7991_0_2					44
#define ELOC_AD7991_0_3					45
#define ELOC_AD7991_1					46
#define ELOC_AD7991_1_0					47
#define ELOC_AD7991_1_1					48
#define ELOC_AD7991_1_2					49
#define ELOC_AD7991_1_3					50

#define ELOC_TCA						51
#define ELOC_CACHED_PERSISTENT_STATE	52
#define ELOC_MRAM_READ					53
#define ELOC_MRAM_WRITE					54
#define ELOC_5V_REF						55
#define ELOC_STATE_HANDLING				56
#define BAT_CHARGING            57

/******************** PROBLEM CODES ********************/

/**** ATMEL DEFINED ****/
#define ECODE_OK						0
#define ECODE_VALID_DATA				1
#define ECODE_NO_CHANGE					2
#define ECODE_ABORTED					3
#define ECODE_BUSY						4
#define ECODE_SUSPEND					5
#define ECODE_IO						6
#define ECODE_REQ_FLUSHED				7
#define ECODE_TIMEOUT					8
#define ECODE_BAD_DATA					9
#define ECODE_NOT_FOUND					10
#define ECODE_UNSUPPORTED_DEV			11
#define ECODE_NO_MEMORY					12
#define ECODE_INVALID_ARG				13
#define ECODE_BAD_ADDRESS				14
#define ECODE_BAD_FORMAT				15
#define ECODE_BAD_FRQ					16
#define ECODE_DENIED					17
#define ECODE_ALREADY_INITIALIZED		18
#define ECODE_OVERFLOW					19
#define ECODE_NOT_INITIALIZED			20
#define ECODE_SAMPLERATE_UNAVAILABLE	21
#define ECODE_RESOLUTION_UNAVAILABLE	22
#define ECODE_BAUDRATE_UNAVAILABLE		23
#define ECODE_PACKET_COLLISION			24
#define ECODE_PROTOCOL					25
#define ECODE_PIN_MUX_INVALID			26

/**** CUSTOM ****/
#define ECODE_READING_HIGH				27
#define ECODE_READING_LOW				28
#define ECODE_SIGNAL_LOST				29

#define ECODE_CONFIRM_TIMEOUT			30
#define ECODE_INCONSISTENT_DATA			31
#define ECODE_UNEXPECTED_CASE				32
#define ECODE_ENTER_RIP							33
#define ECODE_UNCERTAIN_RIP					34


/* Error storage and interfaces */
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
