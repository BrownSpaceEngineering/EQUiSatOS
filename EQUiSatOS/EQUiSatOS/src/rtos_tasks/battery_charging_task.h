/*
 * battery_charging_task.h
 *
 * Created: 10/15/2017 3:21:50 PM
 *  Author: rjha
 */


#ifndef BATTERY_CHARGING_TASK_H_
#define BATTERY_CHARGING_TASK_H_

#include <asf.h>
#include "../data_handling/persistent_storage.h"
#include "rtos_tasks.h"
#include "testing_functions/equisim_simulated_data.h"

// #define BAT_UNIT_TESTING

// thresholds for making very critical charging decisions, including when to go
// into low power mode and when to declare end of life
#define FULL_BAT_CHARGING_PERIOD_MINS   45
#define BAT_CHARGING_PERIOD_MINS        9
#define BAT_CHARGING_ITERS_UNTIL_FULL   (FULL_BAT_CHARGING_PERIOD_MINS / BAT_CHARGING_PERIOD_MINS)

#define LI_FULL_MV                      4170
#define LI_FULL_LOW_MV					4140
#define LI_FULL_LOWER_MV				4100
#define LI_FULL_ANTENNA_DEPLOY_MV       4050
#define LI_DOWN_MV                 		4000
#define LI_LOW_POWER_MV            		3900
#define LI_CRITICAL_MV             		3500
#define LI_MIGHT_NOT_BE_FULL_MV         4050

#define LF_FULL_SUM_MV					6750
#define LF_FULL_MAX_MV                  3900
#define LF_FULL_SANITY_MV               6500
#define LF_MIGHT_NOT_BE_FULL_MV         6400
#define LF_FLASH_MIN_MV            	  	6200

#define SNS_THRESHOLD_LOW               1025
#define SNS_THRESHOLD_LOWER             1000

// something of a misnomer -- the total number of tries
#define RETRIES_AFTER_MUTEX_TIMEOUT     7

// thresholds for error checking and the strikes system
#define DELAY_BEFORE_RETRY_MS           400
#define SAT_NO_POWER_TURN_OFF_T_MS		1000

#define MAX_RECOMMISSION_TIME_S         (3*24*60*60)   // 3 days
#define MAX_TIME_BELOW_V_THRESHOLD_S	(3*60*60)      // 3 hrs
#define INITIAL_RECOMMISSION_TIME_S     (3*60*60)      // 3 hrs

#define PANEL_REF_SUN_MV                1474	// 7500 mv

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
	LI1_DISG,
	LI2_DISG,
	BOTH_DISG,
	NONE_DISG
} li_discharging_t;

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
	int8_t bat_charging;

	// the battery that's currently discharging
	// NOTE: this is only ever a Lion
	int8_t lion_discharging;

	// meta-charging state
	meta_charge_state_t curr_meta_charge_state;

	// charging state
	charge_state_t curr_charge_state;

	// the last time each lion was low voltage
	uint32_t li_entered_low_voltage_timestamp[2];

	// whether or not it's safe to move to antenna deploy at the moment
	bool should_move_to_antenna_deploy;

	// whether or not the satellite state has already been set with the state of each of
	// the batteries
	bool already_set_sat_state[4];

	// voltage data
	uint16_t bat_voltages[4];

	// whether or not the batteries are decommissioned
	bool decommissioned[4];

	// the time at which the battery was last decommissioned
	uint32_t decommissioned_timestamp[4];

	// the battery's total number of decommissions
	uint16_t decommissioned_count[4];

	bool charging_parity;
} charging_data_t;

// although it somewhat breaks abstraction to have the main parameter to
// battery logic be a global variable, this is necessary for some of the
// helper functions
charging_data_t charging_data;

bool read_bat_charge_dig_sigs_batch_with_retry(bat_charge_dig_sigs_batch *batch);
li_discharging_t get_li_discharging(void);
uint8_t get_error_loc(int8_t bat);
uint32_t get_current_timestamp_wrapped(void);
sat_state_t get_sat_state_wrapped(void);
bool fault_pin_active(int8_t bat, bat_charge_dig_sigs_batch batch);
uint8_t get_run_chg_pin(int8_t bat);
uint8_t get_run_dischg_pin(int8_t bat);
bool chg_pin_active(int8_t bat, bat_charge_dig_sigs_batch batch);
bool st_pin_active(int8_t bat, bat_charge_dig_sigs_batch batch);
bool is_lion(int8_t bat);
void init_charging_data(void);
void set_li_to_discharge(int8_t bat, bool discharge);
void set_bat_to_charge(int8_t bat, bool charge);
void battery_logic(void);
void decommission(int8_t bat);
uint32_t time_for_recommission(int8_t bat);
bool check_for_recommission(int8_t bat);
void charge_lower_lf_bank(uint16_t lfb1_max_cell_mv, uint16_t lfb2_max_cell_mv);
void charge_lower_li(void);
void discharge_higher_li(void);
void check_after_charging(int8_t bat_charging, int8_t old_bat_charging);
bool check_chg_should_decommission(int8_t bat, bool should_be_charging, bat_charge_dig_sigs_batch batch);
void check_chg_with_retry(int8_t bat, bool should_be_charging, bat_charge_dig_sigs_batch batch);
void check_fault_with_retry(int8_t bat, bat_charge_dig_sigs_batch batch);
bool get_lf_full(uint16_t sum_cell_mv, uint16_t max_cell_mv, int8_t lf, bat_charge_dig_sigs_batch batch, bool got_batch);
bool get_lfs_both_full(uint8_t num_lf_down, int8_t good_lf, uint16_t lfb1_max_cell_mv, uint16_t lfb2_max_cell_mv);
void check_after_discharging(int8_t bat_discharging, int8_t bat_not_discharging);
void check_discharging_with_retry(int8_t bat_discharging, bat_charge_dig_sigs_batch batch);
void check_not_discharging_with_retry(int8_t bat_not_discharging, bat_charge_dig_sigs_batch batch);
void update_should_deploy_antenna(bool has_li_data);

void run_unit_tests(void);
void run_simulations(void);

#endif /* BATTERY_CHARGING_TASK_H_ */
