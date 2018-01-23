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
#define LI_FULL_SANITY_MV               4100
#define LI_DOWN_MV                 			4050
#define LI_LOW_POWER_MV            			3900
#define LI_CRITICAL_MV             			2750

#define LF_FULL_MAX_MV             			3500
#define LF_FLASH_AVG_MV            			3250

// thresholds for error checking and the strikes system
#define MIGHT_BE_FULL                   4000
#define MAX_TIME_WITHOUT_FULL_MS        6000
#define MAX_VOLTAGE_DROP_MV             300
#define MAX_VOLTAGE_DROP_W_CHARGE_MV		200

#define BAT_MUTEX_WAIT_TIME_TICKS       1000

#define TRY_PIN_DELAY_TIME_MS           100
#define MAX_TIMES_TRY_PIN               3
#define WAIT_TIME_BEFORE_PIN_CHECK_MS   10

// NOTE: the order of elements of this enum is very important -- do not change!
// defines each battery and/or bank
typedef enum
{
	LI1 = 0,
	LI2,
	LFB1,
	LFB2
} battery_t;

typedef enum
{
	FILL_LI_NEITHER_FULL,
	FILL_LI_LI1_FULL,
	FILL_LI_LI2_FULL,
	FILL_LF
} charge_state_t;

typedef struct charging_data {
	// the battery that's currently charging
	battery_t bat_charging;

	// the battery that's currently discharging
	// NOTE: this is only ever a Lion
	battery_t lion_discharging;

	// charging state
	charge_state_t curr_charge_state;

	// number of strikes for each battery
	int bat_strikes[4];

	// the last time each lion was full
	int li1_full_timestamp;
	int li2_full_timestamp;

	// whether or not the satellite state has already been set
	int already_set_sat_state;

	// voltage data
	int bat_voltages[4];

	// old voltage data
	int old_bat_voltages[4];
} charging_data_t;

// NOTE: these are initialized elsewhere -- should they maybe not be?
// anyone doing something during which the battery state should not
// be changed needs to honor this mutex
StaticSemaphore_t _battery_charging_mutex_d;
SemaphoreHandle_t _battery_charging_mutex;

// although it somewhat breaks abstraction to have the main parameter to
// battery logic be a global variable, this is necessary for some of the
// helper functions that use as
charging_data_t charging_data;

int get_fault_pin_val_w_conversion(battery_t bat);
int get_run_chg_pin(battery_t bat);
int get_run_dischg_pin(battery_t bat);
int get_chg_pin_val_w_conversion(battery_t bat);
int get_st_val(battery_t bat);
int get_panel_ref_val(void);
int is_lion(battery_t bat);
void init_charging_data(void);
void battery_logic(void);

#endif /* BATTERY_CHARGING_TASK_H_ */
