/*
 * battery_charging_task.c
 *
 * Created: 9/21/2017 20:37:56
 *  Author: mcken
 */

#include "battery_charging_task.h"

// TODO: make sure that everything is okay after coming back from boot

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

int get_current_timestamp_wrapped(void)
{
	#ifdef BAT_TESTING
	return simulated_timestamp;
	#endif

	#ifndef BAT_TESTING
	return get_current_timestamp();
	#endif
}

sat_state_t get_sat_state_wrapped(void)
{
	#ifdef BAT_TESTING
	return simulated_state;
	#endif

	#ifndef BAT_TESTING
	return get_sat_state();
	#endif
}

int get_error_loc(battery_t bat)
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
			log_error(ELOC_BAT_CHARGING, ECODE_UNEXPECTED_CASE, true);
			return ELOC_BAT_CHARGING;
	}
}

int get_run_chg_pin(battery_t bat)
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
			log_error(ELOC_BAT_CHARGING, ECODE_UNEXPECTED_CASE, true);
			return P_L1_RUN_CHG;
	}
}

int get_run_dischg_pin(battery_t bat)
{
	return (bat == LI1) ? P_L1_DISG : P_L2_DISG;
}

// NOTE: returns the chg pin value with the conversion from active low
int get_chg_pin_val_w_conversion(battery_t bat)
{
	int chg_position;
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
			log_error(ELOC_BAT_CHARGING, ECODE_UNEXPECTED_CASE, true);
			chg_position = 12;
			break;
	}

	bat_charge_dig_sigs_batch batch;
	read_bat_charge_dig_sigs_batch(&batch);

	// NOTE: CHGN is active low, so we're making a conversion here
	return !((batch>>chg_position)&0x1);
}

int get_fault_pin_val_w_conversion(battery_t bat)
{
	int fault_position;
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
			log_error(ELOC_BAT_CHARGING, ECODE_UNEXPECTED_CASE, true);
			fault_position = 13;
			break;
	}

	bat_charge_dig_sigs_batch batch;
	read_bat_charge_dig_sigs_batch(&batch);

	// NOTE: FAULTN is active low, so we're making a conversion here
	return !((batch>>fault_position)&0x1);
}

int get_st_val(battery_t bat)
{
	int st_position = (bat == LI1) ? 9 : 8;
	bat_charge_dig_sigs_batch batch;
	read_bat_charge_dig_sigs_batch(&batch);
	return (batch>>st_position)&0x01;
}

int get_panel_ref_val()
{
	uint8_t four_buf[4];
	read_ad7991_batbrd(four_buf, four_buf+2); // TODO: are we worried about warning?
	return ((int) four_buf[2]<<8);
}

int is_lion(battery_t bat)
{
	return (bat == LI1 || bat == LI2);
}

void decommission(battery_t bat)
{
	if (!charging_data.decommissioned[bat])
	{
		charging_data.decommissioned[bat] = 1;
		charging_data.decommissioned_timestamp[bat] = get_current_timestamp_wrapped();
		charging_data.decommissioned_count[bat]++;
	}
}

void undecommission(battery_t bat)
{
	if (charging_data.decommissioned[bat])
	{
		charging_data.decommissioned[bat] = 0;
		charging_data.decommissioned_count[bat]--;
	}
}

int time_for_recomission(battery_t bat)
{
	int decommissioned_count = charging_data.decommissioned_count[bat];
	return Min(decommissioned_count * INITIAL_RECOMISSION_TIME_S, MAX_RECOMISSION_TIME_S);
}

