/*
 * IncFile1.h
 *
 * Created: 10/15/2017 3:21:50 PM
 *  Author: rj16
 */


#ifndef BATTERY_CHARGING_TASK_H_
#define BATTERY_CHARGING_TASK_H_

#include "global.h"
#include "rtos_tasks.h"

// TODO: figure out these thresholds fully

// thresholds for making very critical charging decisions, including when to go
// into low power mode and when to declare end of life
#define LI_UP_MV                   4170
#define LI_DOWN_MV                 4050
#define LI_LOW_POWER_MV            3900
#define LI_CRITICAL_MV             2750

#define LF_FULL_MAX_MV             3500
#define LF_FLASH_AVG_MV            3250

// the battery that's currently charging
int batt_charging;

// the battery that's currently discharging
// NOTE: this is only ever a Lion
int batt_discharging;

// charging state
int curr_charge_state;

// number of strikes for each battery
int batt_strikes[4];

// NOTE: the order of elements of this enum is very important -- do not change!
// defines each battery and/or bank
typedef enum
{
	LI1 = 0,
	LI2,
	LFB1,
	LFB2
} battery;

typedef enum
{
	FILL_LI,
	FILL_LF
} charge_state;

void battery_logic(
	int li1_mv,
	int li2_mv,
	int lf1_mv,
	int lf2_mv,
	int lf3_mv,
	int lf4_mv);

#endif /* BATTERY_CHARGING_TASK_H_ */
