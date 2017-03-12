/*
 * init_rtos_tasks.h
 *
 * Created: 10/4/2016 8:47:48 PM
 *  Author: mckenna
 */ 

#ifndef INIT_RTOS_TASKS_H
#define INIT_RTOS_TASKS_H

#include "rtos_tasks.h"
#include "rtos_task_frequencies.h"
#include "struct_tests.h"
#include "stacks/equistack.h"

// /* Global task frequencies to allow for changing them in different states */
// We shouldn't need to change more than a few of these... it might be better to just use variables
// int taskFrequencies[6]; //NUM_TASKS - DONT KNOW WHY THIS DOESNT WORK

/* Global satellite state - is this the best way to do this? */
int8_t CurrentState;
int8_t CurrentLoggingState;

void runit_2(void);

/* Task handles for starting and stopping */
TaskHandle_t watchdog_task_handle; // Should we have this?
TaskHandle_t antenna_deploy_task_handle;
TaskHandle_t battery_charging_task_handle; // Should we have this?
TaskHandle_t flash_activate_task_handle;
TaskHandle_t transmit_task_handle;
TaskHandle_t current_data_task_handle;
TaskHandle_t current_data_low_power_task_handle;
TaskHandle_t attitude_data_task_handle;

// TODO: Do we need to change states?
void set_state_hello_world(void);
void set_state_idle(void);
void set_state_low_power(void);

#endif