void check_for_recomission(battery_t bat)
{
	int time_since_decomission = get_current_timestamp_wrapped() - charging_data.decommissioned_timestamp[bat];
	if (time_since_decomission > time_for_recomission(bat))
	{
		charging_data.decommissioned[bat] = 0;

		if (is_lion(bat))
		{
			// we also need to do various things to make sure that the battery won't be
			// immediately decommissioned again
			charging_data.li_entered_low_voltage_timestamp[bat] = -1;
			charging_data.li_last_full_or_recommissioned_timestamp[bat] = get_current_timestamp_wrapped();
		}
	}
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

	for (battery_t bat = 0; bat < 4; bat++)
		charging_data.already_set_sat_state[bat] = 0;

	charging_data.should_move_to_antenna_deploy = false;

	// set all old voltage values to -1
	for (battery_t bat = 0; bat < 4; bat++)
	{
		charging_data.bat_voltages[bat] = -1;
		charging_data.old_bat_voltages[bat] = -1;
	}

	// initializing all values relevant to batteries and decomissioning
	for (battery_t bat = 0; bat < 4; bat++)
	{
		charging_data.decommissioned[bat] = 0;
		charging_data.decommissioned_timestamp[bat] = -1;
		charging_data.decommissioned_count[bat] = 0;
	}

	charging_data.charging_parity = 0;
	print("initializing charging data - complete\n");
}

void set_li_to_discharge(int bat, int discharge)
{
	int discharge_pin = get_run_dischg_pin(bat);

	// NOTE: discharge pins are active low
	set_output(!discharge, discharge_pin);

	vTaskDelay(WAIT_TIME_BEFORE_PIN_CHECK_MS / portTICK_PERIOD_MS);
	int discharging_contributing_to_output = get_st_val(bat);
	if (discharging_contributing_to_output != discharge)
	{
		log_error(get_error_loc(bat), discharge ? ECODE_BAT_NOT_DISCHARGING : ECODE_BAT_NOT_NOT_DISCHARGING, true);
	}
}

void set_bat_to_charge(int bat, int charge)
{
	int chg_pin = get_run_chg_pin(bat);
	set_output(charge, chg_pin);
	print("set bat %d to charge\n", bat);

	vTaskDelay(WAIT_TIME_BEFORE_PIN_CHECK_MS / portTICK_PERIOD_MS);
	if (get_fault_pin_val_w_conversion(bat))
	{
		print("fault pin active for bat %d -- decommmissioning\n", bat);
		log_error(get_error_loc(bat), ECODE_BAT_FAULT, true);
		decommission(bat);
	}

	int new_charge_running = get_chg_pin_val_w_conversion(bat);
	if (new_charge_running != charge)
	{
		log_error(get_error_loc(bat), charge ? ECODE_BAT_NOT_CHARGING : ECODE_BAT_NOT_NOT_CHARGING, false);
	}
}

void battery_charging_task(void *pvParameters)
{
	print("starting battery charging\n");

	// delay to offset task relative to others, then start
	vTaskDelay(BATTERY_CHARGING_TASK_FREQ_OFFSET);
	TickType_t prev_wake_time = xTaskGetTickCount();

	// initialize key global variables
	init_charging_data();

	// initialize xNextWakeTime onces
	init_task_state(BATTERY_CHARGING_TASK); // suspend or run on boot (ALWAYS RUN!)

	while (true)
	{
		vTaskDelayUntil(&prev_wake_time, BATTERY_CHARGING_TASK_FREQ / portTICK_PERIOD_MS);

		// report to watchdog
		report_task_running(BATTERY_CHARGING_TASK);

		// the core battery logic -- a separate function to make it easier to
		// unit test
		#ifdef BAT_CHARGING_ACTIVE
		if (!battery_logic())
		{
			if (!battery_logic())
			{
				battery_logic();
			}
		}
		#endif
	}

	// delete this task if it ever breaks out
	vTaskDelete(NULL);
}

