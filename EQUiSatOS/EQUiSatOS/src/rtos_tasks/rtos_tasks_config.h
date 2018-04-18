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
/* General constants                                                    */
/************************************************************************/
#define ORBITAL_PERIOD_S					5580 // s; 93 mins

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
#ifdef RELEASE
	#define TASK_INIT_STACK_SIZE						(1792/sizeof(portSTACK_TYPE))
#else
	#define TASK_INIT_STACK_SIZE						(1536/sizeof(portSTACK_TYPE))
#endif
#define TASK_INIT_PRIORITY							(STATE_HANDLING_PRIORITY)

#ifdef RELEASE
	#define TASK_BATTERY_CHARGING_STACK_SIZE			(1792/sizeof(portSTACK_TYPE))
#else
	#define TASK_BATTERY_CHARGING_STACK_SIZE			(1280/sizeof(portSTACK_TYPE))
#endif
#define TASK_BATTERY_CHARGING_PRIORITY				(ACTION_PRIORITY)

#ifdef RELEASE
	#define TASK_STATE_HANDLING_STACK_SIZE				(1536/sizeof(portSTACK_TYPE))
#else
	#define TASK_STATE_HANDLING_STACK_SIZE				(1024/sizeof(portSTACK_TYPE))
#endif
#define TASK_STATE_HANDLING_PRIORITY				(STATE_HANDLING_PRIORITY)

#ifdef RELEASE
	#define TASK_ANTENNA_DEPLOY_STACK_SIZE				(1024/sizeof(portSTACK_TYPE))
#else
	#define TASK_ANTENNA_DEPLOY_STACK_SIZE				(1024/sizeof(portSTACK_TYPE))
#endif
#define TASK_ANTENNA_DEPLOY_PRIORITY				(ACTION_PRIORITY)

#ifdef RELEASE
	#define TASK_WATCHDOG_STACK_SIZE					(1280/sizeof(portSTACK_TYPE))
#else
	#ifdef RISKY_STACK_SIZES
	#define TASK_WATCHDOG_STACK_SIZE					(1024/sizeof(portSTACK_TYPE))
	#else
	#define TASK_WATCHDOG_STACK_SIZE					(1024/sizeof(portSTACK_TYPE))
	#endif
#endif
#define TASK_WATCHDOG_PRIORITY						(STATE_HANDLING_PRIORITY)

#ifdef RELEASE
	#define TASK_FLASH_ACTIVATE_STACK_SIZE				(1280/sizeof(portSTACK_TYPE))
#else
	#ifdef RISKY_STACK_SIZES
	#define TASK_FLASH_ACTIVATE_STACK_SIZE				(1024/sizeof(portSTACK_TYPE))
	#else
	#define TASK_FLASH_ACTIVATE_STACK_SIZE				(1280/sizeof(portSTACK_TYPE))
	#endif
#endif
#define TASK_FLASH_ACTIVATE_PRIORITY				(FLASH_PRIORITY)

#ifdef RELEASE
	#define TASK_TRANSMIT_STACK_SIZE					(1536/sizeof(portSTACK_TYPE))
#else
	#define TASK_TRANSMIT_STACK_SIZE					(1024/sizeof(portSTACK_TYPE))
#endif
#define TASK_TRANSMIT_PRIORITY						(ACTION_PRIORITY)

#ifdef RELEASE
	#define TASK_IDLE_DATA_RD_STACK_SIZE				(1536/sizeof(portSTACK_TYPE))
#else
	#define TASK_IDLE_DATA_RD_STACK_SIZE				(1024/sizeof(portSTACK_TYPE))
#endif
#define TASK_IDLE_DATA_RD_PRIORITY					(DATA_READ_PRIORITY)

#ifdef RELEASE
	#define TASK_ATTITUDE_DATA_RD_STACK_SIZE			(1280/sizeof(portSTACK_TYPE))
#else
	#ifdef RISKY_STACK_SIZES
	#define TASK_ATTITUDE_DATA_RD_STACK_SIZE			(600/sizeof(portSTACK_TYPE))
	#else
	#define TASK_ATTITUDE_DATA_RD_STACK_SIZE			(768/sizeof(portSTACK_TYPE)) // note: rather close
	#endif
