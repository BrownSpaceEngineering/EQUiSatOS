/*
 * Watchdog_Task.h
 *
 * Created: 3/1/2017 9:28:19 PM
 *  Author: jleiken
 */ 

#ifndef WATCHDOG_TASK_H_
#define WATCHDOG_TASK_H_

#include <global.h>
#include "../processor_drivers/Watchdog_Commands.h"
#include "../rtos_tasks/rtos_tasks_config.h"
#include "../rtos_tasks/rtos_tasks.h"

#define WATCHDOG_MUTEX_WAIT_TIME_TICKS ((TickType_t) 500)

// static memory for watchdog task mutex
StaticSemaphore_t _watchdog_task_mutex_d;

void watchdog_init(void);
void watchdog_task(void *pvParameters);
bool watchdog_as_function(void);

void watchdog_mutex_take(void);
void watchdog_mutex_give(void);
void check_in_task_unsafe(task_type_t task_ind);
void report_task_running(task_type_t task_ind); // always safe; uses mutex
void check_out_task_unsafe(task_type_t task_ind);

#endif /* WATCHDOG_TASK_H_ */