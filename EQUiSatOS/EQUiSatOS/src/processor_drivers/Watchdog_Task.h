/*
 * Watchdog_Task.h
 *
 * Created: 3/1/2017 9:28:19 PM
 *  Author: jleiken
 */ 

#ifndef WATCHDOG_TASK_H_
#define WATCHDOG_TASK_H_

#include <global.h>
#include "Watchdog_Commands.h"
#include "../runnable_configurations/rtos_tasks.h"

#define ANTENNA_DEPLOY_SHIFT 1
#define BATTERY_CHARGING_SHIFT 2
#define TRANSMIT_SHIFT 3
#define FLASH_ACTIVATE_SHIFT 4
#define CURRENT_DATA_SHIFT 5
#define ATTITUDE_DATA_SHIFT 6

#define WATCHDOG_MUTEX_WAIT_TIME_TICKS 10

// static memory for watchdog task mutex
StaticSemaphore_t _watchdog_task_mutex_d;

void watchdog_init(void);
void watchdog_task(void *pvParameters);
void check_in_task(uint8_t task_ind);
void running_task(uint8_t task_ind);
bool watchdog_as_function(void);
void check_out_task(uint8_t task_ind);

#endif /* WATCHDOG_TASK_H_ */