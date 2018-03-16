/*
 * battery_charging_task.c
 *
 * Created: 9/21/2017 20:37:56
 *  Author: rjha
 */

#include "battery_charging_task.h"

// TODO*: finalize strike conditions once and for all

// To check (TODO):
//   - make sure that everywhere an array is being indexed, the right thing is
//     happening (i.e. we never try to use the index -1)
//   - check for edge cases where something is dependent on the initial value of these state variables
//     (especially the li_discharging and the bat_charging variables)
//   - initialization of timestamps after reboot
//   - look out for battery_t variables being set to -1
//   - make sure the flow isn't in any way dependent on the individual values of the variable for the
//     discharging LI
//   - check all pins
//   - error priorities
//   - everything is okay coming back from a boot
//   - warnings
//   - flow through the discharging section on the first time through

li_discharging_t get_li_discharging(void)
{
	bat_charge_dig_sigs_batch batch;
	if (!read_bat_charge_dig_sigs_batch_with_retry(&batch))
	{
		if (charging_data.lion_discharging == LI1 || charging_data.lion_discharging == LI2)
			return charging_data.lion_discharging == LI1 ? LI1_DISG : LI2_DISG;

		return BOTH_DISG;
	}

	bool li1_st_active = st_pin_active(LI1, batch);
	bool li2_st_active = st_pin_active(LI2, batch);

	if (li1_st_active && !li2_st_active)
		return LI1_DISG;

	if (li2_st_active && !li1_st_active)
		return LI2_DISG;

	if (li1_st_active && li2_st_active)
		return BOTH_DISG;

	return NONE_DISG;
}

uint32_t get_current_timestamp_wrapped(void)
{
	#ifdef BAT_TESTING
	return simulated_timestamp;
	#else
	return get_current_timestamp();
	#endif
}

sat_state_t get_sat_state_wrapped(void)
{
	#ifdef BAT_TESTING
	return simulated_state;
	#else
	return get_sat_state();
	#endif
}

uint8_t get_error_loc(int8_t bat)
{
	switch (bat)
	{
		case LI1:
			return ELOC_BAT_L1;

		case LI2:
			return ELOC_BAT_L2;

		case LFB1:
			return ELOC_BAT_LFB1;

		case LFB2:
			return ELOC_BAT_LFB2;

		default:
			configASSERT(false);
			log_error(ELOC_BAT_CHARGING_SWITCH_1, ECODE_UNEXPECTED_CASE, true);
			return ELOC_BAT_CHARGING;
	}
}

uint8_t get_run_chg_pin(int8_t bat)
{
	switch (bat)
	{
		case LI1:
			return P_L1_RUN_CHG;

		case LI2:
			return P_L2_RUN_CHG;

		case LFB1:
			return P_LF_B1_RUNCHG;

		case LFB2:
			return P_LF_B2_RUNCHG;

		default:
			configASSERT(false);
			log_error(ELOC_BAT_CHARGING_SWITCH_2, ECODE_UNEXPECTED_CASE, true);
			return P_L1_RUN_CHG;
	}
}

uint8_t get_run_dischg_pin(int8_t bat)
{
	return (bat == LI1) ? P_L1_DISG : P_L2_DISG;
}

// NOTE: returns the chg pin value with the conversion from active low
bool chg_pin_active(int8_t bat, bat_charge_dig_sigs_batch batch)
{
	uint8_t chg_position;
	switch (bat)
	{
		case LI1:
			chg_position = 12;
			break;

		case LI2:
			chg_position = 14;
			break;

		case LFB1:
			chg_position = 7;
			break;

		case LFB2:
			chg_position = 4;
			break;

		default:
			configASSERT(false);
			log_error(ELOC_BAT_CHARGING_SWITCH_3, ECODE_UNEXPECTED_CASE, true);
			chg_position = 12;
			break;
	}

	// NOTE: CHGN is active low, so we're making a conversion here
	return !((batch>>chg_position)&0x1);
}

bool fault_pin_active(int8_t bat, bat_charge_dig_sigs_batch batch)
{
	uint8_t fault_position;
	switch (bat)
	{
		case LI1:
			fault_position = 13;
			break;

		case LI2:
			fault_position = 15;
			break;

		case LFB1:
			fault_position = 6;
			break;

		case LFB2:
			fault_position = 5;
			break;

		default:
			configASSERT(false);
			log_error(ELOC_BAT_CHARGING_SWITCH_4, ECODE_UNEXPECTED_CASE, true);
			fault_position = 13;
			break;
	}

	// NOTE: FAULTN is active low, so we're making a conversion here
	return !((batch>>fault_position)&0x1);
}

bool st_pin_active(int8_t bat, bat_charge_dig_sigs_batch batch)
{
	uint8_t st_position = (bat == LI1) ? 9 : 8;
	print("\tbat: %d, value: %d\n", bat, (batch>>st_position)&0x01);
	return (batch>>st_position)&0x01;
}

