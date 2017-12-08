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

/* State functions */
sat_state_t get_sat_state(void);
bool set_sat_state(sat_state_t state);
void suspend_antenna_deploy(void); // interface for ONLY antenna deploy task

/* TEMPORARY GLOBAL SET STATE FUNCTIONS - DONT YOU DARE USE THESE */
void set_state_initial(void);
void set_state_antenna_deploy(void);
void set_state_hello_world(void);
void set_state_idle_no_flash(void);
void set_states_of_idle_no_flash(void);
void set_state_idle_flash(void);
void set_state_low_power(void);
void set_state_rip(void);
void task_suspend(task_type_t task_id);
void task_resume(task_type_t task_id);

#endif
