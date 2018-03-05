/*
 * rtos_tasks_config.h
 *
 * All constants important for the various RTOS_TASKS
 * 
 * Created: 10/2/2016 3:38:22 PM
 *  Author: mckenna
 */ 

#ifndef RTOS_TASKS_CONFIG_H
#define RTOS_TASKS_CONFIG_H

#include <asf.h>
#include <inttypes.h>

/************************************************************************/
/* Classes of Task Priorities                                           */
/************************************************************************/
// lowest priority is at the top
enum {
	DATA_READ_PRIORITY = 1, // above tskIDLE_PRIORITY
	ACTION_PRIORITY,
	FLASH_PRIORITY,
	STATE_HANDLING_PRIORITY
};

/************************************************************************/
/* Task Properties - see below for frequencies							*/
/************************************************************************/
#define TASK_INIT_STACK_SIZE						(1536/sizeof(portSTACK_TYPE))
#define TASK_INIT_PRIORITY							(STATE_HANDLING_PRIORITY)

#define TASK_BATTERY_CHARGING_STACK_SIZE			(768/sizeof(portSTACK_TYPE))
#define TASK_BATTERY_CHARGING_PRIORITY				(ACTION_PRIORITY)

#define TASK_STATE_HANDLING_STACK_SIZE				(512/sizeof(portSTACK_TYPE))
#define TASK_STATE_HANDLING_PRIORITY				(STATE_HANDLING_PRIORITY)

#define TASK_ANTENNA_DEPLOY_STACK_SIZE				(768/sizeof(portSTACK_TYPE))
#define TASK_ANTENNA_DEPLOY_PRIORITY				(ACTION_PRIORITY)

#define TASK_WATCHDOG_STACK_SIZE					(1024/sizeof(portSTACK_TYPE)) // TODO
#define TASK_WATCHDOG_STACK_PRIORITY				(STATE_HANDLING_PRIORITY)

#ifdef RISKY_STACK_SIZES
#define TASK_FLASH_ACTIVATE_STACK_SIZE				(1024/sizeof(portSTACK_TYPE)) 
#else
#define TASK_FLASH_ACTIVATE_STACK_SIZE				(1280/sizeof(portSTACK_TYPE)) 
#endif
#define TASK_FLASH_ACTIVATE_PRIORITY				(FLASH_PRIORITY)

#define TASK_TRANSMIT_STACK_SIZE					(768/sizeof(portSTACK_TYPE))
#define TASK_TRANSMIT_PRIORITY						(ACTION_PRIORITY)

#define TASK_IDLE_DATA_RD_STACK_SIZE				(768/sizeof(portSTACK_TYPE))
#define TASK_IDLE_DATA_RD_PRIORITY					(DATA_READ_PRIORITY)

#define TASK_ATTITUDE_DATA_RD_STACK_SIZE			(768/sizeof(portSTACK_TYPE))
#define TASK_ATTITUDE_DATA_DATA_RD_PRIORITY			(DATA_READ_PRIORITY)

#define TASK_LOW_POWER_DATA_RD_STACK_SIZE			(768/sizeof(portSTACK_TYPE))
#define TASK_LOW_POWER_DATA_RD_PRIORITY				(DATA_READ_PRIORITY)

#ifdef RISKY_STACK_SIZES
#define TASK_PERSISTENT_DATA_BACKUP_STACK_SIZE		(1024/sizeof(portSTACK_TYPE))
#else
#define TASK_PERSISTENT_DATA_BACKUP_STACK_SIZE		(1280/sizeof(portSTACK_TYPE))
#endif
#define TASK_PERSISTENT_DATA_BACKUP_PRIORITY		(DATA_READ_PRIORITY)

/********************************************************************************/
/* Data reading task stack sizes - how many they can store before overwriting	*/
/********************************************************************************/
// we set these (mostly) to be the amount that needs to be transmitted for each data type,
// plus one for the staged pointer
#define IDLE_STACK_MAX					8 // == (IDLE_DATA_PACKETS + 1)
#define LOW_POWER_STACK_MAX				6 // == (LOW_POWER_DATA_PACKETS + 1) 
#define ATTITUDE_STACK_MAX				6 // == (ATTITUDE_DATA_PACKETS + 1)
#define FLASH_STACK_MAX					4 // such that we transmit all we store every minute
#define FLASH_CMP_STACK_MAX				7 // == (FLASH_CMP_DATA_PACKETS + 1)

