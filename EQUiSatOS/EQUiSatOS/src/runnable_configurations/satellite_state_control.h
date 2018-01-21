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

void run_rtos(void);

void init_task_state(task_type_t task);

/* Satellite State Constructs */
enum task_state {
	T_STATE_SUSPENDED = false,	// MUST be suspended
	T_STATE_RUNNING = true,		// MUST be running
	T_STATE_ANY					// can be either, not controlled in current state
};

typedef struct task_states {
	uint8_t states[NUM_TASKS];
} task_states;

// Defined task state sets - order must match enum in rtos_tasks_config.h:
//														WATCHDOG,		STATE,				ANTENNA,			BAT,				TRANS,				FLASH,				IDLE,  				LOWP,				ATTI,				PERSIST
#define INITIAL_TASK_STATES				((task_states){T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_SUSPENDED,	T_STATE_RUNNING,	T_STATE_SUSPENDED,	T_STATE_SUSPENDED,	T_STATE_SUSPENDED,	T_STATE_SUSPENDED,	T_STATE_RUNNING,    T_STATE_RUNNING})
#define ANTENNA_DEPLOY_TASK_STATES		((task_states){T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_SUSPENDED,	T_STATE_SUSPENDED,	T_STATE_SUSPENDED,	T_STATE_SUSPENDED,	T_STATE_RUNNING,    T_STATE_RUNNING})
#define HELLO_WORLD_TASK_STATES			((task_states){T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_ANY,		T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_SUSPENDED,	T_STATE_RUNNING,	T_STATE_SUSPENDED,	T_STATE_RUNNING,    T_STATE_RUNNING})
#define HELLO_WORLD_LOW_POWER_TASK_STATES ((task_states){T_STATE_RUNNING,T_STATE_RUNNING,	T_STATE_SUSPENDED,	T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_SUSPENDED,	T_STATE_SUSPENDED,	T_STATE_RUNNING,	T_STATE_SUSPENDED, 	T_STATE_RUNNING})
#define IDLE_NO_FLASH_TASK_STATES 		HELLO_WORLD_TASK_STATES
#define IDLE_FLASH_TASK_STATES 			((task_states){T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_ANY,		T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_SUSPENDED,	T_STATE_RUNNING,    T_STATE_RUNNING})
#define LOW_POWER_TASK_STATES 			HELLO_WORLD_LOW_POWER_TASK_STATES
#define RIP_TASK_STATES 				IDLE_NO_FLASH_TASK_STATES
// vector of radio states       RLIIHHAI (must be in REVERSE order of states in rtos_task_config.h - see above)
const uint16_t RADIO_STATES = 0b10110100;

/* State functions */
sat_state_t get_sat_state(void);
bool set_sat_state(sat_state_t state);
task_states get_sat_task_states(void);
void suspend_antenna_deploy(void); // interface for ONLY antenna deploy task
bool check_task_state_consistency(void);

/* TEMPORARY GLOBAL SET STATE FUNCTIONS FOR TESTING - DONT YOU DARE USE THESE */
void set_all_task_states(task_states states, sat_state_t state);
void task_suspend(task_type_t task_id);
void task_resume(task_type_t task_id);

#endif
