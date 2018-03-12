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

	ELOC_IR_POS_Y = 					1,
	ELOC_IR_NEG_X = 					2,
	ELOC_IR_NEG_Y = 					3,
	ELOC_IR_POS_X = 						4,
	ELOC_IR_NEG_Z = 					5,
	ELOC_IR_POS_Z = 						6,

	ELOC_PD_POS_Y = 					7,
	ELOC_PD_NEG_X = 					8,
	ELOC_PD_NEG_Y = 					9,
	ELOC_PD_POS_X = 						10,
	ELOC_PD_NEG_Z = 					11,
	ELOC_PD_POS_Z = 						12,

	ELOC_TEMP_LF_1 = 					13,
	ELOC_TEMP_LF_2 = 					14,
	ELOC_TEMP_L_1 = 					15,
	ELOC_TEMP_L_2 = 					16,
	ELOC_TEMP_LED_1 = 					17,
	ELOC_TEMP_LED_2 = 					18,
	ELOC_TEMP_LED_3 = 					19,
	ELOC_TEMP_LED_4 = 					20,

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

	ELOC_AD7991_BBRD = 					41,
	ELOC_AD7991_BBRD_L2_SNS = 			42,
	ELOC_AD7991_BBRD_L1_SNS = 			43,
	ELOC_AD7991_BBRD_L_REF = 			44,
	ELOC_AD7991_BBRD_PANEL_REF = 		45,
	ELOC_AD7991_CBRD = 					46,
	ELOC_AD7991_CBRD_3V6_REF = 			47,
	ELOC_AD7991_CBRD_3V6_SNS =			48,
	ELOC_AD7991_CBRD_5V_REF = 			49,
	ELOC_AD7991_CBRD_3V3_REF = 			50,

	ELOC_TCA = 							51,
	ELOC_CACHED_PERSISTENT_STATE = 		52,
	ELOC_MRAM1_READ = 					53,
	ELOC_MRAM2_READ = 					54,
	ELOC_MRAM_READ =					55,
	ELOC_MRAM1_WRITE = 					56,
	ELOC_MRAM2_WRITE = 					57,
	ELOC_MRAM_WRITE =					58,
	ELOC_5V_REF = 						59,
	ELOC_STATE_HANDLING = 				60,
	ELOC_BAT_CHARGING =					61,
	ELOC_ANTENNA_DEPLOY	=				62,
	ELOC_ERROR_STACK =					63,
	ELOC_WATCHDOG =						64,
	ELOC_IMU_TEMP =						65,
	ELOC_VERIFY_REGS =					66,

	ELOC_IDLE_DATA =					67,
	ELOC_ATTITUDE_DATA =				68,
	ELOC_FLASH =						69, // both flash and flash_cmp (for now)
	ELOC_LOW_POWER_DATA =				70,
	ELOC_EQUISTACK_GET =				71,
	ELOC_EQUISTACK_PUT =				72,

	ELOC_BOOTLOADER =					73,
	ELOC_RTOS =							74,

	ELOC_BAT_L1 =						75,
	ELOC_BAT_L2 =						76,
	ELOC_BAT_LFB1 =						77,
	ELOC_BAT_LFB2 =						78,
	ELOC_BAT_CHARGING_SWITCH_1 =        79,
	ELOC_BAT_CHARGING_SWITCH_2 =        80,
	ELOC_BAT_CHARGING_SWITCH_3 =        81,
	ELOC_BAT_CHARGING_SWITCH_4 =        82,
	ELOC_BAT_CHARGING_SWITCH_5 =        83,
	ELOC_BAT_CHARGING_SWITCH_6 =        84,
	ELOC_BAT_CHARGING_SWITCH_7 =        85,
	ELOC_BAT_CHARGING_SWITCH_8 =        86,
	ELOC_BAT_CHARGING_SWITCH_9 =        87,
	ELOC_IR_POW =						88,
	ELOC_RADIO_KILLTIME =				89
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
	ECODE_OUT_OF_BOUNDS =				29,
	ECODE_SIGNAL_LOST = 				30,

	ECODE_CONFIRM_TIMEOUT = 			31,
	ECODE_INCONSISTENT_DATA = 			32,
	ECODE_UNEXPECTED_CASE = 			33,
	ECODE_WATCHDOG_EARLY_WARNING =		34,
	ECODE_WATCHDOG_RESET =				35,
	ECODE_WATCHDOG_DID_KICK =			36,
	ECODE_EXCESSIVE_SUSPENSION =		37,

	ECODE_CRIT_ACTION_MUTEX_TIMEOUT =	38,
	ECODE_I2C_MUTEX_TIMEOUT =			39,
	ECODE_PROC_ADC_MUTEX_TIMEOUT =		40,
	ECODE_HW_STATE_MUTEX_TIMEOUT =		41,
	ECODE_USART_MUTEX_TIMEOUT =			42,
	ECODE_SPI_MUTEX_TIMEOUT =			43,
	ECODE_BAT_CHARGING_MUTEX_TIMEOUT =  44,
	ECODE_WATCHDOG_MUTEX_TIMEOUT =		45,
	ECODE_EQUISTACK_MUTEX_TIMEOUT =		46,
	ECODE_IRPOW_MUTEX_TIMEOUT =			47,

	ECODE_REWROTE_PROG_MEM =			48,
	ECODE_STACK_OVERFLOW =				49,
	ECODE_DET_ALREADY_HIGH =			50,

	ECODE_BAT_NOT_DISCHARGING =         51,
	ECODE_BAT_NOT_NOT_DISCHARGING =     52,
	ECODE_BAT_NOT_CHARGING =			53,
	ECODE_BAT_NOT_NOT_CHARGING =		54,
	ECODE_BAT_NOT_DISCHARGING_RESTART = 55,
	ECODE_BAT_FAULT =                   56,
	ECODE_NOT_FULL_FOR_WHILE =          57,
	ECODE_LOW_VOLTAGE_FOR_WHILE =       58,
	ECODE_RECOMMISSION =                59,
	ECODE_ALL_SAME_VAL =				60,
	ECODE_CORRUPTED =					61,
	ECODE_INVALID_STATE_CHANGE =		62,
	ECODE_TIMESTAMP_WRAPAROUND =		63,
	ECODE_INCONSISTENT_STATE =			64,
	ECODE_PWM_CUR_LOW_ON_DEPLOY =		65,
	ECODE_PWM_CUR_LOW_ON_MAX_CYCLE =	66,
	ECODE_PWM_CUR_VERY_LOW_ON_DEPLOY =	67,
	ECODE_PWM_CUR_VERY_LOW_ON_MAX_CYCLE = 68,
};

