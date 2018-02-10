/*
 * battery_charging_task.c
 *
 * Created: 9/21/2017 20:37:56
 *  Author: mcken
 */

#include "battery_charging_task.h"

// high-level TODO's:
//  0. read through and do eye test
//  1. unit tests
//  2. how do charging states interface with global states
//  3. add last couple strikes features
//  4. write simulator to sequentially run unit tests (later)
//  5. iron out threshold values (later)

// TODO: how do we interface with global state?
// TODO: read through TODO's and make sure of everything!
// TODO: do we need two tiers of error?
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
	read_ad7991_batbrd(four_buf, four_buf+2);
	return ((int) four_buf[2]<<8);
}

int is_lion(battery_t bat)
{
	return (bat == LI1 || bat == LI2);
}

// NOTE: it's important that batteries don't get doubly decomissioned in a single
// loop of the battery logic
void decommission(battery_t bat)
{
	charging_data.decommissioned[bat] = 1;
	charging_data.decommissioned_timestamp[bat] = get_current_timestamp_wrapped();
	charging_data.decommissioned_count[bat]++;
}

int time_for_recomission(battery_t bat)
{
	int scale_factor = 1;
	int decomissioned_count = charging_data.decommissioned_count[bat];
	for (int i = 0; i < decomissioned_count; i++)
		scale_factor *= RECOMISSION_TIME_INCREASE;

	return Min(scale_factor * INITIAL_RECOMISSION_TIME_S, MAX_RECOMISSION_TIME_S);
}

void check_for_recomission(battery_t bat)
{
	int time_since_decomission = get_current_timestamp_wrapped() - charging_data.decommissioned_timestamp[bat];
	if (time_since_decomission > time_for_recomission(bat))
		charging_data.decommissioned[bat] = 0;
}

void init_charging_data()
{
	// explicitly initializing these even though initial value doesn't matter
	// NOTE: nothing should rely on these variables' initial state without taking
	// -1 into account
	// TODO: check for edge cases where something is dependent on the initial value
	// of these state variables

	print("initializing charging data -- start\n");

	charging_data.bat_charging = -1;
	charging_data.lion_discharging = -1;

	// we want to initially be in ALL_GOOD
	charging_data.curr_meta_charge_state = ALL_GOOD;

	// we want to initially be in FILL_LI_NEITHER_FULL_A
	charging_data.curr_charge_state = FILL_LI_NEITHER_FULL_A;

	// set all battery timestamps to -1
	charging_data.li_full_timestamp[0] = -1;
	charging_data.li_full_timestamp[1] = -1;

	charging_data.li_low_voltage_timestamp[0] = -1;
	charging_data.li_low_voltage_timestamp[1] = -1;

	charging_data.already_set_sat_state = false;

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
	print("initializing charging data - complete");
}

void battery_charging_task(void *pvParameters)
{
	print("starting battery charging");

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
		battery_logic();
		#endif
	}

	// delete this task if it ever breaks out
	vTaskDelete(NULL);
}