#endif
#define TASK_ATTITUDE_DATA_DATA_RD_PRIORITY			(DATA_READ_PRIORITY)

#ifdef RELEASE
	#define TASK_LOW_POWER_DATA_RD_STACK_SIZE			(1536/sizeof(portSTACK_TYPE))
#else
	#ifdef RISKY_STACK_SIZES
	#define TASK_LOW_POWER_DATA_RD_STACK_SIZE			(768/sizeof(portSTACK_TYPE))
	#else
	#define TASK_LOW_POWER_DATA_RD_STACK_SIZE			(1280/sizeof(portSTACK_TYPE))
	#endif
#endif
#define TASK_LOW_POWER_DATA_RD_PRIORITY				(DATA_READ_PRIORITY)


#ifdef RELEASE
	#define TASK_PERSISTENT_DATA_BACKUP_STACK_SIZE		(1536/sizeof(portSTACK_TYPE))
#else
	#ifdef RISKY_STACK_SIZES
	#define TASK_PERSISTENT_DATA_BACKUP_STACK_SIZE		(1024/sizeof(portSTACK_TYPE))
	#else
	#define TASK_PERSISTENT_DATA_BACKUP_STACK_SIZE		(1280/sizeof(portSTACK_TYPE))
	#endif
#endif
#define TASK_PERSISTENT_DATA_BACKUP_PRIORITY		(ACTION_PRIORITY)

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
	IDLE_NO_FLASH,
	IDLE_FLASH,
	LOW_POWER,
	NUM_SAT_STATES, // = LOW_POWER + 1
} sat_state_t;

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
#define WATCHDOG_TASK_FREQ_OFFSET				100 // high-freq; should be small
#define STATE_HANDLING_TASK_FREQ_OFFSET			200
#define ANTENNA_DEPLOY_TASK_FREQ_OFFSET			300 // high-freq
#define BATTERY_CHARGING_TASK_FREQ_OFFSET		30000 // should be small
#define TRANSMIT_TASK_FREQ_OFFSET				1500
#define FLASH_ACTIVATE_TASK_FREQ_OFFSET			600
#define IDLE_DATA_TASK_FREQ_OFFSET				10000 // separate from attitude
#define LOW_POWER_DATA_TASK_FREQ_OFFSET			800
#define ATTITUDE_DATA_TASK_FREQ_OFFSET			900
#define PERSISTENT_DATA_BACKUP_TASK_FREQ_OFFSET	500	// high-freq

/* action frequency periods in MS (some that actually have data collection are below) */
#ifndef TESTING_SPEEDUP
#define STATE_HANDLING_TASK_FREQ				(2*60*1000)	// ms
#endif

#define WATCHDOG_TASK_FREQ						1500

#define ANTENNA_DEPLOY_TASK_FREQ				1000
#ifndef TESTING_SPEEDUP
	#define ANTENNA_DEPLOY_TASK_LESS_FREQ			(60*60*1000)	// 60 minutes; don't do it often if it seems to not be working
#endif
	#define ANTENNA_DEPLOY_LI_NOT_CHARGED_WAIT		(30*60*1000) // 30 minutes
	#define ANTENNA_DEPLOY_LF_NOT_CHARGED_WAIT		(60*60*1000) // 60 minutes
	#define ANTENNA_DEPLOY_TASK_WATCHDOG_TIMEOUT	(max(ANTENNA_DEPLOY_LI_NOT_CHARGED_WAIT, max(ANTENNA_DEPLOY_LF_NOT_CHARGED_WAIT, ANTENNA_DEPLOY_TASK_LESS_FREQ)))

#ifndef TESTING_SPEEDUP
#define BATTERY_CHARGING_TASK_FREQ				(9*60*1000)	// 9 minutes; how often run battery charging logic
#endif

#ifndef TESTING_SPEEDUP
#define TRANSMIT_TASK_FREQ						(20*1000)	// 20 secs; how often to transmit
	#define TRANSMIT_TASK_LESS_FREQ					(40*1000) // 40 secs; half as fast in low power (also transmits half the data)
#endif

