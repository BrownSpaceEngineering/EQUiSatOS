/*
 * battery_charging_simulated_data.h
 *
 * Created: 2/10/2018 12:40:32 AM
 *  Author: rjha
 */
#include "rtos_tasks/rtos_tasks.h"

#ifndef BATTERY_CHARGING_SIMULATED_DATA_H_
#define BATTERY_CHARGING_SIMULATED_DATA_H_

// NOTE: for battery charging unit tests, not the simulator!
extern int simulated_timestamp;
extern int simulated_state;
extern int simulated_curr_charging_filled_up;

/************************************************************************/
/* CONFIG                                                               */
/************************************************************************/
#define EQUISIM_TIMESTAMP_SCALING	1

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
	bool l1_chgn;
	bool l2_chgn;
	bool l1_faultn;
	bool l2_faultn;
	bool lf_b1_chgn;
	bool lf_b2_chgn;
	bool lf_b1_faultn;
	bool lf_b2_faultn;

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

// functions to get state
void equisim_read_lion_volts_precise(uint16_t* val_1, uint16_t* val_2);
void equisim_read_lifepo_volts_precise(uint16_t* val_1, uint16_t* val_2, uint16_t* val_3, uint16_t* val_4);
void equisim_read_lion_current_precise(uint16_t* val_1, uint16_t* val_2);
void equisim_read_lifepo_current_precise(uint16_t* val_1, uint16_t* val_2, uint16_t* val_3, uint16_t* val_4);

#endif /* BATTERY_CHARGING_SIMULATED_DATA_H_ */
