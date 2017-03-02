/*
 * Watchdog_Task.h
 *
 * Created: 3/1/2017 9:28:19 PM
 *  Author: jleiken
 */ 

#ifndef WATCHDOG_TASK_H_
#define WATCHDOG_TASK_H_

#include "asf.h"
#include "USART_Commands.h"

#define ANTENNA_DEPLOY_SHIFT 0
#define BATTERY_CHARGING_SHIFT 1
#define TRANSMIT_SHIFT 2
#define FLASH_ACTIVATE_SHIFT 3
#define CURRENT_DATA_SHIFT 4
#define ATTITUDE_DATA_SHIFT 5

// This is just a placeholder before integration with rtos_tasks.c
#define WATCHDOG_TASK_FREQ 10

uint8_t check_ins;
uint8_t is_running;
void watchdog_task(void *pvParameters);
void check_in_task(uint8_t task_ind);
void running_task(uint8_t task_ind);

#endif /* WATCHDOG_TASK_H_ */