/*
 * battery_charging_task.c
 *
 * Created: 9/21/2017 20:37:56
 *  Author: mcken
 */

#include "battery_charging_task.h"

// lion_down_mv-level TODO's:
//  0. understand the code as it stands (done)
//  1. be working in terms of raw voltages (done)
//  2. make sure of state changes
//  3. work through TODO's
//  4. implement strikes! (also, what will change once a battery has struck out?)
//  5. write a simulator to test the correctness of the battery code
//  6. iron out threshold values
//  7. put algorithm through longevity tests

// number of strikes for each battery
extern int batt_strikes[4] = {0, 0, 0, 0};

void battery_charging_task(void *pvParameters)
{
	/////
	/// initialize key global variables
	/////

	// explicitly initializing this even though initial value doesn't matter
	curr_charge_state = FILL_LI;

	// explicitly initializing this even though initial value doesn't matter
	// TODO: make sure it doesn't matter
	batt_charging = -1;
	batt_discharging = -1;

	// initialize xNextWakeTime onces
	TickType_t prev_wake_time = xTaskGetTickCount();
	init_task_state(BATTERY_CHARGING_TASK); // suspend or run on boot (ALWAYS RUN!)

	while (true)
	{
		vTaskDelayUntil(&prev_wake_time, BATTERY_CHARGING_TASK_FREQ / portTICK_PERIOD_MS);

    // report to watchdog
		report_task_running(BATTERY_CHARGING_TASK);

 		// before getting into it, grab the percentages of each of the batteries
 		// lions

		int li1_mv;
		int li2_mv;
 		get_current_lion_volts(&li1_mv, &li2_mv, NULL);

 		// individual batteries within the life po banks
 		int lf1_mv;
 		int lf2_mv;
 		int lf3_mv;
 		int lf4_mv;
		get_current_lifepo_volts(
			&lf1_mv,
			&lf2_mv,
			&lf3_mv,
			&lf4_mv,
			NULL);

 		// NOTE: battery_logic is an individual function in order to make
 		// it easier to "unit test" them with contrived inputs

 		// what batteries should we be charging?
 		battery_logic(
 			li1_mv,
 			li2_mv,
 			lf1_mv,
 			lf2_mv,
 			lf3_mv,
			lf4_mv);
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
	int lf4_mv)
{
	// we often want to know the lower of the cells within the life po banks
	int max_lfb1_mv = Max(lf1_mv, lf2_mv);
	int max_lfb2_mv = Max(lf3_mv, lf4_mv);

	// we often want to know whether the lower cells within the life po banks have filled up
	bool life_po_full_mv = max_lfb1_mv > lf_full_max_mv && max_lfb2_mv > lf_full_max_mv;

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

	// the flow is as follows: we should charge the lion's up to lion_up_mv, and in the time between
	// their filling up and their dropping down below an optimal state, we should charge the
	// lifepo

	// more concretely:
	//  - if we're in FILL_LION, we switch if the lifepo's aren't lion_up_mv and both lion's are
	//  - if we're in FILL_LIFE_PO, we switch if the either lion is below lion_low_power_mv, or both are
	//    below lion_down_mv

	// this isn't synonymous with the global state, but they're related in the following ways:
	//  - if the global state is idle flash or idle not flash, we can be in either FILL_LION,
	//    or FILL_LIFE_PO
	//  - otherwise, FILL_LION is the only option

	// TODO: incorporate charge pin in some way
	// TODO: satellite state should be stored in a gloal somewhere instead of being searched
	// for in this way
	if (get_sat_state() != IDLE_FLASH && get_sat_state() != IDLE_NO_FLASH)
	{
		// TODO: make extra sure of this
		curr_charge_state = FILL_LI;
	}
	else
	{
		switch (curr_charge_state)
		{
			// NOTE: these conditions are a subset of the low power conditions -- if it's low
			// power, it'll be FILL_LION

			case FILL_LI:
				if (!life_po_full_mv && (li1_mv > li_up_mv && li2_mv > li_up_mv))
				{
					curr_charge_state = FILL_LF;
				}

				break;

			case FILL_LF:
				if ((li1_mv < li_down_mv && li2_mv < li_down_mv) ||
					(li1_mv < li_low_power_mv || li2_mv < li_low_power_mv) ||
					life_po_full_mv)
				{
					curr_charge_state = FILL_LI;
				}

				break;
		}
	}

	/////
	// phase 2: determine which batteries should be charging
	/////

	switch (curr_charge_state)
	{
		case FILL_LI:
			// charge the lion with the the lower percentage
			// discharge the lion with the lion_down_mver percentages
			if (li1_mv <= li2_mv)
			{
				batt_charging = LI1;
				batt_discharging = LI2;
			}
			else
			{
				batt_charging = LI2;
				batt_discharging = LI1;
			}

			break;

		case FILL_LF:
			// charge the life po bank with the lower max percentage
			// NOTE: this might lead to issues if the cells are very
			// unbalanced
			if (max_lfb1_mv <= max_lfb2_mv)
			{
				batt_charging = LFB1;
			}
			else
			{
				batt_charging = LFB2;
			}

			// discharge the lion with the the lion_down_mver percentage
			if (li1_mv <= li2_mv)
			{
				batt_discharging = LI2;
			}
			else
			{
				batt_discharging = LI1;
			}

			break;
	}

	/////
	// phase 3: apply the decisions we've made about which batteries to charge!
	/////

	// TODO: reconsider, down the road, the effects of the suspend and resume all
	// TODO: make very sure this doesn't hang
	#ifndef BATTERY_DEBUG
	vTaskSuspendAll();

	// set the lion that should be discharging to discharge
	// set the other lion to not discharge
	// NOTE: very important to set the discharging pin to true before setting the other to false
	int discharge_pin = 0;
	int not_discharge_pin = 0;
	if (batt_discharging == LI1)
	{
		discharge_pin = P_L1_DISG;
		not_discharge_pin = P_L2_DISG;
	}
	else
	{
		discharge_pin = P_L2_DISG;
		not_discharge_pin = P_L1_DISG;
	}

	// TODO: make sure this went through by looking at the CHG PIN
	// TODO: this should be done with more error checks
	set_output(true, discharge_pin);
	set_output(false, not_discharge_pin);

	// set the battery that should be charging to charge
	// set the others to not charge
	for (int i = 0; i < 4; i++)
	{
		int charge_pin = 0;
		switch (i)
		{
			case LI1:
				charge_pin = P_L1_RUN_CHG;
				break;

			case LI2:
				charge_pin = P_L2_RUN_CHG;
				break;

			case LFB1:
				charge_pin = P_LF_B1_RUNCHG;
				break;

			case LFB2:
				charge_pin = P_LF_B2_RUNCHG;
				break;
		}

		// TODO: make sure this went through by looking at CHG
		set_output(i == batt_charging, charge_pin);
	}

	xTaskResumeAll();
	#endif
}
