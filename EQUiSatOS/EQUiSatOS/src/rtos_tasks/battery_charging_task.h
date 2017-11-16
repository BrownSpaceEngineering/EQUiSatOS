/*
 * IncFile1.h
 *
 * Created: 10/15/2017 3:21:50 PM
 *  Author: rj16
 */ 


#ifndef BATTERY_CHARGING_TASK_H_
#define BATTERY_CHARGING_TASK_H_

#define BATTERY_DEBUG

#include "global.h"
#include "rtos_tasks.h"

// thresholds for making very critical charging decisions, including when to go into low power mode
// and when to declare end of life
#define critical   40
#define low        70
#define high       90

// difference between battery percentages that's considered significant enough to change the charhing plan
#define difference 10

#define ITERATIONS_FOR_BATTERY_LOGIC    BATTERY_LOGIC_FREQ / BATTERY_CHARGING_TASK_FREQ

extern bool batt_charging[4];
extern int batt_strikes[4];

// defines each battery and/or bank
typedef enum
{	
	LION_ONE,
	LION_TWO,
	LIFE_PO_BANK_ONE,
	LIFE_PO_BANK_TWO
} battery;

void battery_logic(int lion_one_percentage, int lion_two_percentage, int life_po_bank_one_percentage, int life_po_bank_two_percentage);
void check_for_end_of_life(int lion_one_percentage, int lion_two_percentage);
void battery_charging_task(void *pvParameters);

#endif /* BATTERY_CHARGING_TASK_H_ */