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

/************************************************************************/
/* Satellite state constructs                                           */
/************************************************************************/
enum task_state {
	T_STATE_SUSPENDED = false,	// MUST be suspended
	T_STATE_RUNNING = true,		// MUST be running
	T_STATE_ANY					// can be either, not controlled in current state
};

// array wrapped in struct to allow easy copying specification in #defines
typedef struct task_states {
	uint8_t states[NUM_TASKS];
} task_states;

/************************************************************************/
/* Defined task state sets - order must match enum in rtos_tasks_config.h: */
/************************************************************************/
//														WATCHDOG,		STATE,				ANTENNA,			BAT,				TRANS,				FLASH,				IDLE,  				LOWP,				ATTI,				PERSIST
#define INITIAL_TASK_STATES				((task_states){T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_SUSPENDED,	T_STATE_RUNNING,	T_STATE_SUSPENDED,	T_STATE_SUSPENDED,	T_STATE_SUSPENDED,	T_STATE_SUSPENDED,	T_STATE_RUNNING,    T_STATE_RUNNING})
#define ANTENNA_DEPLOY_TASK_STATES		((task_states){T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_SUSPENDED,	T_STATE_SUSPENDED,	T_STATE_SUSPENDED,	T_STATE_SUSPENDED,	T_STATE_RUNNING,    T_STATE_RUNNING})
#define HELLO_WORLD_TASK_STATES			((task_states){T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_ANY,		T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_SUSPENDED,	T_STATE_RUNNING,	T_STATE_SUSPENDED,	T_STATE_RUNNING,    T_STATE_RUNNING})
#define HELLO_WORLD_LOW_POWER_TASK_STATES ((task_states){T_STATE_RUNNING,T_STATE_RUNNING,	T_STATE_SUSPENDED,	T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_SUSPENDED,	T_STATE_SUSPENDED,	T_STATE_RUNNING,	T_STATE_SUSPENDED, 	T_STATE_RUNNING})
#define IDLE_NO_FLASH_TASK_STATES 		HELLO_WORLD_TASK_STATES
#define IDLE_FLASH_TASK_STATES 			((task_states){T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_ANY,		T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_SUSPENDED,	T_STATE_RUNNING,    T_STATE_RUNNING})
#define LOW_POWER_TASK_STATES 			HELLO_WORLD_LOW_POWER_TASK_STATES
#define RIP_TASK_STATES 				IDLE_NO_FLASH_TASK_STATES
// **see .c file for radio states**

/************************************************************************/
/* global hardware states (used primarily to know what currents to expect, etc.) */
/************************************************************************/ 
struct hw_states {
	bool rail_5v_enabled : 1;
	bool radio_powered : 1; // if true, both 3V6 regulator and radio power pin are on
	bool radio_transmitting : 1;
	bool antenna_deploying : 1;
	bool flashing : 1;
};
#define HARDWARE_MUTEX_WAIT_TIME_TICKS		2000 // can be used for a while

#define set_hw_state_safe(field, state) { \
	hardware_mutex_take(); \
	get_hw_states()->field = state; \
	hardware_mutex_give(); \
}

/************************************************************************/
/* State functions                                                      */
/************************************************************************/
sat_state_t get_sat_state(void);
bool set_sat_state(sat_state_t state);
task_states get_sat_task_states(void);
void set_task_state_safe(task_type_t task_id, bool run);
bool check_task_state_consistency(void);

// hardware-specific functions
struct hw_states* get_hw_states();
void hardware_mutex_take(void);
void hardware_mutex_give(void);

void run_rtos(void);
void init_task_state(task_type_t task);

/* TEMPORARY GLOBAL SET STATE FUNCTIONS FOR TESTING - DONT YOU DARE USE THESE */
void set_all_task_states(task_states states, sat_state_t state);
void task_suspend(task_type_t task_id);
void task_resume(task_type_t task_id);

#endif
