/*
 * battery_charging_task.c
 *
 * Created: 9/21/2017 20:37:56
 *  Author: mcken
 */

#include "battery_charging_task.h"

// high-level TODO's:
//  0. understand the code as it stands (done)
//  1. be working in terms of raw voltages (done)
//  2. make sure of state changes (done)
//  3. work through TODO's, including new idea of fullness (done)
//  4. implement strikes! (also, what will change once a battery has struck out?)
//  5. write a simulator to test the correctness of the battery code
//  6. iron out threshold values (later)
//  7. put algorithm through longevity tests (later)

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

	// TODO: make sure the CHG pin will be active before reaching trickle charge
	// NOTE: CHGN is active low, so we're making a conversion here
	return !((batch>>chg_position)&0x1);
}

int get_st_val(battery_t bat)
{
	int st_position = (bat == LI1) ? 9 : 8;
	bat_charge_dig_sigs_batch batch;
	read_bat_charge_dig_sigs_batch(&batch);
	return (batch>>st_position)&0x01;
}

void battery_charging_task(void *pvParameters)
{
	// delay to offset task relative to others, then start
	vTaskDelay(BATTERY_CHARGING_TASK_FREQ_OFFSET);
	TickType_t prev_wake_time = xTaskGetTickCount();
	
	/////
	/// initialize key global variables
	/////

	// we want to initially be in FILL_LI_NEITHER_FULL
	curr_charge_state = FILL_LI_NEITHER_FULL;

	// explicitly initializing these even though initial value doesn't matter
	// NOTE: nothing should rely on these variables' initial state without taking
	// -1 into account
	batt_charging = -1;
	lion_discharging = -1;

	already_set_sat_state = false;
	
	init_task_state(BATTERY_CHARGING_TASK); // suspend or run on boot (ALWAYS RUN!)

	while (true)
	{
		vTaskDelayUntil(&prev_wake_time, BATTERY_CHARGING_TASK_FREQ / portTICK_PERIOD_MS);

		// report to watchdog
		report_task_running(BATTERY_CHARGING_TASK);

		#ifdef BAT_CHARGING_ACTIVE

			///
 			// before getting into it, grab the percentages of each of the batteries
 			// lions and life po's
			///

			uint16_t li1_mv;
			uint16_t li2_mv;
 			read_li_volts_precise(&li1_mv, &li2_mv);

 			// individual batteries within the life po banks
 			uint16_t lf1_mv;
 			uint16_t lf2_mv;
 			uint16_t lf3_mv;
 			uint16_t lf4_mv;
			read_lf_volts_precise(&lf1_mv, &lf2_mv, &lf3_mv, &lf4_mv);

 			// battery_logic is an individual function in order to make it easier to
			// "unit test" it with contrived inputs

			///
 			// what batteries should we be charging?
			///

			int curr_charging_filled_up = !get_chg_pin_val_w_conversion(batt_charging);
			battery_logic(li1_mv, li2_mv, lf1_mv, lf2_mv, lf3_mv, lf4_mv, curr_charging_filled_up);
			
		#endif
	}

	// delete this task if it ever breaks out
	vTaskDelete(NULL);
}

