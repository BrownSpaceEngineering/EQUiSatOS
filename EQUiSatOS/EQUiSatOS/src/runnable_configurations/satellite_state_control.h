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

typedef enum task_state task_states[NUM_TASKS];

// Defined task state sets - order must match enum in rtos_tasks_config.h:
//													WATCHDOG,			STATE,				ANTENNA,			BAT,				TRANS,				FLASH,				IDLE,  				LOWP,				ATTI,				PERSIST
const task_states INITIAL_TASK_STATES =				{T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_SUSPENDED,	T_STATE_RUNNING,	T_STATE_SUSPENDED,	T_STATE_SUSPENDED,	T_STATE_SUSPENDED,	T_STATE_SUSPENDED,	T_STATE_RUNNING,    T_STATE_RUNNING};
const task_states ANTENNA_DEPLOY_TASK_STATES =		{T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_SUSPENDED,	T_STATE_SUSPENDED,	T_STATE_SUSPENDED,	T_STATE_SUSPENDED,	T_STATE_RUNNING,    T_STATE_RUNNING};
const task_states HELLO_WORLD_TASK_STATES =			{T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_ANY,		T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_SUSPENDED,	T_STATE_RUNNING,	T_STATE_SUSPENDED,	T_STATE_RUNNING,    T_STATE_RUNNING};
const task_states HELLO_WORLD_LOW_POWER_TASK_STATES={T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_SUSPENDED,	T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_SUSPENDED,	T_STATE_SUSPENDED,	T_STATE_RUNNING,	T_STATE_SUSPENDED, 	T_STATE_RUNNING};
const task_states IDLE_NO_FLASH_TASK_STATES =		HELLO_WORLD_TASK_STATES;
const task_states IDLE_FLASH_TASK_STATES =			{T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_ANY,		T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_RUNNING,	T_STATE_SUSPENDED,	T_STATE_RUNNING,    T_STATE_RUNNING};
const task_states LOW_POWER_TASK_STATES =			HELLO_WORLD_LOW_POWER_TASK_STATES;
const task_states RIP_TASK_STATES =					IDLE_NO_FLASH_TASK_STATES;

/* State functions */
sat_state_t get_sat_state(void);
bool set_sat_state(sat_state_t state);
enum task_state* get_sat_task_states(void);
void suspend_antenna_deploy(void); // interface for ONLY antenna deploy task
bool check_task_state_consistency(void);

/* TEMPORARY GLOBAL SET STATE FUNCTIONS FOR TESTING - DONT YOU DARE USE THESE */
void set_all_task_states(const task_states states, sat_state_t state);
void task_suspend(task_type_t task_id);
void task_resume(task_type_t task_id);

#endif