static uint16_t get_panel_ref_val_with_retry(void)
{
	uint16_t four_buf[4];
	bool success = false;
	for (int i = 0; i < RETRIES_AFTER_MUTEX_TIMEOUT && !success; i++)
		success = read_ad7991_batbrd_precise(four_buf);

	if (!success)
		return -1;

	return ((uint16_t)four_buf[2]);
}

bool read_bat_charge_dig_sigs_batch_with_retry(bat_charge_dig_sigs_batch *batch)
{
	bool success = false;
	for (int i = 0; i < RETRIES_AFTER_MUTEX_TIMEOUT && !success; i++)
		success = read_bat_charge_dig_sigs_batch(batch);

	return success;
}

bool is_lion(int8_t bat)
{
	return (bat == LI1 || bat == LI2);
}

// NOTE: no error message will be sent from here
void decommission(int8_t bat)
{
	if (!charging_data.decommissioned[bat])
	{
		charging_data.decommissioned[bat] = 1;
		charging_data.decommissioned_timestamp[bat] = get_current_timestamp_wrapped();
		charging_data.decommissioned_count[bat]++;
	}
}

uint32_t time_for_recommission(int8_t bat)
{
	return Min(charging_data.decommissioned_count[bat] * INITIAL_RECOMMISSION_TIME_S, MAX_RECOMMISSION_TIME_S);
}

bool check_for_recommission(int8_t bat)
{
	uint32_t time_since_decommission = get_current_timestamp_wrapped() - charging_data.decommissioned_timestamp[bat];
	print("\tdecomissioned at %d, now %d\n", charging_data.decommissioned_timestamp[bat], get_current_timestamp_wrapped());
	print("\t%d total decomissions for this bat\n");
	print("\tthis battery should be decomissioned for\n", time_for_recommission(bat));
	if (time_since_decommission > time_for_recommission(bat))
	{
		charging_data.decommissioned[bat] = 0;

		if (is_lion(bat))
		{
			// we also need to do various things to make sure that the battery won't be
			// immediately decommissioned again
			charging_data.decommissioned_timestamp[bat] = 0; // NOTE: this doesn't do much but keeps the struct consistent
			charging_data.li_entered_low_voltage_timestamp[bat] = -1;
			charging_data.li_last_full_or_recommissioned_timestamp[bat] = get_current_timestamp_wrapped();
		}

		return true;
	}

	return false;
}

void init_charging_data()
{
	// explicitly initializing these even though initial value doesn't matter
	// NOTE: nothing should rely on these variables' initial state without taking
	// -1 into account

	print("initializing charging data -- start\n");

	charging_data.bat_charging = -1;
	charging_data.lion_discharging = -1;

	// we want to initially be in ALL_GOOD
	charging_data.curr_meta_charge_state = ALL_GOOD;

	// we want to initially be in FILL_LI_NEITHER_FULL_A
	charging_data.curr_charge_state = FILL_LI_NEITHER_FULL_A;

	// set all battery timestamps to -1
	charging_data.li_last_full_or_recommissioned_timestamp[0] = get_current_timestamp_wrapped();
	charging_data.li_last_full_or_recommissioned_timestamp[1] = get_current_timestamp_wrapped();
	charging_data.li_entered_low_voltage_timestamp[0] = -1;
	charging_data.li_entered_low_voltage_timestamp[1] = -1;

	for (int8_t bat = 0; bat < 4; bat++)
		charging_data.already_set_sat_state[bat] = 0;

	charging_data.should_move_to_antenna_deploy = false;

	// set all new and old voltage values to 0
	// nothing should be dependent on these initial values
	for (int8_t bat = 0; bat < 4; bat++)
	{
		charging_data.bat_voltages[bat] = 0;
	}

	// initializing all values relevant to batteries and decommissioning
	for (int8_t bat = 0; bat < 4; bat++) // TODO: can't all of these for loops be combined into one?
	{
		charging_data.decommissioned[bat] = 0;
		charging_data.decommissioned_timestamp[bat] = 0;
		charging_data.decommissioned_count[bat] = 0;
	}

	charging_data.charging_parity = 0;

	// we need to decommission here if the MRAM says that one of the LI's caused
	// a reboot
	persistent_charging_data_t persist_data = cache_get_persistent_charging_data();
	if (persist_data.li_caused_reboot != -1)
	{
		print("mram has battery %d as having caused a reboot -- decommissioning\n");
		log_error(get_error_loc(persist_data.li_caused_reboot), ECODE_BAT_NOT_DISCHARGING, true);
		decommission(persist_data.li_caused_reboot);

		// reset persistent data so we give another chance
		persist_data.li_caused_reboot = -1;
		set_persistent_charging_data_unsafe(persist_data);
	}

	print("initializing charging data - complete\n");
}

