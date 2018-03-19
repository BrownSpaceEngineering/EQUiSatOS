/*
 * battery_charging_simulated_data.h
 *
 * Created: 2/10/2018 12:40:32 AM
 *  Author: rjha
 */

#ifndef BATTERY_CHARGING_SIMULATED_DATA_H_
#define BATTERY_CHARGING_SIMULATED_DATA_H_

#include "rtos_tasks/rtos_tasks.h"
#include "../rtos_tasks/battery_charging_task.h"

// NOTE: for battery charging unit tests, not the simulator!
extern int simulated_timestamp;
extern int simulated_state;
extern int simulated_curr_charging_filled_up;
extern int simulated_lfs_both_filled_up;

/************************************************************************/
/* CONFIG                                                               */
/************************************************************************/
#ifdef TESTING_SPEEDUP
	#define EQUISIM_TIMESTAMP_SCALING		3
#else
	#define EQUISIM_TIMESTAMP_SCALING		1
#endif
#define LI_CHARGING_MV_PER_MS_HIGH				0.005
#define LI_CHARGING_MV_PER_MS_LOW         0.01
#define LI_DISCHARGING_MV_PER_MS_HIGH			0.003
#define LI_DISCHARGING_MV_PER_MS_LOW      0.006
#define LI_HIGH_LOW_THRESHOLD             4000
#define LI_VOLTS_CEILING				  4020

#define LF_CHARGING_MV_PER_MS_HIGH				         0.004
#define LF_CHARGING_MV_PER_MS_LOW                  0.008
#define LF_DISCHARGING_MV_PER_MS_FLASHING_HIGH		 0.0025
#define LF_DISCHARGING_MV_PER_MS_FLASHING_LOW      0.005
#define LF_HIGH_LOW_THRESHOLD                      3000
#define LF_VOLTS_CEILING				LF_FULL_MAX_MV + 20

/************************************************************************/
/* Battery charging sim states                                          */
/************************************************************************/
typedef struct equisim_bat_actions_t {
	// lions
	bool l1_run_chg;
	bool l2_run_chg;
	bool l1_disg;
	bool l2_disg;

	// lifepos
	bool lf_b1_runchg;
	bool lf_b2_runchg;

	// timings
	uint64_t last_setting_time_ms;
} equisim_bat_actions_t;

typedef struct equisim_bat_state_t {
	// discharge state
	bool l1_st;
	bool l2_st;

	// chip outputs
	bool l1_chgn_inv;
	bool l2_chgn_inv;
	bool l1_faultn_inv;
	bool l2_faultn_inv;
	bool lf_b1_chgn_inv;
	bool lf_b2_chgn_inv;
	bool lf_b1_faultn_inv;
	bool lf_b2_faultn_inv;

	// panel-related state
	bool spf_st;
	uint8_t panel_ref_mv;

	// voltages
	uint16_t li1_volts;
	uint16_t li2_volts;
	uint16_t lf1_volts;
	uint16_t lf2_volts;
	uint16_t lf3_volts;
	uint16_t lf4_volts;

	// currents
	uint16_t li1_current;
	uint16_t li2_current;
	uint16_t lf1_current;
	uint16_t lf2_current;
	uint16_t lf3_current;
	uint16_t lf4_current;
} equisim_bat_state_t;

// setting actions
void equisim_init(void);
void equisim_set_action_by_pin(bool setting, int pin);

// functions to get state
void equisim_read_bat_charge_dig_sigs_batch(bat_charge_dig_sigs_batch* batch);
void equisim_read_lion_volts_precise(uint16_t* val_1, uint16_t* val_2);
void equisim_read_lifepo_volts_precise(uint16_t* val_1, uint16_t* val_2, uint16_t* val_3, uint16_t* val_4);
void equisim_read_lion_current_precise(uint16_t* val_1, uint16_t* val_2);
void equisim_read_lifepo_current_precise(uint16_t* val_1, uint16_t* val_2, uint16_t* val_3, uint16_t* val_4);
uint16_t equisim_read_panelref(void);

#endif /* BATTERY_CHARGING_SIMULATED_DATA_H_ */
