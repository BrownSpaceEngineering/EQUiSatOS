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

#include "struct_tests.h"
#include "task_testing.h"

void run_rtos(void);

void init_task_state(task_type_t task);

/* Satellite State Constructs */
enum {
	T_STATE_SUSPENDED = false,	// MUST be suspended
	T_STATE_RUNNING = true,		// MUST be running
	T_STATE_ANY					// can be either, not controlled in current state
};

struct task_states {
	// watchdog task is not state controlled
	// state handling task is not state controlled
	uint8_t battery_charging_task_state : 2;
	uint8_t antenna_deploy_task_state : 2;
	uint8_t idle_data_task_state : 2;
	uint8_t flash_activate_task_state : 2;
	uint8_t transmit_task_state : 2;
	uint8_t attitude_data_task_state : 2;
	uint8_t low_power_data_task_state : 2;
	uint8_t persistent_data_backup_task_state : 2;
};

//															   BAT,				  ANTENNA,			 IDLE,				FLASH,			   TRANS,			  ATTI,				 LOWP
#define INITIAL_TASK_STATES					((struct task_states) {T_STATE_RUNNING,   T_STATE_SUSPENDED, T_STATE_SUSPENDED, T_STATE_SUSPENDED, T_STATE_SUSPENDED, T_STATE_RUNNING,   T_STATE_SUSPENDED})
#define ANTENNA_DEPLOY_TASK_STATES			((struct task_states) {T_STATE_RUNNING,   T_STATE_ANY,		 T_STATE_SUSPENDED, T_STATE_SUSPENDED, T_STATE_SUSPENDED, T_STATE_RUNNING,   T_STATE_SUSPENDED})
#define HELLO_WORLD_TASK_STATES				((struct task_states) {T_STATE_RUNNING,	  T_STATE_ANY,		 T_STATE_RUNNING,	T_STATE_SUSPENDED, T_STATE_RUNNING,	  T_STATE_RUNNING,   T_STATE_SUSPENDED})
#define HELLO_WORLD_LOW_POWER_TASK_STATES	((struct task_states) {T_STATE_RUNNING,	  T_STATE_SUSPENDED, T_STATE_SUSPENDED,	T_STATE_SUSPENDED, T_STATE_RUNNING,	  T_STATE_SUSPENDED, T_STATE_RUNNING})
#define IDLE_NO_FLASH_TASK_STATES			HELLO_WORLD_TASK_STATES
#define IDLE_FLASH_TASK_STATES				((struct task_states) {T_STATE_RUNNING,	  T_STATE_ANY,		 T_STATE_RUNNING,	T_STATE_RUNNING,   T_STATE_RUNNING,	  T_STATE_RUNNING,   T_STATE_SUSPENDED})
#define LOW_POWER_TASK_STATES				HELLO_WORLD_LOW_POWER_TASK_STATES
#define RIP_TASK_STATES						IDLE_NO_FLASH_TASK_STATES

/* State functions */
sat_state_t get_sat_state(void);
bool set_sat_state(sat_state_t state);
struct task_states get_sat_task_states(void);
void suspend_antenna_deploy(void); // interface for ONLY antenna deploy task
bool check_task_state_consistency(void);

/* TEMPORARY GLOBAL SET STATE FUNCTIONS FOR TESTING - DONT YOU DARE USE THESE */
void set_task_states(struct task_states states, sat_state_t state);
void task_suspend(task_type_t task_id);
void task_resume(task_type_t task_id);

#endif
