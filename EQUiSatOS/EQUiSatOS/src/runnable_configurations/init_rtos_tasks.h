/*
 * init_rtos_tasks.h
 *
 * Created: 10/4/2016 8:47:48 PM
 *  Author: mckenna
 */ 

#include "rtos_task_frequencies.h"
#include "rtos_tasks.h"

/* Global task frequencies to allow for changing them in different states */
int taskFrequencies[NUM_TASKS]; 

void runit();

void set_state_idle();
void set_state_flash();
void set_state_low_power();