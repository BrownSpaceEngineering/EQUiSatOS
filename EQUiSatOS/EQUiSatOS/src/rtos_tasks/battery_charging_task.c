/*
 * battery_charging_task.c
 *
 * Created: 9/21/2017 20:37:56
 *  Author: rjha
 */

#include "battery_charging_task.h"

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
	#ifdef BAT_UNIT_TESTING
	return simulated_timestamp;
	#else
	return get_current_timestamp();
	#endif
}

sat_state_t get_sat_state_wrapped(void)
{
	#ifdef BAT_UNIT_TESTING
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
			log_error(ELOC_BAT_CHARGING_SWITCH_1, ECODE_UNEXPECTED_CASE, false);
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
			log_error(ELOC_BAT_CHARGING_SWITCH_2, ECODE_UNEXPECTED_CASE, false);
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
			log_error(ELOC_BAT_CHARGING_SWITCH_3, ECODE_UNEXPECTED_CASE, false);
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
			log_error(ELOC_BAT_CHARGING_SWITCH_4, ECODE_UNEXPECTED_CASE, false);
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
	for (int8_t i = 0; i < RETRIES_AFTER_MUTEX_TIMEOUT && !success; i++)
		success = read_ad7991_batbrd_precise(four_buf);

	if (!success)
		return ((uint16_t) -1);

	return ((uint16_t)four_buf[2]);
}

static uint16_t get_sns_val_with_retry(int8_t bat)
{
	uint16_t four_buf[4];
	bool success = false;
	for (int8_t i = 0; i < RETRIES_AFTER_MUTEX_TIMEOUT && !success; i++)
		success = read_ad7991_batbrd_precise(four_buf);

	if (!success)
		return ((uint16_t) -1);

	return ((uint16_t)four_buf[bat == LI1 ? 1 : 0]);
}