int battery_logic()
{
	print("entering battery logic\n");

	///
	// phase prologue: updating relevant data in the charging data struct
	///

	sat_state_t sat_state = get_sat_state_wrapped();

	#ifndef BAT_TESTING
	for (int bat = 0; bat < 4; bat++)
		charging_data.old_bat_voltages[bat] = charging_data.bat_voltages[bat];

	read_lion_volts_precise(
		(uint16_t *) &(charging_data.bat_voltages[LI1]),
		(uint16_t *) &(charging_data.bat_voltages[LI2]));

	// individual batteries within the life po banks
	uint16_t lf1_mv;
	uint16_t lf2_mv;
	uint16_t lf3_mv;
	uint16_t lf4_mv;
	read_lf_volts_precise(&(lf1_mv), &(lf2_mv), &(lf3_mv), &(lf4_mv));

	// considering the voltage of the life po banks to be the max of the cells for
	// our purposes
	charging_data.bat_voltages[LFB1] = Max(lf1_mv, lf2_mv);
	charging_data.bat_voltages[LFB2] = Max(lf3_mv, lf4_mv);

	// we're only interested in the charging battery filling up if it's a lion
	int curr_charging_filled_up = false;
	if (is_lion(charging_data.bat_charging))
	{
		curr_charging_filled_up = (!get_chg_pin_val_w_conversion(charging_data.bat_charging) &&
															 charging_data.bat_voltages[charging_data.bat_charging] > LI_FULL_SANITY_MV &&
															 (get_panel_ref_val() >= PANEL_REF_SUN_MV)) ||
															 charging_data.bat_voltages[charging_data.bat_charging] > LI_FULL_MV;

		if (curr_charging_filled_up)
				charging_data.li_last_full_or_recommissioned_timestamp[charging_data.bat_charging]
					= get_current_timestamp_wrapped();
	}
	#endif

	// reading simulated values -- some of the simulated values will already
	// have been put into the charging_data struct
	#ifdef BAT_TESTING
	int curr_charging_filled_up = simulated_curr_charging_filled_up;
	#endif

	/////
	// phase 0: determine whether any batteries should be decommissioned or
	// recommissioned
	/////

	#ifndef BAT_TESTING
	#ifndef WITHOUT_DECOMMISION
	for (int bat = 0; bat < 4; bat++)
	{
		if (charging_data.decommissioned[bat])
		{
			print("checking whether to recomission battery: %d\n", bat);
			check_for_recomission(bat);
			int recommissioned = !charging_data.decommissioned[bat];

			print("decided: %d\n", recommissioned);
			if (recommissioned)
				log_error(get_error_loc(bat), ECODE_RECOMMISSION, 0);
		}
	}

	// NOTE: only going through LI's
	for (int bat = 0; bat < 2; bat++)
	{
		// this should be taken care of, but just making sure
		if (!charging_data.decommissioned[bat])
		{
			// we have the expectation that both LI's will become full
			// the charging loop will get stuck otherwise
			if ((get_current_timestamp_wrapped() - charging_data.li_last_full_or_recommissioned_timestamp[bat])
					 > MAX_TIME_WITHOUT_FULL_MS)
			{
				print("decomissioning battery: %d because of long time without full\n", bat);
				log_error(get_error_loc(bat), ECODE_NOT_FULL_FOR_WHILE, 1);
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
					print("decomissioning battery: %d because of long time at low voltage\n", bat);
					log_error(get_error_loc(bat), ECODE_LOW_VOLTAGE_FOR_WHILE, 1);
					decommission(bat);
					continue;
				}
			}
			else
			{
				charging_data.li_entered_low_voltage_timestamp[bat] = -1;
			}

			// TODO (stretch): add something here to account for a battery that is
			// being charged without any increases in voltage
		}
	}
	#endif
	#endif

	// drawing metadata about the state of the batteries
	int num_li_down = charging_data.decommissioned[LI1] + charging_data.decommissioned[LI2];
	int num_lf_down = charging_data.decommissioned[LFB1] + charging_data.decommissioned[LFB2];

	int good_li = -1;
	if (num_li_down == 1)
		good_li = charging_data.decommissioned[LI1] ? LI2 : LI1;

	int good_lf = -1;
	if (num_lf_down == 1)
		good_lf = charging_data.decommissioned[LFB1] ? LFB2 : LFB1;

	print("currently have %d good li and %d good lf\n", num_li_down, num_lf_down);

	// we often want to know whether the higher cells within the life po banks have filled up
	bool life_po_full = (num_lf_down == 0 &&
											 charging_data.bat_voltages[LFB1] > LF_FULL_MAX_MV &&
											 charging_data.bat_voltages[LFB2] > LF_FULL_MAX_MV) ||
											(num_lf_down == 1 &&
											 charging_data.bat_voltages[good_lf] > LF_FULL_MAX_MV);

  // updating externally facing variables
	if (num_li_down == 0)
	{
		if (charging_data.bat_voltages[LI1] > LI_FULL_SANITY_MV && charging_data.bat_voltages[LI2] > LI_FULL_SANITY_MV)
			charging_data.should_move_to_antenna_deploy = 1;
	}
	else if (num_li_down == 1)
	{
		if (charging_data.bat_voltages[good_li] > LI_FULL_SANITY_MV)
			charging_data.should_move_to_antenna_deploy = 1;
	}
	else
	{
		charging_data.should_move_to_antenna_deploy = 1;
	}

	for (battery_t bat = 0; bat < 4; bat++)
	{
		if (!charging_data.already_set_sat_state[bat])
		{
			int threshold = is_lion(bat) ? LI_FULL_SANITY_MV : LF_FULL_SANITY_MV;
			if (charging_data.bat_voltages[bat] > threshold)
			{
				switch (bat)
				{
					case LI1:
						update_sat_event_history(false, true, false, false, false, false, false);
						break;

					case LI2:
						update_sat_event_history(false, false, true, false, false, false, false);
						break;

					case LFB1:
						update_sat_event_history(false, false, false, true, false, false, false);
						break;

					case LFB2:
						update_sat_event_history(false, false, false, false, true, false, false);
						break;

					default:
						configASSERT(false);
						log_error(ELOC_BAT_CHARGING, ECODE_UNEXPECTED_CASE, 1);
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

	meta_charge_state_t new_meta_charge_state = -1;
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
			if (!(sat_state == IDLE_FLASH || sat_state == IDLE_NO_FLASH) || life_po_full)
			{
				charging_data.curr_charge_state = FILL_LI_NEITHER_FULL_A;
			}
			else
			{
				switch (charging_data.curr_charge_state)
				{
					// NOTE: the battery that's currently charging has filled up if the charging
					// isn't running -- when the charge pin is inactive
					// on the first time through, curr_charging_filled_up will be false, so all good!
					case FILL_LI_NEITHER_FULL_A:
						if (charging_data.bat_charging == LI1 && curr_charging_filled_up)
							charging_data.curr_charge_state = FILL_LI_LI1_FULL_A;
						else if (charging_data.bat_charging == LI2 && curr_charging_filled_up)
							charging_data.curr_charge_state = FILL_LI_LI2_FULL_A;
						break;

					case FILL_LI_LI1_FULL_A:
						// going back takes precedence
						if (charging_data.bat_voltages[LI1] <= LI_FULL_SANITY_MV || charging_data.bat_voltages[LI2] <= LI_LOW_POWER_MV)
							charging_data.curr_charge_state = FILL_LI_NEITHER_FULL_A;
						else if (charging_data.bat_charging == LI2 && curr_charging_filled_up)
							charging_data.curr_charge_state = FILL_LF_A;
						break;

					case FILL_LI_LI2_FULL_A:
						// going back takes precedence
						if (charging_data.bat_voltages[LI2] <= LI_FULL_SANITY_MV || charging_data.bat_voltages[LI1] <= LI_LOW_POWER_MV)
							charging_data.curr_charge_state = FILL_LI_NEITHER_FULL_A;
						else if (charging_data.bat_charging == LI1 && curr_charging_filled_up)
							charging_data.curr_charge_state = FILL_LF_A;
						break;

					case FILL_LF_A:
						if (charging_data.bat_voltages[LI1] <= LI_DOWN_MV ||
								charging_data.bat_voltages[LI2] <= LI_DOWN_MV)
							charging_data.curr_charge_state = FILL_LI_NEITHER_FULL_A;
						break;

					default:
						// trying to get past -Wswitch
						configASSERT(false);
						log_error(ELOC_BAT_CHARGING, ECODE_UNEXPECTED_CASE, 1);
						break;
				}
			}
	}
	else if (charging_data.curr_meta_charge_state == ONE_LI_DOWN)
	{
		if (!(sat_state == IDLE_FLASH || sat_state == IDLE_NO_FLASH) || life_po_full)
		{
			charging_data.curr_charge_state = FILL_LI_B;
		}
		else
		{
			switch (charging_data.curr_charge_state)
			{
				case FILL_LI_B:
					if (charging_data.bat_charging == good_li && curr_charging_filled_up)
						charging_data.curr_charge_state = FILL_LF_B;
					break;

				case FILL_LF_B:
					if (charging_data.bat_voltages[good_li] <= LI_DOWN_MV)
						charging_data.curr_charge_state = FILL_LI_B;
					break;

				default:
					// trying to get past -Wswitch
					configASSERT(false);
					log_error(ELOC_BAT_CHARGING, ECODE_UNEXPECTED_CASE, 1);
					break;
			}
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
	int old_bat_charging = charging_data.bat_charging;
	int old_li_discharging = charging_data.lion_discharging;

	if (charging_data.curr_meta_charge_state == ALL_GOOD)
	{
		switch (charging_data.curr_charge_state)
		{
			case FILL_LI_NEITHER_FULL_A:
				// charge the lion with the the lower voltage
				// discharge the lion with the higher voltage
				if (charging_data.bat_voltages[LI1] <= charging_data.bat_voltages[LI2])
				{
					charging_data.bat_charging = LI1;
					charging_data.lion_discharging = LI2;
				}
				else
				{
					charging_data.bat_charging = LI2;
					charging_data.lion_discharging = LI1;
				}

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
					if (charging_data.bat_voltages[LFB1] <= charging_data.bat_voltages[LFB2])
						charging_data.bat_charging = LFB1;
					else
						charging_data.bat_charging = LFB2;
				}

				// discharge the lion with the higher voltage
				if (charging_data.bat_voltages[LI1] <= charging_data.bat_voltages[LI2])
					charging_data.lion_discharging = LI2;
				else
					charging_data.lion_discharging = LI1;

				break;

			default:
				// trying to get past -Wswitch
				configASSERT(false);
				log_error(ELOC_BAT_CHARGING, ECODE_UNEXPECTED_CASE, 1);
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
					if (charging_data.bat_voltages[LFB1] <= charging_data.bat_voltages[LFB2])
						charging_data.bat_charging = LFB1;
					else
						charging_data.bat_charging = LFB2;
				}

				charging_data.lion_discharging = good_li;
				break;

			default:
				// trying to get past -Wswitch
				configASSERT(false);
				log_error(ELOC_BAT_CHARGING, ECODE_UNEXPECTED_CASE, 1);
				break;
		}
	}
	else if (charging_data.curr_meta_charge_state == TWO_LF_DOWN)
	{
		// NOTE: only one sub-state here!
		if (num_li_down == 0)
		{
			if (charging_data.bat_voltages[LI1] <= charging_data.bat_voltages[LI2])
			{
				charging_data.bat_charging = LI1;
				charging_data.lion_discharging = LI2;
			}
			else
			{
				charging_data.bat_charging = LI2;
				charging_data.lion_discharging = LI1;
			}
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
		battery_t lion_not_discharging = (charging_data.lion_discharging == LI1) ? LI2 : LI1;

		// NOTE: very important to set the discharging pin to active before setting
		// the other to inactive
		set_li_to_discharge(charging_data.lion_discharging, 1);

		// TODO: suspend the scheduler
		// we're going to decommission here and undecommission if the satellite doesn't
		// reboot
		log_error(get_error_loc(charging_data.lion_discharging), ECODE_BAT_NOT_DISCHARGING_RESTART, 1);
		decommission(charging_data.lion_discharging);

		// TODO: force a write to the MRAM

		set_li_to_discharge(lion_not_discharging, 0);
		undecommission(charging_data.lion_discharging);

		// TODO: resume the scheduler
	}

	///
	// phase 3b: set the battery that should be charging to charge and
	//  set the others to not charge
	///

	if (old_bat_charging == -1)
	{
		for (int i = 0; i < 4; i++)
		{
			set_bat_to_charge(i, i == charging_data.bat_charging);
		}
	}
	else if (charging_data.bat_charging != old_bat_charging)
	{
		set_bat_to_charge(charging_data.bat_charging, 1);

		// TODO: did we want to add a strike condition here for a bad CHGN value?
		// (should ask Tyler)

		set_bat_to_charge(old_bat_charging, 0);
	}

	if (got_mutex) xSemaphoreGive(critical_action_mutex);
	#endif

	print("leaving battery charging\n");
	return true;
}
