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
//  6. iron out threshold values (later)
//  7. put algorithm through longevity tests (later)

// number of strikes for each battery
extern int batt_strikes[4] = {0, 0, 0, 0};

// TODO*: make everything definitely consistent with new conception of state
// TODO*: make sure the flash task deals adequately with a potentially charging lifepo
// TODO*: make sure that a global boolean flag is being set on the first time that
// Lion's become full
void battery_charging_task(void *pvParameters)
{
	/////
	/// initialize key global variables
	/////

	// we want to initially be in FILL_LI
	curr_charge_state = FILL_LI;

	// explicitly initializing these even though initial value doesn't matter
	// NOTE: nothing should rely on these variables' initial state without taking
	// -1 into account
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

		///
 		// before getting into it, grab the percentages of each of the batteries
 		// lions and life po's
		///

		int li1_mv;
		int li2_mv;
 		read_li_volts_precise(&li1_mv, &li2_mv);

 		// individual batteries within the life po banks
 		int lf1_mv;
 		int lf2_mv;
 		int lf3_mv;
 		int lf4_mv;
		read_lf_volts_precise(&lf1_mv, &lf2_mv, &lf3_mv, &lf4_mv);

 		// battery_logic is an individual function in order to make it easier to
		// "unit test" them with contrived inputs

		///
 		// what batteries should we be charging?
		///

		battery_logic(li1_mv, li2_mv, lf1_mv, lf2_mv, lf3_mv, lf4_mv);
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

	// the flow is as follows: we should charge the lion's up to LI_UP_MV, and in the time between
	// their filling up and their dropping down below LI_DOWN_MV, we should charge the
	// lifepo

	// more concretely:
	//  - if we're in FILL_LI, we switch when the lion's becomes full (above LI_UP_MV)
	//  - if we're in FILL_LF, we switch the when the max cell in both lifepo banks
	//    becomes full (above LF_FULL_MAX_MV) or one of the lion's drops below LI_DOWN_MV

	// this is not equivalent with with the global state, but they're related in the following ways:
	//  - if the global state is idle flash or idle not flash, we can be in either FILL_LI,
	//    or FILL_LF
	//  - otherwise, we can only be in FILL_LI

	// NOTE: initial charge state is FILL_LI
	// TODO*: incorporate charge pin in some way -- this needs to be very central to the determination
	// of fullness
	sat_state_t sat_state = get_sat_state();
	switch (curr_charge_state)
	{
		case FILL_LI:
			if ((li1_mv > LI_UP_MV && li2_mv > LI_UP_MV) &&
			    !life_po_full_mv &&
				  (sat_state == IDLE_FLASH || sat_state == IDLE_NO_FLASH))
				curr_charge_state = FILL_LF;
			break;

		case FILL_LF:
			if ((li1_mv <= LI_DOWN_MV || li2_mv <= LI_DOWN_MV) ||
				  life_po_full_mv ||
				  (sat_state != IDLE_FLASH && sat_state != IDLE_NO_FLASH))
				curr_charge_state = FILL_LI;
			break;
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
		case FILL_LI:
			// charge the lion with the the lower voltage
			// discharge the lion with the higher voltage
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
				batt_charging = LFB1;
			else
				batt_charging = LFB2;

			// discharge the lion with the higher voltage
			if (li1_mv <= li2_mv)
				batt_discharging = LI2;
			else
				batt_discharging = LI1;

			break;
	}

	/////
	// phase 3: apply the decisions we've made about which batteries to charge!
	/////

	// TODO*: reconsider, down the road, the effects of the suspend and resume all
	// TODO*: make very sure this doesn't hang
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

	// TODO*: make sure this went through by looking at the CHG PIN
	// TODO*: this should be done with more error checks
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

		// TODO*: make sure this went through by looking at CHG
		set_output(i == batt_charging, charge_pin);
	}

	xTaskResumeAll();
	#endif
}