bool read_bat_charge_dig_sigs_batch_with_retry(bat_charge_dig_sigs_batch *batch)
{
	bool success = false;
	for (int8_t i = 0; i < RETRIES_AFTER_MUTEX_TIMEOUT && !success; i++)
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
	print("\t%d total decommissions for this bat\n", charging_data.decommissioned_count[bat]);
	print("\tthis battery should be decommissioned for %d\n", time_for_recommission(bat));
	if (time_since_decommission > time_for_recommission(bat))
	{
		charging_data.decommissioned[bat] = 0;

		if (is_lion(bat))
		{
			// we also need to do various things to make sure that the battery won't be
			// immediately decommissioned again
			charging_data.decommissioned_timestamp[bat] = 0; // NOTE: this doesn't do much but keeps the struct consistent
			charging_data.li_entered_low_voltage_timestamp[bat] = ((uint32_t) -1);
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

	// set low voltage timestamps to -1
	charging_data.li_entered_low_voltage_timestamp[0] = ((uint32_t) -1);
	charging_data.li_entered_low_voltage_timestamp[1] = ((uint32_t) -1);

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
	for (int8_t bat = 0; bat < 4; bat++)
	{
		charging_data.decommissioned[bat] = 0;
		
		// NOTE: this is okay -- only relevant when really decommissioned, at which point it will be set
		charging_data.decommissioned_timestamp[bat] = 0;
		charging_data.decommissioned_count[bat] = 0;
	}

	charging_data.charging_parity = 0;

	#ifndef BAT_UNIT_TESTING
	// we need to decommission here if the MRAM says that one of the LI's caused
	// a reboot
	persistent_charging_data_t persist_data = cache_get_persistent_charging_data();
	if (persist_data.li_caused_reboot == LI1 || persist_data.li_caused_reboot == LI2)
	{
		print("mram has battery %d as having caused a reboot -- decommissioning\n", persist_data.li_caused_reboot);
		log_error(get_error_loc(persist_data.li_caused_reboot), ECODE_BAT_NOT_DISCHARGING_RESTART, true);
		decommission(persist_data.li_caused_reboot);

		// reset persistent data so we give another chance
		persist_data.li_caused_reboot = ~0;
		set_persistent_charging_data_unsafe(persist_data);
	}
	#endif

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
		equisim_set_action_by_pin(discharge, discharge_pin);
	#endif

	print("set li %d to discharge: %d\n", bat, discharge);
}

void check_discharging_with_retry(int8_t bat_discharging, bat_charge_dig_sigs_batch batch)
{
	print("\tchecking bat %d is discharging\n", bat_discharging);
	bool bat_discharging_st_pin_active = st_pin_active(bat_discharging, batch);
	if (!bat_discharging_st_pin_active)
	{
		print("\t\tbat %d not discharging -- checking again\n", bat_discharging);
		vTaskDelay(DELAY_BEFORE_RETRY_MS / portTICK_PERIOD_MS);

		bat_charge_dig_sigs_batch new_batch;
		if (!read_bat_charge_dig_sigs_batch_with_retry(&new_batch))
		{
			print("\t\tmutex timed out for dig sigs batch -- abandoning check\n");
			return;
		}

		bool new_bat_discharging_st_pin_active = st_pin_active(bat_discharging, new_batch);
		if (!new_bat_discharging_st_pin_active)
		{
			print("\t\tbad result again -- decommissioning\n");
			log_error(get_error_loc(bat_discharging), ECODE_BAT_NOT_DISCHARGING, true);
			decommission(bat_discharging);
		}
		else
		{
			print("\t\tall good on second try\n");
		}
	}
	else
	{
		print("\t\tall good\n");
	}
}

void check_not_discharging_with_retry(int8_t bat_not_discharging, bat_charge_dig_sigs_batch batch)
{
	print("\tchecking bat %d is not discharging\n", bat_not_discharging);
	bool bat_not_discharging_st_pin_active = st_pin_active(bat_not_discharging, batch);
	if (bat_not_discharging_st_pin_active)
	{
		print("\t\tbat %d still discharging -- checking again\n", bat_not_discharging);
		vTaskDelay(DELAY_BEFORE_RETRY_MS / portTICK_PERIOD_MS);

		bat_charge_dig_sigs_batch new_batch;
		if (!read_bat_charge_dig_sigs_batch_with_retry(&new_batch))
		{
			print("\t\tmutex timed out for dig sigs batch -- abandoning check\n");
			return;
		}

		bool new_bat_not_discharging_st_pin_active = st_pin_active(bat_not_discharging, new_batch);
		if (new_bat_not_discharging_st_pin_active)
		{
			print("\t\tbad result again -- decommissioning\n");
			log_error(get_error_loc(bat_not_discharging), ECODE_BAT_NOT_NOT_DISCHARGING, true);
			decommission(bat_not_discharging);
		}
		else
		{
			print("\t\tall good on second try\n");
		}
	}
	else
	{
		print("\t\tall good\n");
	}
}

// NOTE: we should only get here if we switched the batteries we're discharging
void check_after_discharging(int8_t bat_discharging, int8_t bat_not_discharging)
{
	print("checking after discharge:\n");
	bat_charge_dig_sigs_batch batch;

	// if the mutex times out, we'll just return
	if (!read_bat_charge_dig_sigs_batch_with_retry(&batch))
		return;

	check_discharging_with_retry(bat_discharging, batch);
	check_not_discharging_with_retry(bat_not_discharging, batch);
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

void check_fault_with_retry(int8_t bat, bat_charge_dig_sigs_batch batch)
{
	print("\tchecking bat %d for fault\n", bat);
	if (fault_pin_active(bat, batch))
	{
		print("\t\tfault pin active for bat %d -- will retry\n", bat);
		vTaskDelay(DELAY_BEFORE_RETRY_MS / portTICK_PERIOD_MS);

		// if the mutex times out we'll just return
		bat_charge_dig_sigs_batch new_batch;
		if (!read_bat_charge_dig_sigs_batch_with_retry(&new_batch))
		{
			print("\t\tmutex timed out for dig sigs batch -- abandoning check\n");
			return;
		}

		if (fault_pin_active(bat, new_batch))
		{
			print("\t\tfault pin again active for bat %d -- decommissioning\n", bat);
			log_error(get_error_loc(bat), ECODE_BAT_FAULT, true);
			decommission(bat);
		}
		else
		{
			print("\t\tall good on the second time\n");
		}
	}
	else
	{
		print("\t\tall good\n");
	}
}

// retry this if bad value?
// NOTE: returns true if should decommission
bool check_chg_should_decommission(int8_t bat, bool should_be_charging, bat_charge_dig_sigs_batch batch)
{
	bool charge_running = chg_pin_active(bat, batch);
	if (should_be_charging)
	{
		print("\tchecking is bat %d is charging:\n", bat);
		print("\t\tchg pin active: %d\n", chg_pin_active(bat, batch));
		print("\t\tpanel ref: %d\n", get_panel_ref_val_with_retry());

		// we can't make any claims if we don't know anything about PANEL_REF
		uint16_t panel_ref_val = get_panel_ref_val_with_retry();
		if (panel_ref_val == ((uint16_t) -1))
			return false;

		if (!charge_running && (panel_ref_val > PANEL_REF_SUN_MV) &&
			(charging_data.bat_voltages[bat] < (is_lion(bat) ? LI_MIGHT_NOT_BE_FULL_MV : LF_MIGHT_NOT_BE_FULL_MV)))
		{
			print("\t\tnot charging for bat %d\n", bat);
			return true;
		}
		else
		{
			print("\t\tall good!\n");
			return false;
		}
	}
	else
	{
		print("\tchecking is bat %d not charging:\n", bat);
		if (charge_running)
		{
			print("\t\tstill charging for bat %d -- error\n", bat);
			log_error(get_error_loc(bat), ECODE_BAT_NOT_NOT_CHARGING, true);
		}
		else
		{
			print("\t\tall good!\n");
		}

		return false;
	}
}

void check_chg_with_retry(int8_t bat, bool should_be_charging, bat_charge_dig_sigs_batch batch)
{
	if (check_chg_should_decommission(bat, should_be_charging, batch))
	{
		print("\tbad result -- retrying\n");
		vTaskDelay(DELAY_BEFORE_RETRY_MS / portTICK_PERIOD_MS);

		// if the mutex times out we'll just return
		bat_charge_dig_sigs_batch new_batch;
		if (!read_bat_charge_dig_sigs_batch_with_retry(&new_batch))
		{
			print("\t\tmutex timed out for dig sigs batch -- abandoning check\n");
			return;
		}

		if (check_chg_should_decommission(bat, should_be_charging, new_batch))
		{
			print("\t\tbad result again for bat %d -- decommissioning\n", bat);
			log_error(get_error_loc(bat), ECODE_BAT_NOT_CHARGING, true);
			decommission(bat);
		}
		else
		{
			print("\t\tall good on the second time\n");
		}
	}
}

void check_after_charging(int8_t bat_charging, int8_t old_bat_charging)
{
	print("checking after charging\n");
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
			check_fault_with_retry(bat, batch);
			check_chg_with_retry(bat, bat == bat_charging, batch);
		}
	}
	else
	{
		check_fault_with_retry(bat_charging, batch);
		check_chg_with_retry(bat_charging, true, batch);

		check_fault_with_retry(old_bat_charging, batch);
		check_chg_with_retry(old_bat_charging, false, batch);
	}
}

void charge_lower_lf_bank(uint16_t lfb1_max_cell_mv, uint16_t lfb2_max_cell_mv)
{
	#ifndef BAT_UNIT_TESTING
	int8_t lf_charging = -1;
	
	bat_charge_dig_sigs_batch batch;
	bool got_batch = !read_bat_charge_dig_sigs_batch_with_retry(&batch);

	// NOTE: it shouldn't be the case that both are full -- we'd be charging LI's
	if (get_lf_full(charging_data.bat_voltages[LFB1], lfb1_max_cell_mv, LFB1, batch, got_batch))
	{
		lf_charging = LFB2;
	}
	else if (get_lf_full(charging_data.bat_voltages[LFB2], lfb2_max_cell_mv, LFB2, batch, got_batch))
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
	#endif
	
	#ifdef BAT_UNIT_TESTING
	int8_t lf_charging = -1;
	if (charging_data.bat_voltages[LFB1] <= charging_data.bat_voltages[LFB2])
		lf_charging = LFB1;
	else
		lf_charging = LFB2;
		
	charging_data.bat_charging = lf_charging;
	#endif
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

void update_should_deploy_antenna(bool has_li_data)
{
	print("checking antenna deploy\n");
	
	bool li_success = false;
	uint16_t li1_mv = 0;
	uint16_t li2_mv = 0;
	if (!has_li_data)
	{
		for (int8_t i = 0; i < RETRIES_AFTER_MUTEX_TIMEOUT && !li_success; i++)
			li_success = read_lion_volts_precise((uint16_t *) &li1_mv, (uint16_t *) &li2_mv, true);
	}
	else
	{
		li1_mv = charging_data.bat_voltages[LI1];
		li2_mv = charging_data.bat_voltages[LI2];
		li_success = true;
	}
	
	uint8_t num_li_down = charging_data.decommissioned[LI1] + charging_data.decommissioned[LI2];
	int8_t good_li = -1;
	if (num_li_down == 1)
		good_li = charging_data.decommissioned[LI1] ? LI2 : LI1;
	
	// if we don't have any good data about li's, we'll take the chance and move
	// to antenna deploy
	if (!li_success)
	{
		charging_data.should_move_to_antenna_deploy = 1;
	}
	else
	{
		if (num_li_down == 0)
		{
			if (charging_data.bat_voltages[LI1] > LI_FULL_ANTENNA_DEPLOY_MV && charging_data.bat_voltages[LI2] > LI_FULL_ANTENNA_DEPLOY_MV)
				charging_data.should_move_to_antenna_deploy = 1;
			else
				charging_data.should_move_to_antenna_deploy = 0;
		}
		else if (num_li_down == 1)
		{
			if (charging_data.bat_voltages[good_li] > LI_FULL_ANTENNA_DEPLOY_MV)
				charging_data.should_move_to_antenna_deploy = 1;
			else
				charging_data.should_move_to_antenna_deploy = 0;
		}
		else
		{
			charging_data.should_move_to_antenna_deploy = 1;
		}
	}

	print("\tdecided: %d\n", charging_data.should_move_to_antenna_deploy);
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

	uint8_t iter_count = 0;
	while (true)
	{
		// report to watchdog
		report_task_running(BATTERY_CHARGING_TASK);
		
		print("entering battery task!\n");
		if (iter_count % BAT_CHARGING_ITERS_UNTIL_FULL == 0 || get_sat_state_wrapped() == LOW_POWER)
		{
			// the core battery logic -- a separate function to make it easier to
			// unit test
			// there's an update to should_deploy_antenna within battery logic
			battery_logic();
			iter_count = 0;
		}
		else
		{
			print("not running the full logic yet: iter at %d\n", iter_count);
			update_should_deploy_antenna(false);
		}
		
		iter_count++;
		vTaskDelayUntil(&prev_wake_time, BATTERY_CHARGING_TASK_FREQ / portTICK_PERIOD_MS);
	}

	// delete this task if it ever breaks out
	vTaskDelete(NULL);
}

bool get_lf_full(
	uint16_t sum_cell_mv, 
	uint16_t max_cell_mv, 
	int8_t lf, 
	bat_charge_dig_sigs_batch batch, 
	bool got_batch)
{	
	uint16_t panel_ref_val = get_panel_ref_val_with_retry();
	bool got_panel_ref = panel_ref_val != ((uint16_t) -1);

	return sum_cell_mv > LF_FULL_SUM_MV ||
		   max_cell_mv > LF_FULL_MAX_MV ||
		   (charging_data.bat_charging == lf && got_panel_ref && got_batch &&
		    !chg_pin_active(lf, batch) && (panel_ref_val > PANEL_REF_SUN_MV) &&
		  	sum_cell_mv > LF_FULL_SANITY_MV);
}

bool get_lfs_both_full(
	uint8_t num_lf_down,
	int8_t good_lf,
	uint16_t lfb1_max_cell_mv,
	uint16_t lfb2_max_cell_mv)
{
	bat_charge_dig_sigs_batch batch;
	bool got_batch = !read_bat_charge_dig_sigs_batch_with_retry(&batch);
	
	if (num_lf_down == 0)
		return get_lf_full(charging_data.bat_voltages[LFB1], lfb1_max_cell_mv, LFB1, batch, got_batch) &&
			   get_lf_full(charging_data.bat_voltages[LFB2], lfb2_max_cell_mv, LFB2, batch, got_batch);

	if (num_lf_down == 1)
		return get_lf_full(charging_data.bat_voltages[good_lf], good_lf == LFB1 ? lfb1_max_cell_mv : lfb2_max_cell_mv, good_lf, batch, got_batch);

	return true;
}

void battery_logic()
{
	print("\n\nentering full battery logic\n");

	///
	// phase prologue: updating relevant data in the charging data struct
	///

	#ifdef BAT_UNIT_TESTING
	bool li_success = true;
	bool lf_success = true;
	uint16_t lfb1_max_cell_mv = charging_data.bat_voltages[LFB1] / 2;
	uint16_t lfb2_max_cell_mv = charging_data.bat_voltages[LFB2] / 2;
	#endif

	#ifndef BAT_UNIT_TESTING
	bool li_success = false;
	for (int8_t i = 0; i < RETRIES_AFTER_MUTEX_TIMEOUT && !li_success; i++)
		li_success = read_lion_volts_precise(
			(uint16_t *) &(charging_data.bat_voltages[LI1]),
			(uint16_t *) &(charging_data.bat_voltages[LI2]), true);

	// we're going to move to TWO_LI_DOWN -- but not decommission anyone
	if (!li_success)
	{
		print("mutex timed out before we got good LI readings -- bad!\n");
		log_error(ELOC_BAT_CHARGING, ECODE_BAT_LI_TIMEOUT, true);
	}

	// individual batteries within the life po banks
	uint16_t lf1_mv;
	uint16_t lf2_mv;
	uint16_t lf3_mv;
	uint16_t lf4_mv;
	bool lf_success = false;
	for (int8_t i = 0; i < RETRIES_AFTER_MUTEX_TIMEOUT && !lf_success; i++)
		lf_success = read_lifepo_volts_precise(&lf1_mv, &lf2_mv, &lf3_mv, &lf4_mv, true);

	if (!lf_success)
	{
		print("mutex timed out before we got good LF readings -- bad!\n");
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

	#ifndef BAT_UNIT_TESTING
	// it's alright to recommission, even with bad voltage values
	for (int8_t bat = 0; bat < 4; bat++)
	{
		if (charging_data.decommissioned[bat])
		{
			print("checking whether to recommission battery: %d\n", bat);
			bool recommissioned = check_for_recommission(bat);

			print("\tdecided: %d\n", recommissioned);
			if (recommissioned)
				log_error(get_error_loc(bat), ECODE_RECOMMISSION, true);
		}
	}

	// only going through this if we got good LI readings
	if (li_success)
	{
		// NOTE: only going through LI's
		for (int8_t bat = 0; bat < 2; bat++)
		{
			// this should be taken care of, but just making sure
			if (!charging_data.decommissioned[bat])
			{
				// making sure the batteries haven't been below a battery threshold for
				// too long
				if (charging_data.bat_voltages[bat] <= LI_CRITICAL_MV)
				{
					if (charging_data.li_entered_low_voltage_timestamp[bat] == ((uint32_t) -1))
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
						log_error(get_error_loc(bat), ECODE_BAT_LOW_VOLTAGE_FOR_WHILE, true);
						decommission(bat);
						continue;
					}
				}
				else
				{
					charging_data.li_entered_low_voltage_timestamp[bat] = ((uint32_t) -1);
				}
			}
		}
	}
	
	// not a decommissioning case, but will log errors if very unbalanced
	if (lf_success)
	{
		if (lfb1_max_cell_mv > LF_FULL_MAX_MV)
		{
			log_error(get_error_loc(LFB1), ECODE_BAT_LF_CELLS_UNBALANCED, true);
		}
		
		if (lfb2_max_cell_mv > LF_FULL_MAX_MV)
		{
			log_error(get_error_loc(LFB2), ECODE_BAT_LF_CELLS_UNBALANCED, true);
		}
	}
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

	// reading simulated values -- some of the simulated values will already
	// have been put into the charging_data struct
	#ifdef BAT_UNIT_TESTING
	bool curr_charging_filled_up = simulated_curr_charging_filled_up;
	#endif
	
	#ifndef BAT_UNIT_TESTING
	// we're interested in the charging battery filling up if it's a lion
	bool curr_charging_filled_up = false;

	// we can only do this if we got good li readings
	if (is_lion(charging_data.bat_charging) && li_success)
	{
		print("currently charging li %d, will check to see if it's full\n", charging_data.bat_charging);

		uint16_t sns_value = get_sns_val_with_retry(charging_data.bat_charging);
		bool got_sns_data = sns_value != ((uint16_t) -1);

		print("\tsns value: %d\n", sns_value);

		curr_charging_filled_up =
			(charging_data.bat_voltages[charging_data.bat_charging] > LI_FULL_LOW_MV &&
			 got_sns_data && sns_value < SNS_THRESHOLD_LOW) ||
			 (charging_data.bat_voltages[charging_data.bat_charging] > LI_FULL_LOWER_MV &&
			 got_sns_data && sns_value < SNS_THRESHOLD_LOWER) ||
			(charging_data.bat_voltages[charging_data.bat_charging] > LI_FULL_MV);

		print("\tdecided: %d\n", curr_charging_filled_up);
	}
	#endif

	#ifdef BAT_UNIT_TESTING
	bool life_pos_filled_up = simulated_lfs_both_filled_up;
	#endif

	#ifndef BAT_UNIT_TESTING
	// we also want to know whether the life po banks have filled up
	bool life_pos_filled_up = false;

	// this only makes sense if we have lf values
	if (lf_success)
		life_pos_filled_up = get_lfs_both_full(num_lf_down, good_lf, lfb1_max_cell_mv, lfb2_max_cell_mv);
	#endif

	// updating externally facing variables
	
	#ifndef BAT_UNIT_TESTING
	// updating antenna deploy
	// if there are good values in charging_data, we'll just use those
	// if not we'll try to grab new ones -- but this won't affect the current flow
	update_should_deploy_antenna(li_success);
	
	// updating sat state
	for (int8_t bat = 0; bat < 4; bat++)
	{
		// only valid if we have values
		if (!charging_data.already_set_sat_state[bat] && (is_lion(bat) ? li_success : lf_success))
		{
			uint16_t threshold = is_lion(bat) ? LI_FULL_LOWER_MV : LF_FULL_SANITY_MV;
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
						log_error(ELOC_BAT_CHARGING_SWITCH_5, ECODE_UNEXPECTED_CASE, false);
						break;
				}
			}
		}
	}
	#endif

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
	if (!li_success)
	{
		// we're going to act as if we have two li down
		new_meta_charge_state = TWO_LI_DOWN;
	}
	else
	{
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
				log_error(ELOC_BAT_CHARGING_SWITCH_6, ECODE_UNEXPECTED_CASE, false);
				break;
		}
	}
	else if (charging_data.curr_meta_charge_state == ONE_LI_DOWN)
	{
		switch (charging_data.curr_charge_state)
		{
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
				log_error(ELOC_BAT_CHARGING_SWITCH_7, ECODE_UNEXPECTED_CASE, false);
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
				log_error(ELOC_BAT_CHARGING_SWITCH_8, ECODE_UNEXPECTED_CASE, false);
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
				log_error(ELOC_BAT_CHARGING_SWITCH_9, ECODE_UNEXPECTED_CASE, false);
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
		// NOTE: only one sub-state here, too!
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

	#ifndef BAT_UNIT_TESTING
	bool got_mutex = true;
	if (!xSemaphoreTake(critical_action_mutex, (TickType_t) CRITICAL_MUTEX_WAIT_TIME_TICKS)) {
		// if for some reason we can't get the bat charging mutex (it times out),
		// ignore it and move on (the only things this mutex prevents is flashing while
		// lifepos are charging, which is less worrisome than not running charging logic)
		log_error(ELOC_BAT_CHARGING, ECODE_CRIT_ACTION_MUTEX_TIMEOUT, false);
		got_mutex = false;
	}

	///
	// phase 3a: set the lion that should be discharging to discharge and
	//  set the other lion to not discharge
	///

	if (charging_data.curr_meta_charge_state == TWO_LI_DOWN)
	{
		print("emergency discharging\n");
		set_li_to_discharge(LI1, 1);
		set_li_to_discharge(LI2, 1);
	}
	// NOTE: lion_discharging cannot be -1 at this point
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

		// write to the MRAM that the battery caused a reboot, in case does
		// (we'll deal with this when we reboot)
		persistent_charging_data_t persist_data;
		persist_data.li_caused_reboot = charging_data.lion_discharging;
		if (got_mutex_spi) set_persistent_charging_data_unsafe(persist_data);

		set_li_to_discharge(lion_not_discharging, 0);
		// wait a sufficiently long time that we think the satellite would've rebooted
		// if this battery had failed
		vTaskDelay(SAT_NO_POWER_TURN_OFF_T_MS / portTICK_PERIOD_MS);

		// reset our emergency write to the MRAM
		persist_data.li_caused_reboot = ~0;
		if (got_mutex_spi) set_persistent_charging_data_unsafe(persist_data);

		// resume normal operation
		if (got_mutex_spi) xSemaphoreGive(mram_spi_cache_mutex);
		
		// it's okay to check discharging even with bad voltages -- there aren't
		// any voltage based checks
		check_after_discharging(charging_data.lion_discharging, lion_not_discharging);
	}
	else
	{
		print("no change to discharging li's\n");
	}

	if (got_mutex) xSemaphoreGive(critical_action_mutex);

	///
	// phase 3b: set the battery that should be charging to charge and
	//  set the others to not charge
	///

	if (old_bat_charging == -1)
	{
		for (int8_t bat = 0; bat < 4; bat++)
			set_bat_to_charge(bat, bat == charging_data.bat_charging);

		// NOTE: no wait here but reading takes a while and will retry in worst case
		// NOTE: check after charging accounts for old_bat_charging as -1
		// only makes sense to do this if we got good voltage values
		if (li_success && lf_success)
			check_after_charging(charging_data.bat_charging, old_bat_charging);
	}
	else if (charging_data.bat_charging != old_bat_charging)
	{
		set_bat_to_charge(old_bat_charging, 0);
		set_bat_to_charge(charging_data.bat_charging, 1);

		// NOTE: no wait here but reading takes a while and will retry in worst case
		// only makes sense to do this if we got good voltage values
		if (li_success && lf_success)
			check_after_charging(charging_data.bat_charging, old_bat_charging);
	}
	else
	{
		print("no change to charging bats\n");
	}

	#endif

	print("leaving battery charging\n");
}
