/*
 * init_rtos_tasks.h
 *
 * Created: 10/4/2016 8:47:48 PM
 *  Author: mckenna
 */

#ifndef SATELLITE_STATE_CONTROL_H
#define SATELLITE_STATE_CONTROL_H

#include "rtos_tasks/rtos_tasks.h"
#include "data_handling/equistack.h"

#include "testing_tasks.h"
#include "testing_functions/struct_tests.h"
#include "testing_functions/sat_data_tests.h"
#include "../testing_functions/os_system_tests.h"
#include "antenna_pwm.h"
#include "../errors.h"

/************************************************************************/
/* Satellite state constructs                                           */
/************************************************************************/
// array wrapped in struct to allow easy copying specification in #defines
typedef struct task_states {
	bool states[NUM_TASKS];
} task_states;

// (atomic) field to notify watchdog early warning interrupt of RTOS being
// ready so it can start logging data if it occurs
// NOTE: these are not bools out of concurrency concerns
uint8_t rtos_ready;
uint8_t got_early_warning_callback_in_boot;

/************************************************************************/
/* Defined task state sets - order must match enum in rtos_tasks_config.h: */
/************************************************************************/
//													WDOG,  STATE,	(ant),	BAT,	TRANS,	FLASH,	IDLE,  	LOWP,	ATTI,	PERSIST
#define INITIAL_TASK_STATES			((task_states){{true,	true,	true,	true,	false,	false,	false,	false,	true,   true}})
#define ANTENNA_DEPLOY_TASK_STATES	((task_states){{true,	true,	true,	true,	false,	false,	false,	false,	true,   true}})
#define HELLO_WORLD_TASK_STATES		((task_states){{true,	true,	true,	true,	true,	false,	true,	false,	true,   true}})
#define IDLE_NO_FLASH_TASK_STATES	HELLO_WORLD_TASK_STATES
#define IDLE_FLASH_TASK_STATES 		((task_states){{true,	true,	true,	true,	true,	true,	true,	false,	true,   true}})
#define LOW_POWER_TASK_STATES 		((task_states){{true,	true,	true,	true,	true,	false,	false,	true,	false, 	true}})
// **see .c file for ir power states**

// duration to wait to get each of ALL the mutexes in sequence (LONG because we really need this)
#define TASK_STATE_CHANGE_MUTEX_WAIT_TIME_TICKS		(4000 / portTICK_PERIOD_MS)
#define TASK_STATE_CHANGE_MUTEX_TAKE_RETRIES		10
#define SEMAPHORE_EMPTY_POLL_TIME_TICKS				10 // poll a lot

/************************************************************************/
/* global hardware states												
   (used primarily to know what currents to expect, etc.)		
   		
   This mutex is take briefly by any hardware changing state,
   such that any section of code that must be run with a consistent hardware
   state can take the hardware state mutex and be assured of the state in 
   this struct.
   
   Places where we need to be sure of the state in this struct
   (other places use other methods, namely the lower-level peripheral mutexes):
	   1. Validation of regulator voltages
	   2. Measuring battery currents									*/
/************************************************************************/ 
typedef enum {
	HW_OFF = false,
	HW_ON = true,
	HW_TRANSITIONING,
} hw_state_t;

typedef enum {
	RADIO_OFF = false,
	RADIO_IDLE = true,
	RADIO_TRANSMITTING,
	RADIO_OFF_IDLE_TRANSITION,
	RADIO_IDLE_TRANS_TRANSITION
} radio_state_t;

struct hw_states {
	/* locked by peripheral mutexes - mainly done to simplify function arguments */
	hw_state_t rail_5v_enabled : 2;
	/* locked by hardware state mutex */
	radio_state_t radio_state : 3;
	bool antenna_deploying : 1;
	/* note: flashing state is passed down */
	
	// time that radio will be done coming down from transmitting (other times are handled with delays)
	TickType_t radio_trans_done_target_time;
	
	// time that IR power will be fully on; used so that no one uses IR power
	// in an (upwards) transition state (nothing needs to know explicitly when it's off)
	TickType_t ir_target_on_time;

	// time that IR power will be fully off; used so that no one tries to confirm
	// the value until it's good
	TickType_t rail_5v_target_off_time;
};
#define HARDWARE_STATE_MUTEX_WAIT_TIME_TICKS	(1000 / portTICK_PERIOD_MS)
StaticSemaphore_t _hardware_state_mutex_d;
SemaphoreHandle_t hardware_state_mutex;

/************************************************************************/
/* Mutex for major satellite operations that should be mutually exclusive*/
/************************************************************************/
#define CRITICAL_MUTEX_WAIT_TIME_TICKS			(10000 / portTICK_PERIOD_MS) // these can take quite a while
StaticSemaphore_t _critical_action_mutex_d;
SemaphoreHandle_t critical_action_mutex;

/************************************************************************/
/* State functions                                                      */
/************************************************************************/
sat_state_t get_sat_state(void);
bool set_sat_state(sat_state_t state);
task_states get_sat_task_states(void);
void task_resume_safe(task_type_t task_id);
bool check_task_state_consistency(void);
bool low_power_active(void);

// hardware-specific functions
struct hw_states* get_hw_states(void);
bool hardware_state_mutex_take(uint8_t eloc);
void hardware_state_mutex_give(void);

void run_rtos(void);
void init_task_state(task_type_t task);

/* TEMPORARY GLOBAL SET STATE FUNCTIONS FOR TESTING - DONT YOU DARE USE THESE */
bool set_sat_state_helper(sat_state_t state);
void set_all_task_states(const task_states states, sat_state_t state, sat_state_t prev_sat_state);
void task_suspend(task_type_t task_id);
void task_resume(task_type_t task_id);

#endif
