/*
 * battery_charging_task.c
 *
 * Created: 9/21/2017 20:37:56
 *  Author: mcken
 */ 

#include "battery_charging_task.h"

// High-level TODO's:
//  - implement strikes!
//  - what will change once a battery has struck out?

// number of strikes for each battery
extern int batt_strikes[4] = {0, 0, 0, 0};

void battery_charging_task(void *pvParameters)
{
	/////
	/// initialize global state variables
	/////
	
	curr_charge_state = FILL_LION;
	
	// let's explicitly initialize these to -1 -- we don't have an opinion on the
	// lion that should start out discharging and the battery that should start out
	// charging -- the algorithm will figure these things out for us
	batt_charging = -1;
	batt_discharging = -1; 
		
	// initialize xNextWakeTime onces
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	while (true)
	{
		vTaskDelayUntil(&xNextWakeTime, BATTERY_CHARGING_TASK_FREQ / portTICK_PERIOD_MS);
		
		// TODO: do this with sensor read commands	
		// before getting into it, grab the percentages of each of the batteries
		// lions
		int lion_one_percentage = 0; // get_battery_percentage(LION_ONE);
		int lion_two_percentage = 0; // get_battery_percentage(LION_TWO);
		
		// life po banks
		int life_po_bank_one_percentage = 0; // get_battery_percentage(LIFE_PO_BANK_ONE);
		int life_po_bank_two_percentage = 0; // get_battery_percentage(LIFE_PO_BANK_TWO);
		
		// individual batteries within the life po banks
		int life_po_bank_one_bat_one_percentage = 0; // get_battery_percentage(LIFE_PO_BANK_ONE_BAT_ONE);
		int life_po_bank_one_bat_two_percentage = 0; // get_battery_percentage(LIFE_PO_BANK_ONE_BAT_TWO);
		int life_po_bank_two_bat_one_percentage = 0; // get_battery_percentage(LIFE_PO_BANK_TWO_BAT_ONE);
		int life_po_bank_two_bat_two_percentage = 0; // get_battery_percentage(LIFE_PO_BANK_TWO_BAT_TWO);

		// NOTE: get_global_state and battery_logic are individual functions in order to make
		// it easier to "unit test" them with contrived inputs
		
		// what's the global state of the satellite?
		int curr_global_state = get_global_state(
			lion_one_percentage, 
			lion_two_percentage, 
			life_po_bank_one_percentage, 
			life_po_bank_two_percentage);	
		// TODO: set the global state to the return value of charge state
		
		// what batteries should we be charging?
		battery_logic(
			lion_one_percentage, 
			lion_two_percentage, 
			life_po_bank_one_percentage, 
			life_po_bank_two_percentage,
			life_po_bank_one_bat_one_percentage,
			life_po_bank_one_bat_two_percentage,
			life_po_bank_two_bat_one_percentage,
			life_po_bank_two_bat_two_percentage);
	}
	
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}

int get_global_state(int lion_one_percentage, int lion_two_percentage, int life_po_bank_one_percentage, int life_po_bank_two_percentage) 
{
	// TODO:
	// return "rip" if the global state is rip
	
	// enter the rip state if both are critical
	if (lion_one_percentage <= critical && lion_two_percentage <= critical)
	{
		int end_of_life = 1;
			
		// we want to be very sure that both are actually critical
		for (int i = 0; i < 5; i++)
		{
			// TODO: get these for real
			int recalc_lion_one_percentage = 0; // get_battery_percentage(LION_ONE);
			int recalc_lion_two_percentage = 0; // get_battery_percentage(LION_TWO);
				
			if (!(recalc_lion_one_percentage <= critical && recalc_lion_two_percentage <= critical))
			{
				end_of_life = 0;
				break;
			}
				
			// TODO: add a delay here
		}
			
		if (end_of_life)
		{
			// TODO: return "rip"
		}
		else
		{
			// TODO: send a hi-pri error
		}
	}
	
	// TODO: return "hello world" if the global state is "hello world"
	
	// TODO: return "antenna deplot" if the global state is "antenna deploy"
	
	// enter low power if lion's aren't great
	if (lion_one_percentage < med || lion_two_percentage < med)
	{
		// TODO: return "low power"
	}
	
	if (!(life_po_bank_one_percentage > high && life_po_bank_two_percentage > high))
	{
		// TODO: return "idle no flash"
	} 
	
	// TODO: return "idle flash"
}

