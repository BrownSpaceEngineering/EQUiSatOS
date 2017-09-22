/*
 * init_rtos_tasks.h
 *
 * Created: 10/4/2016 8:47:48 PM
 *  Author: mckenna
 */ 

#ifndef INIT_RTOS_TASKS_H
#define INIT_RTOS_TASKS_H

#include "../rtos_tasks/rtos_tasks.h"
#include "processor_drivers\USART_Commands.h"
#include "stacks/equistack.h"
#include "stacks/package_transmission.h"

#include "struct_tests.h"
#include "task_testing.h"

/* Global satellite state */
int8_t CurrentState;

/* Task handles for starting and stopping */
TaskHandle_t watchdog_task_handle; // Should we have this?
TaskHandle_t antenna_deploy_task_handle;
TaskHandle_t battery_charging_task_handle; // Should we have this?
TaskHandle_t flash_activate_task_handle;
TaskHandle_t transmit_task_handle;

TaskHandle_t current_data_task_handle;
TaskHandle_t current_data_low_power_task_handle;
TaskHandle_t flash_data_task_handle;
TaskHandle_t transmit_data_task_handle;
TaskHandle_t attitude_data_task_handle;

/* List (series of bits) of whether a given task is RESUMING from suspension.
 * NOTE: a bit does NOT indicate the task is suspended. The time sequence is this:
 * 
 *				|---suspended----|       | task reports unsuspended
 * task bit:  0   0   0   0   0    1   1   0   0   0   ...
 * 
 * This was put here to allow tasks to immediately write old data to their
 * equistacks when resuming from suspend.
 * NOTE: We WILL merge this with the watchdog check in / check out indicators.
 */
uint8_t TaskSuspendedStates;

void run_rtos(void);

void set_state_hello_world(void);
void set_state_idle(void);
void set_state_low_power(void);


/* Required functions for FreeRTOS 9 static allocation */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
									StackType_t **ppxIdleTaskStackBuffer,
									uint32_t *pulIdleTaskStackSize );
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
									 StackType_t **ppxTimerTaskStackBuffer,
									 uint32_t *pulTimerTaskStackSize );
#endif