/************************************************************************/
/* ERROR STORAGE / INTERFACES                                           */
/************************************************************************/
#define ERROR_STACK_MAX		51 // including staged
#define PRIORITY_ERROR_IMPORTANCE_TIMEOUT_S		86400 // 1 day

typedef struct {
	uint32_t timestamp;
	uint8_t eloc;
	uint8_t ecode; // top bit is priority of error
} sat_error_t;
#define SAT_ERROR_T_SIZE		6

equistack error_equistack; // of sat_error_t

// static data used inside error equistack
sat_error_t _error_equistack_arr[ERROR_STACK_MAX];
StaticSemaphore_t _error_equistack_mutex_d;
SemaphoreHandle_t _error_equistack_mutex;

void init_errors(void);
uint8_t atmel_to_equi_error(enum status_code sc);
bool is_error(enum status_code sc);
bool log_if_error(uint8_t loc, enum status_code sc, bool priority);
void log_error(uint8_t loc, uint8_t err, bool priority);
void log_error_from_isr(uint8_t loc, uint8_t err, bool priority);
bool is_priority_error(sat_error_t err);
void print_error(enum status_code code);
// defined in rtos_system_test.c
void print_sat_error(sat_error_t* data, int i);
const char* get_eloc_str(sat_error_t* err);
const char* get_ecode_str(sat_error_t* err);

#endif /* ERRORS_H_ */