void set_li_to_discharge(int8_t bat, bool discharge)
{
	uint8_t discharge_pin = get_run_dischg_pin(bat);

	// NOTE: discharge pins are active low
	#ifndef EQUISIM_SIMULATE_BATTERIES
		#ifdef BAT_CHARGING_ACTIVE
			set_output(!discharge, discharge_pin);
		#endif
	#else
		equisim_set_action_by_pin(!discharge, discharge_pin);
	#endif

	print("set li %d to discharge: %d\n", bat, discharge);
}

// TODO: retry after seeing problems?
void check_after_discharging(int8_t bat_discharging, int8_t bat_not_discharging)
{
	print("checking after discharge:\n");
	bat_charge_dig_sigs_batch batch;

	// if the mutex times out, we'll just return
	if (!read_bat_charge_dig_sigs_batch_with_retry(&batch))
		return;

	bool bat_discharging_st_pin_active = st_pin_active(bat_discharging, batch);
	bool bat_not_discharging_st_pin_active = st_pin_active(bat_not_discharging, batch);

	if (!bat_discharging_st_pin_active)
	{
		log_error(get_error_loc(bat_discharging), ECODE_BAT_NOT_DISCHARGING, true);
		print("\tbat %d not discharging -- decommissioning\n", bat_discharging);
		decommission(bat_discharging);
	}

	if (bat_not_discharging_st_pin_active)
	{
		log_error(get_error_loc(bat_not_discharging), ECODE_BAT_NOT_NOT_DISCHARGING, true);
		print("\tbat %d not not discharging -- decommissioning\n", bat_not_discharging);
		decommission(bat_not_discharging);
	}
}

void set_bat_to_charge(int8_t bat, bool charge)
{
	uint8_t chg_pin = get_run_chg_pin(bat);
	#ifndef EQUISIM_SIMULATE_BATTERIES
		#ifdef BAT_CHARGING_ACTIVE
			set_output(charge, chg_pin);
		#endif
	#else
		equisim_set_action_by_pin(charge, chg_pin);
	#endif

	print("set bat %d to charge: %d\n", bat, charge);
}

void check_fault(int8_t bat, bat_charge_dig_sigs_batch batch)
{
	if (fault_pin_active(bat, batch))
	{
		print("fault pin active for bat %d -- decommissioning\n", bat);
		log_error(get_error_loc(bat), ECODE_BAT_FAULT, true);
		decommission(bat);
	}
}

void check_chg(int8_t bat, bool should_be_charging, bat_charge_dig_sigs_batch batch)
{
	bool charge_running = chg_pin_active(bat, batch);
	if (should_be_charging)
	{
		print("checking is bat %d is charging:\n", bat);
		print("\tchg pin active: %d\n", chg_pin_active(charging_data.bat_charging, batch));
		print("\tpanel ref: %d\n", get_panel_ref_val_with_retry());

		// we can't make any claims if we don't know anything about PANEL_REF
		int panel_ref_val = get_panel_ref_val_with_retry();
		if (panel_ref_val == -1)
			return;

		if (!charge_running &&
			panel_ref_val > PANEL_REF_SUN_MV &&
			charging_data.bat_voltages[bat] < MIGHT_BE_FULL)
		{
			print("\tnot charging for bat %d -- decommissioning\n", bat);
			log_error(get_error_loc(bat), ECODE_BAT_NOT_CHARGING, false);
			decommission(bat);
		}
		else
		{
			print("\tnot decommissioning!\n");
		}
	}
	else if (charge_running)
	{
		log_error(get_error_loc(bat), ECODE_BAT_NOT_NOT_CHARGING, false);
	}
}

// TODO: retry after seeing problems?
void check_after_charging(int8_t bat_charging, int8_t old_bat_charging)
{
	bat_charge_dig_sigs_batch batch;

	// if the mutex times out we'll just return
	if (!read_bat_charge_dig_sigs_batch_with_retry(&batch))
		return;

	// NOTE: on the first time through, we want to check each of the batteries
	bool first_time_through = (old_bat_charging == -1);
	if (first_time_through)
	{
		for (int8_t bat = 0; bat < 4; bat++)
		{
			check_fault(bat, batch);
			check_chg(bat, bat == bat_charging, batch);
		}
	}
	else
	{
		check_fault(bat_charging, batch);
		check_chg(bat_charging, true, batch);

		check_fault(old_bat_charging, batch);
		check_chg(old_bat_charging, false, batch);
	}
}

void charge_lower_lf_bank(uint16_t lfb1_max_cell_mv, uint16_t lfb2_max_cell_mv)
{
	int8_t lf_charging = -1;

	// NOTE: it shouldn't be the case that both are full -- we'd be charging LI's
	if (get_lf_full(LFB1, lfb1_max_cell_mv))
	{
		lf_charging = LFB2;
	}
	else if (get_lf_full(LFB2, lfb2_max_cell_mv))
	{
		lf_charging = LFB1;
	}
	else
	{
		if (charging_data.bat_voltages[LFB1] <= charging_data.bat_voltages[LFB2])
			lf_charging = LFB1;
		else
			lf_charging = LFB2;
	}

	charging_data.bat_charging = lf_charging;
}

