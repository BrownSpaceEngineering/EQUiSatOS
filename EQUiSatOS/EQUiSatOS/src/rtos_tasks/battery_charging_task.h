/*
 * IncFile1.h
 *
 * Created: 10/15/2017 3:21:50 PM
 *  Author: rj16
 */


#ifndef BATTERY_CHARGING_TASK_H_
#define BATTERY_CHARGING_TASK_H_

#include <asf.h>
#include "rtos_tasks.h"
#include "testing_functions/battery_charging_simulated_data.h"
#include "../data_handling/persistent_storage.h"

// TODO: figure out these thresholds fully and deal with scaling

// #define BAT_TESTING
#define WITHOUT_DECOMMISION

// thresholds for making very critical charging decisions, including when to go
// into low power mode and when to declare end of life
#define LI_FULL_MV                      4190
#define LI_FULL_SANITY_MV               4100
#define LI_DOWN_MV                 			4050
#define LI_LOW_POWER_MV            			3900
#define LI_CRITICAL_MV             			2750

#define LF_FULL_MAX_MV             			3500
#define LF_FULL_SANITY_MV               3000
#define LF_FLASH_AVG_MV            			3250

// thresholds for error checking and the strikes system
#define MIGHT_BE_FULL                   4000
#define MAX_TIME_WITHOUT_FULL_MS        6000
#define MAX_TIME_WITHOUT_CHARGE_MS      (3 * 60 * 60 * 1000)

#define BAT_MUTEX_WAIT_TIME_TICKS       (3000 / portTICK_PERIOD_MS)

#define TRY_PIN_DELAY_TIME_MS           100
#define MAX_TIMES_TRY_PIN               3
#define WAIT_TIME_BEFORE_PIN_CHECK_MS   10

#define MAX_TIME_TO_WAIT_FOR_DEPLOY_S   10000 // TODO: figure this out

#define MAX_RECOMISSION_TIME_S          10000 // TODO: figure this out
#define MAX_TIME_BELOW_V_THRESHOLD_S    10000
#define INITIAL_RECOMISSION_TIME_S      500
#define RECOMISSION_TIME_INCREASE       2

#define PANEL_REF_SUN_MV                7500

// NOTE: the order of elements of this enum is very important -- do not change!
// defines each battery and/or bank
typedef enum
{
	LI1 = 0,
	LI2,
	LFB1,
	LFB2,
} battery_t;

typedef enum
{
		// also known as A
	  ALL_GOOD,

		// also known as B
		ONE_LI_DOWN,

		// also known as C
		TWO_LF_DOWN,

		// also known as D
		TWO_LI_DOWN
} meta_charge_state_t;

typedef enum
{
	// states in meta-state ALL_GOOD (A)
	FILL_LI_NEITHER_FULL_A,
	FILL_LI_LI1_FULL_A,
	FILL_LI_LI2_FULL_A,
	FILL_LF_A,

	// states in meta-state ONE_LI_DOWN (B)
	FILL_LI_B,
	FILL_LF_B,

	// states in meta-state TWO_LF_DOWN (C)
	FILL_LI_C,

	// states in meta-state TWO_LI_DOWN(D)
	FILL_LI_D
} charge_state_t;

typedef struct charging_data
{
	// the battery that's currently charging
	int bat_charging;

	// the battery that's currently discharging
	// NOTE: this is only ever a Lion
	int lion_discharging;

	// meta-charging state
	meta_charge_state_t curr_meta_charge_state;

	// charging state
	charge_state_t curr_charge_state;

	// TODO: make sure that this does well on a reboot
	// the timestamp when the LI was last full
	int li_last_full_or_recommissioned_timestamp[2];

	// the last time each lion was low voltage
	int li_entered_low_voltage_timestamp[2];

	// whether or not it's safe to move to antenna deploy at the moment
	int should_move_to_antenna_deploy;

	// whether or not the satellite state has already been set witht the state of each of
	// the batteries
	int already_set_sat_state[4];

	// voltage data
	int bat_voltages[4];

	// old voltage data
	int old_bat_voltages[4];

	// whether or not the batteries are decomissioned
	int decommissioned[4];

	// the time at which the battery was last decomissioned
	int decommissioned_timestamp[4];

	// the battery's total number of decomissions
	int decommissioned_count[4];

	int charging_parity;
} charging_data_t;

// NOTE: these are initialized elsewhere -- should they maybe not be?
// anyone doing something during which the battery state should not
// be changed needs to honor this mutex
StaticSemaphore_t _battery_charging_mutex_d;
SemaphoreHandle_t battery_charging_mutex;

// although it somewhat breaks abstraction to have the main parameter to
// battery logic be a global variable, this is necessary for some of the
// helper functions
charging_data_t charging_data;

persistent_charging_data_t persistent_charging_data;

int get_error_loc(battery_t bat);
int get_current_timestamp_wrapped(void);
sat_state_t get_sat_state_wrapped(void);
int get_fault_pin_val_w_conversion(battery_t bat);
int get_run_chg_pin(battery_t bat);
int get_run_dischg_pin(battery_t bat);
int get_chg_pin_val_w_conversion(battery_t bat);
int get_st_val(battery_t bat);
int get_panel_ref_val(void);
int is_lion(battery_t bat);
void init_charging_data(void);
void set_li_to_discharge(int bat, int discharge);
void set_bat_to_charge(int bat, int charge);
int battery_logic(void);
void decommission(battery_t bat);
void undecommission(battery_t bat);
int time_for_recomission(battery_t bat);
void check_for_recomission(battery_t bat);

void run_unit_tests(void);
void run_simulations(void);

#endif /* BATTERY_CHARGING_TASK_H_ */
