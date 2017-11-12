/*
 * IncFile1.h
 *
 * Created: 10/15/2017 3:21:50 PM
 *  Author: rj16
 */ 


#ifndef BATTERY_CHARGING_TASK_H_
#define BATTERY_CHARGING_TASK_H_

#define BATTERY_DEBUG

#include "rtos_tasks.h"

// thresholds for making very critical charging decisions, including when to go into low power mode
// and when to declare end of life
#define critical   40
#define low        70
#define high       90

// difference between battery percentages that's considered significant enough to change the charhing plan
#define difference 10

// defines the distribution of power between Lion and LifePo
typedef enum
{
	FULL_LION,
	FULL_LIFE_PO
} charge_state;

// defines each battery and/or bank
typedef enum
{	
	LION_ONE,
	LION_TWO,
	LIFE_PO_BANK_ONE,
	LIFE_PO_BANK_TWO
} battery;

///
// current state of the battery charging task
// (exposed here, so the tester can initialize and manipulate the state)
///

// current state
int state = FULL_LION;

// the batteries that are currently charging
// zero -- not charging, one -- charging
// this array represents both the past state and the current state
// it's smartly refreshed such that if it's old values are needed,
// they aren't overwritten until after they're needed
bool charging[4] = {false, false, false, false};

// number of strikes for each battery
int strikes[4] = {0, 0, 0, 0};

void battery_logic(int lion_one_percentage, int lion_two_percentage, int life_po_bank_one_percentage, int life_po_bank_two_percentage);
void check_for_end_of_life(int lion_one_percentage, int lion_two_percentage);
void battery_charging_task(void *pvParameters);

#endif /* BATTERY_CHARGING_TASK_H_ */