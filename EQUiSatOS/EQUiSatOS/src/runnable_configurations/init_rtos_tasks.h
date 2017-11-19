/*
 * init_rtos_tasks.h
 *
 * Created: 10/4/2016 8:47:48 PM
 *  Author: mckenna
 */

#ifndef INIT_RTOS_TASKS_H
#define INIT_RTOS_TASKS_H

#include "rtos_tasks/rtos_tasks.h"
#include "processor_drivers/USART_Commands.h"
#include "data_handling/equistack.h"

#include "struct_tests.h"
#include "task_testing.h"

void run_rtos(void);

void init_task_state(task_type_t task);
/* State setting functions */
bool set_state(global_state_t state);

#endif