/************************************************************************/
/* Enum for states that represent changes in which tasks are running	*/
/************************************************************************/
typedef enum
{
	INITIAL,
	ANTENNA_DEPLOY,
	HELLO_WORLD,
	HELLO_WORLD_LOW_POWER,
	IDLE_NO_FLASH,
	IDLE_FLASH,
	LOW_POWER,
	RIP,
	NUM_SAT_STATES, // = RIP + 1
} sat_state_t;

/************************************************************************/
/*  Enum for all types of collected sensor readings						*
 * (for consistency across sensor read functions)						*
 * Based off: https://docs.google.com/a/brown.edu/spreadsheets/d/1sHQNTC5f5sg6j5DD4OKjuQykpIM3z16uetWT9YuB9PQ/edit?usp=sharing
 *	NOTE:																*
 *	If you add/remove a type of collected data, there are several		*
 *	things you must change:												*
 *		- Create a batch type definition								*
 *		- Create the required frequencies								*
 *		- Add a new array of data to ALL of the relevant state structs  *
 *		- Add an add_*_batch_if_ready function						    *
 * NOTE: To move this somewhere, use this regex: (\w*)_DATA, --> $1		*
 ************************************************************************/
typedef enum // TODO: needed?
{
	LION_VOLTS_DATA,
	LION_CURRENT_DATA,
	LED_TEMPS_DATA,
	LIFEPO_CURRENT_DATA,
	LIFEPO_VOLTS_DATA,
	IR_DATA,
	DIODE_DATA,
	BAT_TEMP_DATA,
	IR_TEMPS_DATA,
	RADIO_TEMP_DATA,
	ACCELEROMETER_DATA,
	GYRO_DATA,
	MAGNETOMETER_DATA,
	LED_CURRENT_DATA,
	RADIO_VOLTS_DATA,
	IMU_TEMP_DATA,
	BAT_CHARGE_VOLTS_DATA,
	BAT_CHARGE_DIG_SIGS_DATA,
	DIGITAL_OUT_DATA,
	RAIL_5V_DATA,
	NUM_DATA_TYPES //= RAIL_5V_DATA + 1
} sensor_type_t;

/************************************************************************/
/* enum for all types of data that can be read							*/
/* (all types that will be in the 'data' section of a message packet)   */
/************************************************************************/
typedef enum
{
	IDLE_DATA,
	ATTITUDE_DATA,
	FLASH_DATA,
	FLASH_CMP_DATA,
	LOW_POWER_DATA, // must be the largest value, because we wrap around this list and ignore it
	NUM_MSG_TYPE // = LOW_POWER_DATA + 1
} msg_data_type_t;

/************************************************************************/
/* Enum for all tasks (for array-wise referencing for last_state, etc.) */
/************************************************************************/
typedef enum
{
	WATCHDOG_TASK,
	STATE_HANDLING_TASK,
	ANTENNA_DEPLOY_TASK,
	BATTERY_CHARGING_TASK,
	TRANSMIT_TASK,
	FLASH_ACTIVATE_TASK,
	IDLE_DATA_TASK,
	LOW_POWER_DATA_TASK,
	ATTITUDE_DATA_TASK,
	PERSISTENT_DATA_BACKUP_TASK,
	NUM_TASKS //= PERSISTENT_DATA_BACKUP_TASK + 1
} task_type_t;

/************************************************************************/
/* TASK EXECUTE FREQUENCIES	AND INITIAL OFFSETS							*/
/* These can be complex because of multi-frequency data collection		*/
/* issues... see below for details.				                        */
/************************************************************************/

