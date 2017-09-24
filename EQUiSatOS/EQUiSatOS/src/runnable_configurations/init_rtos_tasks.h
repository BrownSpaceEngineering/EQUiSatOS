/*
 * init_rtos_tasks.h
 *
 * Created: 10/4/2016 8:47:48 PM
 *  Author: mckenna
 */ 

#ifndef INIT_RTOS_TASKS_H
#define INIT_RTOS_TASKS_H

#include "./rtos_tasks/rtos_tasks_config.h"
#include "processor_drivers/USART_Commands.h"
#include "stacks/equistack.h"
#include "stacks/package_transmission.h"

#include "struct_tests.h"
#include "task_testing.h"

void run_rtos(void);

/* State setting functions */
void set_state_hello_world(void);
void set_state_idle(void);
void set_state_low_power(void);

#endif