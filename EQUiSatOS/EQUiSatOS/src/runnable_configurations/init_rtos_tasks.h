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
//#include "radio/Stacks/EquiStacks.h"

/* Global task frequencies to allow for changing them in different states */
int taskFrequencies[6]; //NUM_TASKS - DONT KNOW WHY THIS DOESNT WORK

/* Global satellite state - is this the best way to do this? */
int CurrentState;

void runit_2();

/* Task handles for starting and stopping */
TaskHandle_t idle_task_handle;
TaskHandle_t flash_task_handle;
TaskHandle_t boot_task_handle;
TaskHandle_t low_power_task_handle;

void set_state_idle();
void set_state_flash();
//void set_state_boot();
void set_state_low_power();

#endif