// NOTE: subtle case when the voltages are equivalent for LI1 and LI2
void charge_lower_li(void)
{
	if (charging_data.bat_voltages[LI1] <= charging_data.bat_voltages[LI2])
		charging_data.bat_charging = LI1;
	else
		charging_data.bat_charging = LI2;
}

void discharge_higher_li(void)
{
	if (charging_data.bat_voltages[LI1] <= charging_data.bat_voltages[LI2])
		charging_data.lion_discharging = LI2;
	else
		charging_data.lion_discharging = LI1;
}

void battery_charging_task(void *pvParameters)
{
	print("\n\nstarting battery charging (:\n");

	// delay to offset task relative to others, then start
	vTaskDelay(BATTERY_CHARGING_TASK_FREQ_OFFSET);
	TickType_t prev_wake_time = xTaskGetTickCount();

	// initialize key global variables
	init_charging_data();

	// initialize xNextWakeTime onces
	init_task_state(BATTERY_CHARGING_TASK); // suspend or run on boot (ALWAYS RUN!)

	while (true)
	{
		// report to watchdog
		report_task_running(BATTERY_CHARGING_TASK);

		// the core battery logic -- a separate function to make it easier to
		// unit test
		battery_logic();
		
		vTaskDelayUntil(&prev_wake_time, BATTERY_CHARGING_TASK_FREQ / portTICK_PERIOD_MS);
	}

	// delete this task if it ever breaks out
	vTaskDelete(NULL);
}

bool get_lf_full(int8_t lf, uint16_t max_cell_mv)
{
	return charging_data.bat_voltages[lf] > LF_FULL_SUM_MV ||
		   max_cell_mv > LF_FULL_MAX_MV;
}

bool get_lfs_both_full(
	uint8_t num_lf_down,
	int8_t good_lf,
	uint16_t lfb1_max_cell_mv,
	uint16_t lfb2_max_cell_mv)
{
	if (num_lf_down == 0)
		return get_lf_full(LFB1, lfb1_max_cell_mv) ||
			   get_lf_full(LFB2, lfb2_max_cell_mv);

	if (num_lf_down == 1)
		return get_lf_full(good_lf, good_lf == LFB1 ? lfb1_max_cell_mv : lfb2_max_cell_mv);

	return true;
}