void battery_logic()
{
	print("entering battery logic");

	///
	// phase prologue: updating relevant data in the charging data struct
	// NOTE: this should be skipped when running unit tests
	///

	sat_state_t sat_state = get_sat_state_wrapped();

	#ifndef BAT_TESTING
	read_li_volts_precise(
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

	// *TODO: how to simulate this?
	// conditional so everything goes smoothly on the first time through
	int curr_charging_filled_up = false;
	if (charging_data.bat_charging != -1)
	{
		// TODO: add an extra piece so it calls itself full if it's above a certain
		// threshold?
		curr_charging_filled_up = !get_chg_pin_val_w_conversion(charging_data.bat_charging) &&
															charging_data.bat_charging > LI_FULL_SANITY_MV &&
															(get_panel_ref_val() <= 8000); // TODO: how's this conditional?

		if (curr_charging_filled_up)
		{
			if (is_lion(charging_data.bat_charging))
				charging_data.li_full_timestamp[charging_data.bat_charging] = get_current_timestamp_wrapped();
		}
	}
	#endif

	#ifdef BAT_TESTING
	int curr_charging_filled_up = simulated_curr_charging_filled_up;
	#endif

	/////
	// phase 0: determine whether any batteries should be decomissioned
	/////

	#ifndef BAT_TESTING
	for (battery_t bat = 0; bat < 4; bat++)
	{
		if (charging_data.decommissioned[bat])
		{
			print("checking whether to recomission battery: %d", bat);
			check_for_recomission(bat);
			print("decided: %d", bat);
		}
	}

	// TODO: where else do we want to use PANEL_REF
	for (battery_t bat = 0; bat < 4; bat++)
	{
		// making sure that no one is decommissioned twice
		if (!charging_data.decommissioned[bat])
		{
			if (get_fault_pin_val_w_conversion(bat))
			{
				print("decomissioning battery: %d because of fault", bat);
				decommission(bat);
				continue;
			}

			if (is_lion(bat))
			{
				if ((charging_data.li_full_timestamp[bat] == -1 &&
					   get_current_timestamp_wrapped() > MAX_TIME_WITHOUT_FULL_MS) ||
						(charging_data.li_full_timestamp[bat] != -1 &&
						 (get_current_timestamp_wrapped() - charging_data.li_full_timestamp[bat])
						  > MAX_TIME_WITHOUT_FULL_MS))
					{
						print("decomissioning battery: %d because of long time without full", bat);
						decommission(bat);
						continue;
					}
			}

			if (is_lion(bat))
			{
				if (charging_data.bat_voltages[bat] <= LI_CRITICAL_MV)
				{
					if (charging_data.li_low_voltage_timestamp[bat] == -1)
					{
						charging_data.li_low_voltage_timestamp[bat] = get_current_timestamp_wrapped();
					}
					else if (get_current_timestamp_wrapped() - charging_data.li_low_voltage_timestamp[bat] >
									 MAX_TIME_BELOW_V_THRESHOLD_S)
					{
						// we should reset the low voltage timestamp, so this battery will get a chance
						// next time
						print("decomissioning battery: %d because of long time at low voltage", bat);
						charging_data.li_low_voltage_timestamp[bat] = -1;
						decommission(bat);
						continue;
					}
				}
				else
				{
					charging_data.li_low_voltage_timestamp[bat] = -1;
				}
			}

			// TODO: should we check a couple times here?
			if (charging_data.old_bat_voltages[bat] != -1)
			{
				int voltage_drop_mv = charging_data.bat_voltages[bat] - charging_data.old_bat_voltages[bat];
				int threshold = (bat == charging_data.bat_charging) ? MAX_VOLTAGE_DROP_W_CHARGE_MV : MAX_VOLTAGE_DROP_MV;

				if (voltage_drop_mv > threshold)
				{
					print("decomissioning battery: %d because of sudden voltage drop", bat);
					decommission(bat);
					continue; // just for consistency
				}
			}
		}
	}
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

	print("currently have %d good li and %d good lf", good_li, good_lf);

	// we often want to know whether the higher cells within the life po banks have filled up
	bool life_po_full = (num_lf_down == 0 &&
											 charging_data.bat_voltages[LFB1] > LF_FULL_MAX_MV &&
											 charging_data.bat_voltages[LFB2] > LF_FULL_MAX_MV) ||
											(num_lf_down == 1 &&
											 charging_data.bat_voltages[good_lf] > LF_FULL_MAX_MV);

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

	// this is not equivalent with with the global state, but they're related in the following ways:
	//  - if the global state is idle flash or idle not flash, we in any state
	//  - otherwise, we can only be in FILL_LI_NEITHER_FULL_A

	// if someone has recently been decomissioned it will most likely result in a
	// change in the meta-state -- we should take a look

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

	print("decided on meta charge state: %d", new_meta_charge_state);

	if (new_meta_charge_state != charging_data.curr_meta_charge_state)
	{
		print("this is a new meta charge state");
		charging_data.curr_meta_charge_state = new_meta_charge_state;
		switch (charging_data.curr_meta_charge_state)
		{
			case ALL_GOOD:
				charging_data.curr_charge_state = FILL_LI_NEITHER_FULL_A;
				break;

			// TODO: global state makes no sense here
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
						// TODO: should we check for either being below LI_FULL_SANITY_MV?
						if (charging_data.bat_voltages[LI1] <= LI_FULL_SANITY_MV)
							charging_data.curr_charge_state = FILL_LI_NEITHER_FULL_A;
						else if (charging_data.bat_charging == LI2 && curr_charging_filled_up)
							charging_data.curr_charge_state = FILL_LF_A;
						break;

					case FILL_LI_LI2_FULL_A:
						// going back takes precedence
						if (charging_data.bat_voltages[LI2] <= LI_FULL_SANITY_MV)
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
					log_error(ELOC_BAT_CHARGING, ECODE_UNEXPECTED_CASE, 1);
					break;
			}
		}
	}

	// NOTE: the other two meta states only have one charge state within them!!

	// TODO: do we want a more complex system here?
	if (!charging_data.already_set_sat_state &&
			(charging_data.curr_charge_state == FILL_LF_A ||
			 charging_data.curr_charge_state == FILL_LF_B ||
		 	 get_current_timestamp_wrapped() > MAX_TIME_TO_WAIT_FOR_DEPLOY_S))
	{
		update_sat_event_history(false, true, true, false, false, false);
		charging_data.already_set_sat_state = true;
	}

	print("decided on charge state: %d", charging_data.curr_charge_state);

	/////
	// phase 2: determine which batteries should be charging
	/////

	// this is generally dictated by wanting the two lion's or the two lifepo banks
	// to be balanced as they charged and discharged

	// therefore, in accordance with the charging state, the batteries with lower
	// voltages will be charged, and those with higher voltages will be discharged

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
			charging_data.lion_discharging = LI2;
		}
		else
		{
			charging_data.bat_charging = LI2;
			charging_data.lion_discharging = LI1;
		}

		charging_data.charging_parity = !charging_data.charging_parity;
	}

	print("decided on battery charging: %d", charging_data.bat_charging);
	print("decided on li discharging: %d", charging_data.lion_discharging);

	// in all cases, we also need to make changes with respect to lion that isn't
	// discharging
	battery_t lion_not_discharging = (charging_data.lion_discharging == LI1) ? LI2 : LI1;

	/////
	// phase 3: apply the decisions we've made about which batteries to charge!
	/////

	#ifndef BAT_TESTING
	if (!xSemaphoreTake(battery_charging_mutex, (TickType_t) BAT_MUTEX_WAIT_TIME_TICKS)) {
		// if for some reason we can't get the bat charging mutex (it times out),
		// ignore it and move on (the only things this mutex prevents is flashing while
		// lifepos are charging, which is less worrisome than not running charging logic)
		log_error(ELOC_BAT_CHARGING, ECODE_BAT_CHARGING_MUTEX_TIMEOUT, true);
	}

	///
	// phase 3a: set the lion that should be discharging to discharge and
	//  set the other lion to not discharge
	///

	// TODO: check with special attention here
	// NOTE: very important to set the discharging pin to true before setting the other to false
	int discharge_pin = get_run_dischg_pin(charging_data.lion_discharging);
	int discharge_success = 0;
	for (int i = 0; !discharge_success && i < MAX_TIMES_TRY_PIN; i++)
	{
		// NOTE: discharge pins are active low
		set_output(false, discharge_pin);
		vTaskDelay(WAIT_TIME_BEFORE_PIN_CHECK_MS / portTICK_PERIOD_MS);

		int contributing_to_output = get_st_val(charging_data.lion_discharging);
		discharge_success = contributing_to_output;
	}

	print("set bat to discharge: %d", charging_data.lion_discharging);

	// TODO: really difficult edge case -- it might be the case that we're working
	// with a battery that's already been decommissioned
	if (!discharge_success)
	{
		print("discharging failed, decomissioning bat: %d", charging_data.lion_discharging);
		decommission(charging_data.lion_discharging);

		// TODO: check with special attention here
		for (battery_t bat = 0; bat < 4; bat++)
			charging_data.old_bat_voltages[bat] = charging_data.bat_voltages[bat];

		// we're going to just restart the battery charging logic
		xSemaphoreGive(battery_charging_mutex);
		print("restarting battery logic");
		battery_logic();
	}

	// set the lion that should not be discharging to not discharge
	int not_discharge_pin = get_run_dischg_pin(lion_not_discharging);
	int stop_discharge_success = 0;
	for (int i = 0; !stop_discharge_success && i < MAX_TIMES_TRY_PIN; i++)
	{
		// NOTE: discharge pins are active low
		set_output(true, not_discharge_pin);
		vTaskDelay(WAIT_TIME_BEFORE_PIN_CHECK_MS / portTICK_PERIOD_MS);

		int contributing_to_output = get_st_val(lion_not_discharging);
		stop_discharge_success = !contributing_to_output;
	}

	print("set bat to not discharge: %d", lion_not_discharging);

	// variable prevents a possible double count of whether or not a battery has
	// been decomissioned
	int already_decomissioned = -1;
	if (!stop_discharge_success)
	{
		// TODO: do we need immediate action here?
		// TODO: do we want to try more here?
		print("setting to not discharge failed, decomissioning bat: %d", lion_not_discharging);
		decommission(lion_not_discharging);
		already_decomissioned = lion_not_discharging;
	}

	///
	// phase 3b: set the battery that should be charging to charge and
	//  set the others to not charge
	///

	// looping through each of the batteries
	for (battery_t bat_type = 0; bat_type < 4; bat_type++)
	{
		int charge_pin = get_run_chg_pin(bat_type);
		int should_be_charging = (charging_data.bat_charging == bat_type); // see the enum in .h

		int charge_success = 0;

		// TODO: should we only go through this if it's a change from their current
		// status
		for (int j = 0; !charge_success && j < MAX_TIMES_TRY_PIN; j++)
		{
			set_output(should_be_charging, charge_pin);
			vTaskDelay(WAIT_TIME_BEFORE_PIN_CHECK_MS / portTICK_PERIOD_MS);

			// TODO: I don't really agree with the error check here
			int charge_running = get_chg_pin_val_w_conversion(bat_type);

			// TODO: check this boolean here
			charge_success = (charge_running == should_be_charging) ||
											 (get_panel_ref_val() <= 8000) || // we aren't in the sun
											 (charging_data.bat_voltages[charging_data.bat_charging] >= MIGHT_BE_FULL); // could be full
		}

		print("set bat: %d to charge status: %d", bat_type, should_be_charging);

		if (!charge_success)
		{
			print("changing charge pin failed, decomissioning bat %d if it hasn't been decomissioned already", bat_type);

			// TODO: do we need immediate action here?
			if (charging_data.bat_charging != already_decomissioned)
				decommission(charging_data.bat_charging);
		}
	}

	///
	// phase epilogue: getting everything ready for next time through
	///

	for (battery_t bat = 0; bat < 4; bat++)
		charging_data.old_bat_voltages[bat] = charging_data.bat_voltages[bat];

	xSemaphoreGive(battery_charging_mutex);
	#endif

	print("leaving battery charging");
}