void battery_logic(
	int lion_one_percentage, 
	int lion_two_percentage, 
	int life_po_bank_one_percentage, 
	int life_po_bank_two_percentage,
	int life_po_bank_one_bat_one_percentage,
	int life_po_bank_one_bat_two_percentage,
	int life_po_bank_two_bat_one_percentage,
	int life_po_bank_two_bat_two_percentage)
{	
	// we often want to know the higher of the cells within the life po banks
	int max_life_po_bank_one = Max(life_po_bank_one_bat_one_percentage, life_po_bank_one_bat_two_percentage);
	int max_life_po_bank_two = Max(life_po_bank_two_bat_one_percentage, life_po_bank_two_bat_two_percentage);
	
	// we often want to know whether the higher cells within the life po banks have filled up
	bool life_po_full = max_life_po_bank_one > full && max_life_po_bank_two > full;
	
	/////
	// phase 0: determine whether any batteries should have their strikes incremented
	/////
	
	// TODO: Implement this. Some of the remaining q's:
	//   - How does striking out work for life po? It might be hard to avoid false positives
	//     (wrongly adding a strike) with a diff. in the charge of the cells.
	//   - What do we do if we strike out?
	
	/////
	// phase 1: determine whether we want to make a state change
	/////
	
	// the flow is as follows: we should charge the lion's up to full, and in the time between
	// their filling up and their dropping down below an optimal state, we should charge the
	// lifepo
	
	// more concretely:
	//  - if we're in fill_lion, we switch if the lifepo's aren't full and both lion's are
	//  - if we're in fill_life_po, we switch if the either lion is below med, or both are
	//    below high
	
	// this isn't synonymous with the global state, but they're related in the following ways:
	//  - if the global state is idle flash or idle not flash, we can be in either FILL_LION,
	//    or FILL_LIFE_PO
	//  - otherwise, FILL_LION is the only option
	
	// TODO: set the charge state to FILL_LION if the global state is not idle_charge or
	// idle_no_charge
	// TODO: else...
	
	switch (curr_charge_state) 
	{
		// NOTE: these conditions are a subset of the low power conditions -- if it's low
		// power, it'll be full lion
		case FILL_LION:
			if (!life_po_full && (lion_one_percentage > full && lion_two_percentage > full))
			{
				curr_charge_state = FILL_LIFE_PO;	
			}
			
			break;
		
		case FILL_LIFE_PO:
			if ((lion_one_percentage < high && lion_two_percentage < high) || 
				(lion_one_percentage < med || lion_two_percentage < med) ||
				life_po_full)
			{
				curr_charge_state = FILL_LION;
			}
		
			break;	
	}
	
	/////
	// phase 2: determine which batteries should be charging
	/////
				
	switch (curr_charge_state)
	{
		case FILL_LION:
			// charge the lion with the the lower percentage
			// discharge the lion with the higher percentages
			if (lion_one_percentage <= lion_two_percentage)
			{
				batt_charging = LION_ONE;
				batt_discharging = LION_TWO;
			}
			else
			{
				batt_charging = LION_TWO;
				batt_discharging = LION_ONE;
			}
			
			break;
		
		case FILL_LIFE_PO:
			// charge the life po bank with the lower max percentage
			// Note: this might lead to issues if the cells are very
			// unbalanced
			if (max_life_po_bank_one <= max_life_po_bank_two)
			{
				batt_charging = LIFE_PO_BANK_ONE;
			}
			else
			{
				batt_charging = LIFE_PO_BANK_TWO;
			}
			
			// discharge the lion with the the higher percentage
			if (lion_one_percentage <= lion_two_percentage)
			{
				batt_discharging = LION_TWO;
			}
			else
			{
				batt_discharging = LION_ONE;
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
	if (batt_discharging == LION_ONE)
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
	setup_pin(discharge_pin);
	set_output(true, discharge_pin);
	setup_pin(not_discharge_pin);
	set_output(false, not_discharge_pin);
	
	// set the battery that should be charging to charge
	// set the others to not charge
	for (int i = 0; i < 4; i++)
	{
		int charge_pin = 0;
		switch (i)
		{
			case LION_ONE:
				charge_pin = P_L1_RUN_CHG;
				break;
			
			case LION_TWO:
				charge_pin = P_L2_RUN_CHG;
				break;
			
			case LIFE_PO_BANK_ONE:
				charge_pin = P_LF_B1_RUNCHG;
				break;
				
			case LIFE_PO_BANK_TWO:
				charge_pin = P_LF_B2_RUNCHG;
				break;
		}
		
		// TODO: make sure this went through by looking at CHG
		setup_pin(charge_pin);
		set_output(i == batt_charging, charge_pin);
	}
	
	xTaskResumeAll();
	#endif
}