void battery_logic()
{
	print("\n\nentering battery logic\n");

	///
	// phase prologue: updating relevant data in the charging data struct
	///

	#ifndef BAT_TESTING

	bool li_success = false;
	for (int i = 0; i < RETRIES_AFTER_MUTEX_TIMEOUT && !li_success; i++)
		li_success = read_lion_volts_precise(
			(uint16_t *) &(charging_data.bat_voltages[LI1]),
			(uint16_t *) &(charging_data.bat_voltages[LI2]));

	if (!li_success)
	{
		// TODO: what to do here?
		log_error(ELOC_BAT_CHARGING, ECODE_BAT_LI_TIMEOUT, true);
	}

	// individual batteries within the life po banks
	uint16_t lf1_mv;
	uint16_t lf2_mv;
	uint16_t lf3_mv;
	uint16_t lf4_mv;
	bool lf_success = false;
	for (int i = 0; i < RETRIES_AFTER_MUTEX_TIMEOUT && !lf_success; i++)
		lf_success = read_lifepo_volts_precise(&lf1_mv, &lf2_mv, &lf3_mv, &lf4_mv);

	if (!lf_success)
	{
		// TODO: what to do here?
		log_error(ELOC_BAT_CHARGING, ECODE_BAT_LF_TIMEOUT, true);
	}

	// considering the voltage of the life po banks to be the sum of the cells for
	// our purposes
	charging_data.bat_voltages[LFB1] = lf1_mv + lf2_mv;
	charging_data.bat_voltages[LFB2] = lf3_mv + lf4_mv;

	uint16_t lfb1_max_cell_mv = Max(lf1_mv, lf2_mv);
	uint16_t lfb2_max_cell_mv = Max(lf3_mv, lf4_mv);

	print("voltage data:\n");
	print("\tli1: %d\n", charging_data.bat_voltages[LI1]);
	print("\tli2: %d\n", charging_data.bat_voltages[LI2]);
	print("\tlf1: %d\n", lf1_mv);
	print("\tlf2: %d\n", lf2_mv);
	print("\tlf3: %d\n", lf3_mv);
	print("\tlf4: %d\n", lf4_mv);
	#endif

	/////
	// phase 0: determine whether any batteries should be decommissioned or
	// recommissioned
	/////

	#ifndef BAT_TESTING
	#ifndef WITHOUT_DECOMMISION
	for (int8_t bat = 0; bat < 4; bat++)
	{
		if (charging_data.decommissioned[bat])
		{
			print("checking whether to recommission battery: %d\n", bat);
			bool recommissioned = check_for_recommission(bat);

			print("\tdecided: %d\n", recommissioned);
			if (recommissioned)
				log_error(get_error_loc(bat), ECODE_RECOMMISSION, 0);
		}
	}

	// NOTE: only going through LI's
	for (int8_t bat = 0; bat < 2; bat++)
	{
		// this should be taken care of, but just making sure
		if (!charging_data.decommissioned[bat])
		{
			// we have the expectation that both LI's will become full
			// the charging loop will get stuck otherwise
			// TODO: do we want to change this to use SNS
			if ((get_current_timestamp_wrapped() - charging_data.li_last_full_or_recommissioned_timestamp[bat])
					 > MAX_TIME_WITHOUT_FULL_MS)
			{
				print("decommissioning battery: %d because of long time without full\n", bat);
				print("\twas full at %d, now %d\n", charging_data.li_last_full_or_recommissioned_timestamp[bat], get_current_timestamp_wrapped());
				log_error(get_error_loc(bat), ECODE_BAT_NOT_FULL_FOR_WHILE, 1);
				decommission(bat);
				continue;
			}

			// making sure the batteries haven't been below a battery threshold for
			// too long
			if (charging_data.bat_voltages[bat] <= LI_CRITICAL_MV)
			{
				if (charging_data.li_entered_low_voltage_timestamp[bat] == -1)
				{
					charging_data.li_entered_low_voltage_timestamp[bat] = get_current_timestamp_wrapped();
				}
				else if (get_current_timestamp_wrapped() - charging_data.li_entered_low_voltage_timestamp[bat] >
								 MAX_TIME_BELOW_V_THRESHOLD_S)
				{
					// we should reset the low voltage timestamp, so this battery will get a chance
					// next time -- this is done in the decommission function that will also take
					// care of other things like this
					print("decommissioning battery: %d because of long time at low voltage\n", bat);
					print("\tentered low voltage at %d, now %d\n", charging_data.li_entered_low_voltage_timestamp[bat], get_current_timestamp_wrapped());
					log_error(get_error_loc(bat), ECODE_BAT_LOW_VOLTAGE_FOR_WHILE, 1);
					decommission(bat);
					continue;
				}
			}
			else
			{
				charging_data.li_entered_low_voltage_timestamp[bat] = -1;
			}
		}
	}
	#endif
	#endif

	// drawing metadata about the state of the batteries
	uint8_t num_li_down = charging_data.decommissioned[LI1] + charging_data.decommissioned[LI2];
	uint8_t num_lf_down = charging_data.decommissioned[LFB1] + charging_data.decommissioned[LFB2];

	int8_t good_li = -1;
	if (num_li_down == 1)
		good_li = charging_data.decommissioned[LI1] ? LI2 : LI1;

	int8_t good_lf = -1;
	if (num_lf_down == 1)
		good_lf = charging_data.decommissioned[LFB1] ? LFB2 : LFB1;

	print("currently have %d bad li and %d bad lf\n", num_li_down, num_lf_down);

	#ifndef BAT_TESTING
	// we're interested in the charging battery filling up if it's a lion
	bool curr_charging_filled_up = false;
	if (is_lion(charging_data.bat_charging))
	{
		print("currently charging li %d, will check to see if it's full\n", charging_data.bat_charging);

		// we can't call the battery full from CHGN if we don't know CHGN
		bat_charge_dig_sigs_batch batch;
		bool dig_sigs_read_success = read_bat_charge_dig_sigs_batch_with_retry(&batch);
		print("\tchg pin active: %d\n", chg_pin_active(charging_data.bat_charging, batch));

		// we can't call the battery full from CHGN if we don't know PANEL_REF
		int panel_ref_val = get_panel_ref_val_with_retry();
		print("\tpanel ref: %d\n", panel_ref_val);

		// TODO: do we want this to be based on SNS
		curr_charging_filled_up =
									(dig_sigs_read_success && panel_ref_val != -1 &&
									!chg_pin_active(charging_data.bat_charging, batch) &&
								  charging_data.bat_voltages[charging_data.bat_charging] > LI_FULL_SANITY_MV &&
								  panel_ref_val >= PANEL_REF_SUN_MV) ||
								  charging_data.bat_voltages[charging_data.bat_charging] > LI_FULL_MV;

		print("\tdecided: %d\n", curr_charging_filled_up);
		if (curr_charging_filled_up)
			charging_data.li_last_full_or_recommissioned_timestamp[charging_data.bat_charging] = get_current_timestamp_wrapped();
	}
	#endif

	// reading simulated values -- some of the simulated values will already
	// have been put into the charging_data struct
	#ifdef BAT_TESTING
	bool curr_charging_filled_up = simulated_curr_charging_filled_up;
	#endif

	// we also want to know whether the life po banks have filled up
	bool life_pos_filled_up = get_lfs_both_full(num_lf_down, good_lf, lfb1_max_cell_mv, lfb2_max_cell_mv);

	// updating externally facing variables
	if (num_li_down == 0)
	{
		if (charging_data.bat_voltages[LI1] > LI_FULL_SANITY_MV && charging_data.bat_voltages[LI2] > LI_FULL_SANITY_MV)
			charging_data.should_move_to_antenna_deploy = 1;
		else
			charging_data.should_move_to_antenna_deploy = 0;
	}
	else if (num_li_down == 1)
	{
		if (charging_data.bat_voltages[good_li] > LI_FULL_SANITY_MV)
			charging_data.should_move_to_antenna_deploy = 1;
		else
			charging_data.should_move_to_antenna_deploy = 0;
	}
	else
	{
		charging_data.should_move_to_antenna_deploy = 1;
	}

	for (int8_t bat = 0; bat < 4; bat++)
	{
		if (!charging_data.already_set_sat_state[bat])
		{
			uint16_t threshold = is_lion(bat) ? LI_FULL_SANITY_MV : LF_FULL_SANITY_MV;
			if (charging_data.bat_voltages[bat] > threshold)
			{
				switch (bat)
				{
					case LI1:
						update_sat_event_history(0, 1, 0, 0, 0, 0, 0);
						break;

					case LI2:
						update_sat_event_history(0, 0, 1, 0, 0, 0, 0);
						break;

					case LFB1:
						update_sat_event_history(0, 0, 0, 1, 0, 0, 0);
						break;

					case LFB2:
						update_sat_event_history(0, 0, 0, 0, 1, 0, 0);
						break;

					default:
						configASSERT(false);
						log_error(ELOC_BAT_CHARGING_SWITCH_5, ECODE_UNEXPECTED_CASE, 1);
						break;
				}
			}
		}
	}

	/////
	// phase 1: determine whether we want to make a change to the charge state
	/////

	// the flow is as follows in the all_good state:
	// the default is to stay in a holding pattern, always charging the lion with
	//   the lower voltage
	// if we decide to eventually charge life po's, we get a little more aggresive,
	//   trying to systematically fill up both lion's
	// finally, when we do fill up the lion's we move to charging life po's --
	//   until one of the lion's drops below a low threshold

	// we make common-sense adjustments for the other states (detailed in the docs)

	meta_charge_state_t new_meta_charge_state = 0;
	if (num_li_down == 0)
	{
		if (num_lf_down < 2)
		{
			// ALL_GOOD (A)
			new_meta_charge_state = ALL_GOOD;
		}
		else
		{
			// TWO_LF_DOWN (C)
			new_meta_charge_state = TWO_LF_DOWN;
		}
	}
	else if (num_li_down == 1)
	{
		if (num_lf_down < 2)
		{
			// ONE_LI_DOWN (B)
			new_meta_charge_state = ONE_LI_DOWN;
		}
		else
		{
			// TWO_LF_DOWN (C)
			new_meta_charge_state = TWO_LF_DOWN;
		}
	}
	else
	{
		// TWO_LI_DOWN (D)
		new_meta_charge_state = TWO_LI_DOWN;
	}

	print("decided on meta charge state: %d\n", new_meta_charge_state);

	if (new_meta_charge_state != charging_data.curr_meta_charge_state)
	{
		print("this is a new meta charge state\n");
		charging_data.curr_meta_charge_state = new_meta_charge_state;
		switch (charging_data.curr_meta_charge_state)
		{
			case ALL_GOOD:
				charging_data.curr_charge_state = FILL_LI_NEITHER_FULL_A;
				break;

			case ONE_LI_DOWN:
				charging_data.curr_charge_state = FILL_LI_B;
				break;

			case TWO_LF_DOWN:
				charging_data.curr_charge_state = FILL_LI_C;
				break;

			case TWO_LI_DOWN:
				charging_data.curr_charge_state = FILL_LI_D;
				break;
		}
	}
	// otherwise, let's see if there's a state change to be made within the current
	// meta-state
	else if (charging_data.curr_meta_charge_state == ALL_GOOD)
	{
		switch (charging_data.curr_charge_state)
		{
			// NOTE: the battery that's currently charging has filled up if the charging
			// isn't running -- when the charge pin is inactive
			// on the first time through, curr_charging_filled_up will be false, so all good!

			case FILL_LI_NEITHER_FULL_A:
				// we only want to make a move when both batteries are good -- this is
				// like our default state
				if (charging_data.bat_voltages[LI1] > LI_DOWN_MV &&
					  charging_data.bat_voltages[LI2] > LI_DOWN_MV)
				{
					if (charging_data.bat_charging == LI1 && curr_charging_filled_up)
						charging_data.curr_charge_state = FILL_LI_LI1_FULL_A;
					else if (charging_data.bat_charging == LI2 && curr_charging_filled_up)
						charging_data.curr_charge_state = FILL_LI_LI2_FULL_A;
				}
				break;

			case FILL_LI_LI1_FULL_A:
				// going back takes precedence
				if (charging_data.bat_voltages[LI1] <= LI_DOWN_MV || charging_data.bat_voltages[LI2] <= LI_DOWN_MV)
					charging_data.curr_charge_state = FILL_LI_NEITHER_FULL_A;
				else if (charging_data.bat_charging == LI2 && curr_charging_filled_up)
					charging_data.curr_charge_state = FILL_LF_A;
				break;

			case FILL_LI_LI2_FULL_A:
				// going back takes precedence
				if (charging_data.bat_voltages[LI2] <= LI_DOWN_MV || charging_data.bat_voltages[LI1] <= LI_DOWN_MV)
					charging_data.curr_charge_state = FILL_LI_NEITHER_FULL_A;
				else if (charging_data.bat_charging == LI1 && curr_charging_filled_up)
					charging_data.curr_charge_state = FILL_LF_A;
				break;

			case FILL_LF_A:
				if ((charging_data.bat_voltages[LI1] <= LI_DOWN_MV ||
					charging_data.bat_voltages[LI2] <= LI_DOWN_MV) ||
					life_pos_filled_up)
					charging_data.curr_charge_state = FILL_LI_NEITHER_FULL_A;
				break;

			default:
				// trying to get past -Wswitch
				configASSERT(false);
				log_error(ELOC_BAT_CHARGING_SWITCH_6, ECODE_UNEXPECTED_CASE, 1);
				break;
		}
	}
	else if (charging_data.curr_meta_charge_state == ONE_LI_DOWN)
	{
		switch (charging_data.curr_charge_state)
		{
			// TODO*: add strike case if lifepos aren't charging for a long time
			case FILL_LI_B:
				if (charging_data.bat_charging == good_li && curr_charging_filled_up)
					charging_data.curr_charge_state = FILL_LF_B;
				break;

			case FILL_LF_B:
				if (charging_data.bat_voltages[good_li] <= LI_DOWN_MV || life_pos_filled_up)
					charging_data.curr_charge_state = FILL_LI_B;
				break;

			default:
				// trying to get past -Wswitch
				configASSERT(false);
				log_error(ELOC_BAT_CHARGING_SWITCH_7, ECODE_UNEXPECTED_CASE, 1);
				break;
		}
	}

	// NOTE: the other two meta states only have one charge state within them!!

	print("decided on charge state: %d\n", charging_data.curr_charge_state);

	/////
	// phase 2: determine which batteries should be charging
	/////

	// this is generally dictated by wanting the two lion's or the two lifepo banks
	// to be balanced as they charged and discharged

	// therefore, in accordance with the charging state, the batteries with lower
	// voltages will be charged, and those with higher voltages will be discharged

	// keeping track of the battery currently charging and the LI currently
	// discharging
	int8_t old_bat_charging = charging_data.bat_charging;
	int8_t old_li_discharging = charging_data.lion_discharging;

	if (charging_data.curr_meta_charge_state == ALL_GOOD)
	{
		switch (charging_data.curr_charge_state)
		{
			case FILL_LI_NEITHER_FULL_A:
				// charge the lion with the the lower voltage
				// discharge the lion with the higher voltage
				charge_lower_li();
				discharge_higher_li();
				break;

			case FILL_LI_LI1_FULL_A:
				charging_data.bat_charging = LI2;
				charging_data.lion_discharging = LI1;
				break;

			case FILL_LI_LI2_FULL_A:
				charging_data.bat_charging = LI1;
				charging_data.lion_discharging = LI2;
				break;

			case FILL_LF_A:
				// charge the life po bank with the lower max percentage
				// NOTE: this might lead to issues if the cells are very
				// unbalanced
				if (num_lf_down == 1)
				{
					charging_data.bat_charging = good_lf;
				}
				else
				{
					charge_lower_lf_bank(lfb1_max_cell_mv, lfb2_max_cell_mv);
				}

				// discharge the lion with the higher voltage
				discharge_higher_li();
				break;

			default:
				// trying to get past -Wswitch
				configASSERT(false);
				log_error(ELOC_BAT_CHARGING_SWITCH_8, ECODE_UNEXPECTED_CASE, 1);
				break;
		}
	}
	else if (charging_data.curr_meta_charge_state == ONE_LI_DOWN)
	{
		switch (charging_data.curr_charge_state)
		{
			case FILL_LI_B:
				charging_data.bat_charging = good_li;
				charging_data.lion_discharging = good_li;
				break;

			case FILL_LF_B:
				// charge the life po bank with the lower max percentage
				// NOTE: this might lead to issues if the cells are very
				// unbalanced
				if (num_lf_down == 1)
				{
					charging_data.bat_charging = good_lf;
				}
				else
				{
					charge_lower_lf_bank(lfb1_max_cell_mv, lfb2_max_cell_mv);
				}

				charging_data.lion_discharging = good_li;
				break;

			default:
				// trying to get past -Wswitch
				configASSERT(false);
				log_error(ELOC_BAT_CHARGING_SWITCH_9, ECODE_UNEXPECTED_CASE, 1);
				break;
		}
	}
	else if (charging_data.curr_meta_charge_state == TWO_LF_DOWN)
	{
		// NOTE: only one sub-state here!
		if (num_li_down == 0)
		{
			charge_lower_li();
			discharge_higher_li();
		}
		else // if (num_li_down == 1)
		{
			charging_data.bat_charging = good_li;
			charging_data.lion_discharging = good_li;
		}
	}
	else // if (charging_data.curr_meta_charge_state == TWO_LI_DOWN)
	{
		if (charging_data.charging_parity)
		{
			charging_data.bat_charging = LI1;
			charging_data.lion_discharging = -1;
		}
		else
		{
			charging_data.bat_charging = LI2;
			charging_data.lion_discharging = -1;
		}

		charging_data.charging_parity = !charging_data.charging_parity;
	}

	print("decided on battery charging: %d\n", charging_data.bat_charging);
	print("decided on li discharging: %d\n", charging_data.lion_discharging);

	/////
	// phase 3: apply the decisions we've made about which batteries to charge!
	/////

	#ifndef BAT_TESTING
	bool got_mutex = true;
	if (!xSemaphoreTake(critical_action_mutex, (TickType_t) CRITICAL_MUTEX_WAIT_TIME_TICKS)) {
		// if for some reason we can't get the bat charging mutex (it times out),
		// ignore it and move on (the only things this mutex prevents is flashing while
		// lifepos are charging, which is less worrisome than not running charging logic)
		log_error(ELOC_BAT_CHARGING, ECODE_CRIT_ACTION_MUTEX_TIMEOUT, true);
		got_mutex = false;
	}

	///
	// phase 3a: set the lion that should be discharging to discharge and
	//  set the other lion to not discharge
	///

	if (charging_data.curr_meta_charge_state == TWO_LI_DOWN)
	{
		set_li_to_discharge(LI1, 1);
		set_li_to_discharge(LI2, 1);
	}
	else if (charging_data.lion_discharging != old_li_discharging)
	{
		// we also need to make changes with respect to lion that isn't discharging
		int8_t lion_not_discharging = (charging_data.lion_discharging == LI1) ? LI2 : LI1;

		// NOTE: very important to set the discharging pin to active before setting
		// the other to inactive
		set_li_to_discharge(charging_data.lion_discharging, 1);

		// take the SPI mutex because we'll be using it AND we don't
		// want the satellite to reboot from power failure while writing to the MRAM
		// elsewhere. We increase the timeout hear to be more confident we get it.
		// (if it fails, continue on, and don't write to the MRAM)
		bool got_mutex_spi = false;
		if (xSemaphoreTake(mram_spi_cache_mutex, 2 * MRAM_SPI_MUTEX_WAIT_TIME_TICKS)) {
			got_mutex_spi = true;
		} else {
			log_error(ELOC_BAT_CHARGING, ECODE_SPI_MUTEX_TIMEOUT, true);
		}



		// TODO: maybe give up on this check if we fail to get the mutex?? ROHAN??



		// write to the MRAM that the battery caused a reboot, in case does
		// (we'll deal with this when we reboot)
		persistent_charging_data_t persist_data;
		persist_data.li_caused_reboot = charging_data.lion_discharging;
		if (got_mutex_spi) set_persistent_charging_data_unsafe(persist_data);

		set_li_to_discharge(lion_not_discharging, 0);
		// wait a sufficiently long time that we think the satellite would've rebooted
		// if this battery had failed
		vTaskDelay(SAT_NO_POWER_TURN_OFF_T_MS / portTICK_PERIOD_MS);

		// TODO: check this flow with Mckenna -- enough delay?
		check_after_discharging(charging_data.lion_discharging, lion_not_discharging);

		// reset our emergency write to the MRAM
		persist_data.li_caused_reboot = -1;
		if (got_mutex_spi) set_persistent_charging_data_unsafe(persist_data);

		// resume normal operation
		if (got_mutex_spi) xSemaphoreGive(mram_spi_cache_mutex);
	}

	if (got_mutex) xSemaphoreGive(critical_action_mutex);
	#endif

	///
	// phase 3b: set the battery that should be charging to charge and
	//  set the others to not charge
	///

	if (old_bat_charging == -1)
	{
		for (int8_t bat = 0; bat < 4; bat++)
			set_bat_to_charge(bat, bat == charging_data.bat_charging);
	}
	else if (charging_data.bat_charging != old_bat_charging)
	{
		set_bat_to_charge(old_bat_charging, 0);
		set_bat_to_charge(charging_data.bat_charging, 1);
	}

	vTaskDelay(WAIT_TIME_BEFORE_PIN_CHECK_MS / portTICK_PERIOD_MS);
	check_after_charging(charging_data.bat_charging, old_bat_charging);

	print("leaving battery charging\n");
}