#ifndef TESTING_SPEEDUP
#define IDLE_DATA_TASK_FREQ						(3*60*1000) // ms
#endif
	#define IDLE_DATA_MAX_READ_TIME					4000 // has to turn on IR power
	#define IDLE_DATA_LOGS_PER_ORBIT				IDLE_DATA_PACKETS // == 7
	#ifndef TESTING_SPEEDUP
	#define IDLE_DATA_LOG_FREQ_S					(ORBITAL_PERIOD_S / IDLE_DATA_LOGS_PER_ORBIT)
	#endif

#ifndef TESTING_SPEEDUP
#define LOW_POWER_DATA_TASK_FREQ				(2*60*1000)
#endif
	#define LOW_POWER_DATA_MAX_READ_TIME			8000 // has to turn on IR power

#ifndef TESTING_SPEEDUP
#define PERSISTENT_DATA_BACKUP_TASK_FREQ		(1*60*1000)
#endif

#ifndef TESTING_SPEEDUP
#define ATTITUDE_DATA_TASK_FREQ					(4*60*1000)
#endif
	#define ATTITUDE_DATA_MAX_READ_TIME				4000
	#define ATTITUDE_DATA_LOGS_PER_ORBIT			ATTITUDE_DATA_PACKETS // == 5
	#ifndef TESTING_SPEEDUP
	#define ATTITUDE_DATA_LOG_FREQ_S				(ORBITAL_PERIOD_S / ATTITUDE_DATA_LOGS_PER_ORBIT)
	#endif
	#define ATTITUDE_DATA_SECOND_SAMPLE_DELAY		500

#ifndef TESTING_SPEEDUP
#define FLASH_ACTIVATE_TASK_FREQ				(1*60*1000)	// 1 minute; how often to flash
#endif
	#define FLASH_DATA_READ_FREQ	20 // ms - this should be longer than 2ms because its used as a buffer for pin transitions
	#define FLASH_DATA_ARR_LEN		7 // implies that the total data read duration is:
										// FLASH_DATA_READ_FREQ * FLASH_DATA_ARR_LEN = 100 ms + time before/after for pre- and post-read
	#define FLASH_CMP_DATA_LOGS_PER_ORBIT			FLASH_CMP_DATA_PACKETS // == 6
	#ifndef TESTING_SPEEDUP
	#define FLASH_CMP_DATA_LOG_FREQ_S				(ORBITAL_PERIOD_S / FLASH_CMP_DATA_LOGS_PER_ORBIT)
	#endif

#ifdef RELEASE
	#define MIN_TIME_IN_INITIAL_S		(30*60)
	#define MIN_TIME_IN_BOOT_S			(26*ORBITAL_PERIOD_S)
#else
	#ifndef TESTING_SPEEDUP
		#define MIN_TIME_IN_INITIAL_S		(30*60)
		#define MIN_TIME_IN_BOOT_S			(30*60)
	#else
		#define MIN_TIME_IN_INITIAL_S		(15*60)
		#define MIN_TIME_IN_BOOT_S			(ORBITAL_PERIOD_S / 2)
	#endif
#endif

// higher-speed overrides
#ifdef TESTING_SPEEDUP
	#define TRANSMIT_TASK_FREQ					(20*1000) // same
		#define TRANSMIT_TASK_LESS_FREQ				(40*1000) // same
	#define STATE_HANDLING_TASK_FREQ			(1*60*1000) // same
	#define ANTENNA_DEPLOY_TASK_LESS_FREQ		1000
	#define IDLE_DATA_TASK_FREQ					15000
		#define IDLE_DATA_LOG_FREQ_S				5
	#define ATTITUDE_DATA_TASK_FREQ				15000
		#define ATTITUDE_DATA_LOG_FREQ_S			10
	#define FLASH_ACTIVATE_TASK_FREQ			(1*60*1000) // same
		#define FLASH_CMP_DATA_LOG_FREQ_S			30
	#define LOW_POWER_DATA_TASK_FREQ			20000
	#define BATTERY_CHARGING_TASK_FREQ			(1*60*1000)
	#define PERSISTENT_DATA_BACKUP_TASK_FREQ	(30*1000)
#endif

#endif
