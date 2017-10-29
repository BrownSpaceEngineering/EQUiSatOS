/*
 * IncFile1.h
 *
 * Created: 10/15/2017 3:21:50 PM
 *  Author: rj16
 */ 


#ifndef BATTERY_CHARGING_TASK_H_
#define BATTERY_CHARGING_TASK_H_

#include "rtos_tasks.h"

// Thresholds
#define critical   40
#define low        70
#define high       90

// Amount of charge difference needed to make charging decisions
#define difference 5

// Defines the distribution of power between Lion and LifePo
typedef enum
{
	FULL_LION,
	HALF_LION_HALF_LIFE_PO,
	FULL_LIFE_PO,
	FULL_LION_SPLIT
} charge_state;

// defines each battery and/or bank
typedef enum
{	
	LION_ONE,
	LION_TWO,
	LIFE_PO_BANK_ONE,
	LIFE_PO_BANK_TWO
} battery;

#endif /* BATTERY_CHARGING_TASK_H_ */