// these are offsets are added to "start time" to avoid periodic contention of tasks
// offsets should be different especially for tasks that have similar (high) frequencies/priorities
// these are all within 1000ms because that's the highest task frequency
#define WATCHDOG_TASK_FREQ_OFFSET				100 // high-freq 
#define STATE_HANDLING_TASK_FREQ_OFFSET			200
#define ANTENNA_DEPLOY_TASK_FREQ_OFFSET			300 // high-freq
#define BATTERY_CHARGING_TASK_FREQ_OFFSET		400
#define TRANSMIT_TASK_FREQ_OFFSET				500
#define FLASH_ACTIVATE_TASK_FREQ_OFFSET			600 // high-freq
#define IDLE_DATA_TASK_FREQ_OFFSET				700 // high-freq
#define LOW_POWER_DATA_TASK_FREQ_OFFSET			800 // high-freq
#define ATTITUDE_DATA_TASK_FREQ_OFFSET			900 // high-freq
#define PERSISTENT_DATA_BACKUP_TASK_FREQ_OFFSET	150	// high-freq

/* action frequency periods in MS (some that actually have data collection are below) */
#ifndef TESTING_SPEEDUP
#define STATE_HANDLING_TASK_FREQ				60000	// ms
#endif

#define WATCHDOG_TASK_FREQ						1500
														
#define ANTENNA_DEPLOY_TASK_FREQ				1000
#ifndef TESTING_SPEEDUP
#define ANTENNA_DEPLOY_TASK_LESS_FREQ			900000	// 15 minutes; don't do it often if it seems to not be working
#endif
	
#ifndef TESTING_SPEEDUP
#define BATTERY_CHARGING_TASK_FREQ				300000	// 5 minutes; how often run battery charging logic
#endif

#ifndef TESTING_SPEEDUP
#define TRANSMIT_TASK_FREQ						20000	// 20 secs; how often to transmit
#endif
	#define TRANSMIT_TASK_LESS_FREQ					30000 // 30 secs; half as fast in low power
	#define TRANSMIT_TASK_TRANS_MONITOR_FREQ		150		// check period for transmit_task during transmission
	#define TRANSMIT_TASK_CONFIRM_TIMEOUT			2000	// max "transmission time" before timing out confirmation and quit
	#define TRANSMIT_TASK_MSG_REPEATS				2		// number of times to send the same transmission

#ifndef TESTING_SPEEDUP
#define IDLE_DATA_TASK_FREQ						10000 // ms
#endif
	#define IDLE_DATA_MAX_READ_TIME					1000 
	#define IDLE_DATA_LOGS_PER_ORBIT				IDLE_DATA_PACKETS // == 7
	
#ifndef TESTING_SPEEDUP
#define LOW_POWER_DATA_TASK_FREQ				30000
#endif
	#define LOW_POWER_DATA_MAX_READ_TIME			1000

#define PERSISTENT_DATA_BACKUP_TASK_FREQ		10000

/** 
 * NOTE: The idle data collection task doesn't really need these constants;
 * all sensors are being read at the same frequency, unlike below.
 */

#ifndef TESTING_SPEEDUP
#define ATTITUDE_DATA_TASK_FREQ					20000
#endif
	#define ATTITUDE_DATA_MAX_READ_TIME				1000
	#define ATTITUDE_DATA_LOGS_PER_ORBIT			ATTITUDE_DATA_PACKETS // == 5
	#define ATTITUDE_DATA_SECOND_SAMPLE_DELAY		500

#ifndef TESTING_SPEEDUP
#define FLASH_ACTIVATE_TASK_FREQ				60000	// 1 minute; how often to flash
#endif
	#define FLASH_DATA_READ_FREQ	20 // ms - this should be longer than 2ms because its used as a buffer for pin transitions
	#define FLASH_DATA_ARR_LEN		7 // implies that the total data read duration is:
	// FLASH_DATA_READ_FREQ * FLASH_DATA_ARR_LEN = 100 ms + time before/after for pre- and post-read
	#define FLASH_CMP_DATA_LOGS_PER_ORBIT			FLASH_CMP_DATA_PACKETS // == 6

// higher-speed overrides
#ifdef TESTING_SPEEDUP
	#define TRANSMIT_TASK_FREQ				10000
	#define STATE_HANDLING_TASK_FREQ		30000
	#define ANTENNA_DEPLOY_TASK_LESS_FREQ	900000
	#define IDLE_DATA_TASK_FREQ				2500
	#define ATTITUDE_DATA_TASK_FREQ			5000
	#define FLASH_ACTIVATE_TASK_FREQ		15000
	#define LOW_POWER_DATA_TASK_FREQ		7500
	#define BATTERY_CHARGING_TASK_FREQ		15000
#endif

#endif
