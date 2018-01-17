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

// thresholds for making very critical charging decisions, including when to go into low power mode
// and when to declare end of life
#define li_up_mv                   4170
#define li_down_mv                 4050
#define li_low_power_mv            3900
#define li_critical_mv             2750

#define lf_full_max_mv             3500
#define lf_flash_avg_mv            3250

#define time_to_wait_for_crit_ms   2

// the battery that's currently charging
int batt_charging;

// the battery that's currently discharging
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

int get_global_state(
	int li1_mv,
	int li2_mv,
	int lfb1_avg_mv,
	int lfb2_avg_mv);

void check_for_end_of_life(int li1_mv, int li2_mv);
void battery_charging_task(void *pvParameters);

#endif /* BATTERY_CHARGING_TASK_H_ */