void battery_logic(
	int li1_mv,
	int li2_mv,
	int lf1_mv,
	int lf2_mv,
	int lf3_mv,
	int lf4_mv,
	int curr_charging_filled_up)
{
	// we often want to know the higher of the cells within the life po banks
	int max_lfb1_mv = Max(lf1_mv, lf2_mv);
	int max_lfb2_mv = Max(lf3_mv, lf4_mv);

	// we often want to know whether the higher cells within the life po banks have filled up
	bool life_po_full_mv = max_lfb1_mv > LF_FULL_MAX_MV && max_lfb2_mv > LF_FULL_MAX_MV;

	/////
	// phase 0: determine whether any batteries should have their strikes incremented
	/////

	// TODO: Implement this. Some of the remaining q's:
	//   - How does striking out work for life po? It might be hard to avoid false positives
	//     (wrongly adding a strike) with a diff. in the charge of the cells.
	//   - What do we do if we strike out?

	/////
	// phase 1: determine whether we want to make a change to the charge state
	/////

	// the flow is as follows:
	// the default is to stay in a holding pattern, always charging the lion with
	//   the lower voltage
	// if we decide to eventually charge life po's, we get a little more aggresive,
	//   trying to systematically fill up both lion's
	// finally, when we do fill up the lion's we move to charging life po's --
	//   until one of the lion's drops below a low threshold

	// this is not equivalent with with the global state, but they're related in the following ways:
	//  - if the global state is idle flash or idle not flash, we in any state
	//  - otherwise, we can only be in FILL_LI_NEITHER_FULL

	// NOTE: initial charge state is FILL_LI

	sat_state_t sat_state = get_sat_state();

	// TODO: is this what we want in terms of the full life po condition
	if (!(sat_state == IDLE_FLASH || sat_state == IDLE_NO_FLASH) || life_po_full_mv)
	{
		curr_charge_state = FILL_LI_NEITHER_FULL;
	}
	else
	{
			switch (curr_charge_state)
			{
				// NOTE: the battery that's currently charging has filled up if the charging
				// isn't running -- when the charge pin is inactive
				// on the first time through, batt_charging should be -1, and this branch
				// won't be entered
				case FILL_LI_NEITHER_FULL:
					if (batt_charging == LI1 && curr_charging_filled_up)
						curr_charge_state = FILL_LI_LI1_FULL;
					else if (batt_charging == LI2 && curr_charging_filled_up)
						curr_charge_state = FILL_LI_LI2_FULL;
					break;

				case FILL_LI_LI1_FULL:
					// going back takes precedence
					if (li1_mv <= LI_FULL_SANITY_MV || li2_mv <= LI_FULL_SANITY_MV)
						curr_charge_state = FILL_LI_NEITHER_FULL;
					else if (batt_charging == LI2 && curr_charging_filled_up)
						curr_charge_state = FILL_LF;
					break;

				case FILL_LI_LI2_FULL:
					// going back takes precedence
					if (li1_mv <= LI_FULL_SANITY_MV || li2_mv <= LI_FULL_SANITY_MV)
						curr_charge_state = FILL_LI_NEITHER_FULL;
					else if (batt_charging == LI1 && curr_charging_filled_up)
						curr_charge_state = FILL_LF;
					break;

				case FILL_LF:
					if (li1_mv <= LI_DOWN_MV || li2_mv <= LI_DOWN_MV)
						curr_charge_state = FILL_LI_NEITHER_FULL;
					break;
			}
	}

	if (!already_set_sat_state && curr_charge_state == FILL_LF)
	{
		update_sat_event_history(false, true, true, false, false, false);
		already_set_sat_state = true;
	}

	/////
	// phase 2: determine which batteries should be charging
	/////

	// this is generally dictated by wanting the two lion's or the two lifepo banks
	// to be balanced as they charged and discharged

	// therefore, in accordance with the charging state, the batteries with lower
	// voltages will be charged, and those with higher voltages will be discharged

	switch (curr_charge_state)
	{
		case FILL_LI_NEITHER_FULL:
			// charge the lion with the the lower voltage
			// discharge the lion with the higher voltage
			if (li1_mv <= li2_mv)
			{
				batt_charging = LI1;
				lion_discharging = LI2;
			}
			else
			{
				batt_charging = LI2;
				lion_discharging = LI1;
			}

			break;

		case FILL_LI_LI1_FULL:
			batt_charging = LI2;
			lion_discharging = LI1;
			break;

		case FILL_LI_LI2_FULL:
			batt_charging = LI1;
			lion_discharging = LI2;
			break;

		case FILL_LF:
			// charge the life po bank with the lower max percentage
			// NOTE: this might lead to issues if the cells are very
			// unbalanced
			if (max_lfb1_mv <= max_lfb2_mv)
				batt_charging = LFB1;
			else
				batt_charging = LFB2;

			// discharge the lion with the higher voltage
			if (li1_mv <= li2_mv)
				lion_discharging = LI2;
			else
				lion_discharging = LI1;

			break;
	}

	// in all cases, we also need to make changes with respect to lion that isn't
	// discharging
	battery_t lion_not_discharging = (lion_discharging == LI1) ? LI2 : LI1;

	/////
	// phase 3: apply the decisions we've made about which batteries to charge!
	/////

	xSemaphoreTake(_battery_charging_mutex, (TickType_t) BAT_MUTEX_WAIT_TIME_TICKS);

	///
	// phase 3a: set the lion that should be discharging to discharge and
	//  set the other lion to not discharge
	///

	// TODO: make sure all of the logic with the checking works
	// NOTE: very important to set the discharging pin to true before setting the other to false
	int discharge_pin = get_run_dischg_pin(lion_discharging);
	int discharge_success = 0;
	for (int i = 0; !discharge_success && i < MAX_TIMES_TRY_PIN; i++)
	{
		// NOTE: discharge pins are active low
		set_output(false, discharge_pin);
		vTaskDelay(WAIT_TIME_BEFORE_PIN_CHECK_MS / portTICK_PERIOD_MS);

		int contributing_to_output = get_st_val(lion_discharging);
		discharge_success = contributing_to_output;
	}

	if (!discharge_success)
	{
		// TODO: add a strike and take some action
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

	if (!stop_discharge_success)
	{
		// TODO: add a strike and take some action
	}

	///
	// phase 3b: set the battery that should be charging to charge and
	//  set the others to not charge
	///

	// looping through each of the batteries
	for (battery_t bat_type = 0; bat_type < 4; bat_type++)
	{
		int charge_pin = get_run_chg_pin(bat_type);
		int should_be_charging = (batt_charging == bat_type); // see the enum in .h

		int charge_success = 0;
		for (int j = 0; !charge_success && j < MAX_TIMES_TRY_PIN; j++)
		{
			set_output(should_be_charging, charge_pin);
			vTaskDelay(WAIT_TIME_BEFORE_PIN_CHECK_MS / portTICK_PERIOD_MS);

			int charge_running = get_chg_pin_val_w_conversion(bat_type);
			charge_success = (charge_running == should_be_charging);
		}

		if (!charge_success)
		{
			// TODO: add a strike and take some action
		}
	}

	xSemaphoreGive(_battery_charging_mutex);
}
