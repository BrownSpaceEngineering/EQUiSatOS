#include "equisim_simulated_data.h"

/************************************************************************/
/* Misc. testing variables                                              */
/************************************************************************/
int simulated_timestamp = 0;
int simulated_state = IDLE_NO_FLASH;
int simulated_curr_charging_filled_up = 0;

/************************************************************************/
/* Battery representations (forward defined list)                       */
/************************************************************************/
void normal_charge_discharge(uint64_t timestamp_ms);

/************************************************************************/
/* Battery charging action setting interfaces                           */
/************************************************************************/
equisim_bat_actions_t cur_actions;
equisim_bat_state_t cur_state;
// 	bool l1_run_chg;
// 	bool l2_run_chg;
// 	bool l1_disg;
// 	bool l2_disg;
// 	// lifepos
// 	bool lf_b1_runchg;
// 	bool lf_b2_runchg;

void equisim_set_action_by_pin(bool setting, int pin) {
	switch (pin) {
		case P_L1_RUN_CHG: cur_actions.l1_run_chg = setting;
		// TODO...
	}
}

uint32_t equisim_get_current_timestamp_ms() {
	return EQUISIM_TIMESTAMP_SCALING * get_current_timestamp_ms();
}

void update_bat_state_from_actions(void) {
	normal_charge_discharge(equisim_get_current_timestamp_ms());
}

/************************************************************************/
/* Battery voltage/current output interfaces                            */
/************************************************************************/
void equisim_read_lion_volts_precise(uint16_t* val_1, uint16_t* val_2) {
	update_bat_state_from_actions();
	*val_1 = cur_state.li1_volts;
	*val_2 = cur_state.li2_volts;
}

void equisim_read_lifepo_volts_precise(uint16_t* val_1, uint16_t* val_2, uint16_t* val_3, uint16_t* val_4) {
	update_bat_state_from_actions();
	*val_1 = cur_state.lf1_volts;
	*val_2 = cur_state.lf2_volts;
	*val_3 = cur_state.lf3_volts;
	*val_4 = cur_state.lf4_volts;
}

void equisim_read_lion_current_precise(uint16_t* val_1, uint16_t* val_2) {
	*val_1 = cur_state.li1_current;
	*val_2 = cur_state.li2_current;
}

void equisim_read_lifepo_current_precise(uint16_t* val_1, uint16_t* val_2, uint16_t* val_3, uint16_t* val_4) {
	update_bat_state_from_actions();
	*val_1 = cur_state.lf1_current;
	*val_2 = cur_state.lf2_current;
	*val_3 = cur_state.lf3_current;
	*val_4 = cur_state.lf4_current;
}

/************************************************************************/
/* Actual battery representation functions                              */
/************************************************************************/
void normal_charge_discharge(uint64_t timestamp_ms) {
	
}