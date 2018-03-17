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
#include "../data_handling/persistent_storage.h"

#define WATCHDOG_MUTEX_WAIT_TIME_TICKS ((TickType_t) 500)

#ifdef TESTING_SPEEDUP
	#define WATCHDOG_TASK_TIMEOUT_BUFFER 10000
#else
	#define WATCHDOG_TASK_TIMEOUT_BUFFER (1000*60*5)
#endif

// static memory for watchdog task mutex
// This MUST be taken when using check_in_task_unsafe and check_out_take_unsafe
StaticSemaphore_t _watchdog_task_mutex_d;
SemaphoreHandle_t watchdog_mutex;

void init_watchdog_clock(void);
void init_watchdog_task(void);
bool watchdog_as_function(void);

void check_in_task_unsafe(task_type_t task_ind);
void report_task_running(task_type_t task_ind); // always safe; uses mutex
void check_out_task_unsafe(task_type_t task_ind);
void watchdog_early_warning_callback(void);

// testing functions
bool _get_task_checked_in(task_type_t task);
uint32_t _get_task_checked_in_time(task_type_t task);

#endif /* WATCHDOG_TASK_H_ */