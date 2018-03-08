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

void fault_to_default()
{
	// NOTE: actie low, so flipping here
	cur_state.l1_faultn = 1;
	cur_state.l2_faultn = 1;
	cur_state.lf_b1_faultn = 1;
	cur_state.lf_b1_faultn = 1;
}

void panel_to_default()
{
	cur_state.panel_ref_mv = 9000;
	cur_state.spf_st = 1;
}

void currents_to_default()
{
	cur_state.li1_current = 0;
	cur_state.li2_current = 0;
	cur_state.lf1_current = 0;
	cur_state.lf2_current = 0;
	cur_state.lf3_current = 0;
	cur_state.lf4_current = 0;
}

void st_normal()
{
	cur_state.l1_st = cur_actions.l1_disg;
	cur_state.l2_st = cur_actions.l2_disg;
}

void chgn_normal()
{
	// NOTE: active low, so flipping here
	cur_state.l1_chgn = !cur_actions.l1_run_chg;
	cur_state.l2_chgn = !cur_actions.l2_run_chg;
	cur_state.lf_b1_chgn = !cur_actions.lf_b1_runchg;
	cur_state.lf_b2_chgn = !cur_actions.lf_b2_runchg;
}

void voltages_normal(uint64_t timestamp_ms)
{
	uint64_t time_since_last_update = timestamp_ms - cur_actions.last_setting_time_ms;
	cur_actions.last_setting_time_ms = timestamp_ms;

	// starting with linear for now
	float li_charging_mv_per_ms = 0.005;
	float li_discharging_mv_per_ms = -0.003;
	float lf_charging_mv_per_ms = 0.005;

	// discharging
	if (cur_actions.l1_disg && cur_actions.l2_disg)
	{
		cur_state.li1_volts -= (li_discharging_mv_per_ms / 2);
		cur_state.li2_volts -= (li_discharging_mv_per_ms / 2);
	}
	else
	{
		cur_state.li1_volts -= cur_actions.l1_disg ? (li_discharging_mv_per_ms / 2) : 0;
		cur_state.li2_volts -= cur_actions.l2_disg ? (li_discharging_mv_per_ms / 2) : 0;
	}

	cur_state.li1_volts += cur_actions.l1_run_chg ? li_charging_mv_per_ms : 0;
	cur_state.li2_volts += cur_actions.l2_run_chg ? li_charging_mv_per_ms : 0;
	cur_state.lf1_volts += cur_actions.lf_b1_runchg ? lf_charging_mv_per_ms : 0;
	cur_state.lf2_volts += cur_actions.lf_b1_runchg ? lf_charging_mv_per_ms : 0;
	cur_state.lf3_volts += cur_actions.lf_b2_runchg ? lf_charging_mv_per_ms : 0;
	cur_state.lf4_volts += cur_actions.lf_b2_runchg ? lf_charging_mv_per_ms : 0;

	// can't be negative
	cur_state.li1_volts = Max(cur_state.li1_volts, 0);
	cur_state.li2_volts = Max(cur_state.li2_volts, 0);
	cur_state.lf1_volts = Max(cur_state.lf1_volts, 0);
	cur_state.lf2_volts = Max(cur_state.lf2_volts, 0);
	cur_state.lf3_volts = Max(cur_state.lf3_volts, 0);
	cur_state.lf4_volts = Max(cur_state.lf4_volts, 0);
}

void normal_charge_discharge(uint64_t timestamp_ms) {
		// takes the information from the action struct and updates the state
		// struct

		st_normal(); // ST pins
		chgn_normal(); // CHGN pins
		fault_to_default(); // FAULTN pins
		panel_to_default(); // PANEL_REF
		currents_to_default(); // currents
		voltages_normal(timestamp_ms);
}
