#include "equisim_simulated_data.h"

/************************************************************************/
/* Misc. testing variables                                              */
/************************************************************************/
int simulated_timestamp = 0;
int simulated_state = IDLE_NO_FLASH;
int simulated_curr_charging_filled_up = 0;

/************************************************************************/
/* Battery charging action setting interfaces                           */
/************************************************************************/
equisim_bat_actions_t cur_actions;
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

uint32_t equisim_get_current_timestamp() {
	return EQUISIM_TIMESTAMP_SCALING * get_current_timestamp();
}

/************************************************************************/
/* Battery representations (list)                                       */
/************************************************************************/
equisim_bat_state_t normal_charge_discharge(uint32_t timestamp, equisim_bat_actions_t actions);

/************************************************************************/
/* Battery voltage/current output interfaces                            */
/************************************************************************/
void equisim_read_lion_volts_precise(uint16_t* val_1, uint16_t* val_2) {
	equisim_bat_state_t state = normal_charge_discharge(equisim_get_current_timestamp(), cur_actions);
	*val_1 = state.li1_volts;
	*val_2 = state.li2_volts;
}

void equisim_read_lifepo_volts_precise(uint16_t* val_1, uint16_t* val_2, uint16_t* val_3, uint16_t* val_4) {
	
}

void equisim_read_lion_current_precise(uint16_t* val_1, uint16_t* val_2) {
	
}

void equisim_read_lifepo_current_precise(uint16_t* val_1, uint16_t* val_2, uint16_t* val_3, uint16_t* val_4) {
	
}

/************************************************************************/
/* Actual battery representation functions                              */
/************************************************************************/
equisim_bat_state_t normal_charge_discharge(uint32_t timestamp, equisim_bat_actions_t actions) {
	
}