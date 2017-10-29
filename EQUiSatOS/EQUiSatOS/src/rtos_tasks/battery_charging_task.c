/*
 * battery_charging_task.c
 *
 * Created: 9/21/2017 20:37:56
 *  Author: mcken
 */ 

#include "battery_charging_task.h"

// current state
int state = FULL_LION;

// the batteries that are currently charging
// zero -- not charging, one -- charging
// this array represents both the past state and the current state
// it's smartly refreshed such that if it's old values are needed,
// they aren't overwritten until after they're needed 
bool charging[4] = {false, false, false, false};

// number of strikes for each battery
bool strikes[4] = {false, false, false, false};

// counts up from zero as the method gets executed
int iteration_count = 0;

int get_battery_percentage(battery batt)
{
	struct adc_module adc_instance;
	struct adc_module adc_instance1;
	uint16_t volts;
	// for explanation of conversions (multiplication) see Battery Board V2 Datasheet
	switch (batt) {
		case LION_ONE:
			configure_adc(&adc_instance, P_AI_L1_REF);
			volts = read_adc(adc_instance) * 2.5;
			break;
		case LION_TWO:
			configure_adc(&adc_instance, P_AI_L2_REF);
			volts = read_adc(adc_instance) * 2.5;
			break;
		case LIFE_PO_BANK_ONE:
			configure_adc(&adc_instance, P_AI_LF1REF);
			volts = read_adc(adc_instance) * 3.87;
			configure_adc(&adc_instance1, P_AI_LF2REF);
			volts -= read_adc(adc_instance1);
			break;
		case LIFE_PO_BANK_TWO:
			configure_adc(&adc_instance, P_AI_LF3REF);
			volts = read_adc(adc_instance) * 3.87;
			configure_adc(&adc_instance1, P_AI_LF4REF);
			volts -= read_adc(adc_instance1);
			break;
	}
	// TODO: convert to percentages
	return volts;
}

void set_battery_charge(battery batt, bool should_charge)
{
	int disg = -1, pin;
	// find the proper pins based on batt
	switch (batt) {
		case LION_ONE:
			pin = P_L1_RUN_CHG;
			disg = P_L1_DISG;
			break;
		case LION_TWO:
			pin = P_L2_RUN_CHG;
			disg = P_L2_DISG;
			break;
		case LIFE_PO_BANK_ONE:
			pin = P_LF_B1_RUNCHG;
			break;
		case LIFE_PO_BANK_TWO:
			pin = P_LF_B2_RUNCHG;
			break;
	}
	
	if (disg != -1) {
		// if batt is LiON, stop its discharging before charging it
		if (should_charge) {
			setup_pin(true, disg);
			set_output(!should_charge, pin);
			setup_pin(true, pin);
			set_output(should_charge, pin);
		} else {
			setup_pin(true, pin);
			set_output(should_charge, pin);
			setup_pin(true, disg);
			set_output(!should_charge, pin);
		}
	} else {
		// if batt is LiFePO, just set its charge to should_charge
		setup_pin(true, pin);
		set_output(should_charge, pin);
	}
}

void reset_charging()
{
	for (int i = 0; i < 4; i++)
	{
		charging[i] = 0;
	}
}


void battery_charging_task(void *pvParameters)
{
	// TODO: consider the beginning case; the current iteration
	// of this logic makes a number of assumptions about the
	// initial state of the program
	
	// initialize xNextWakeTime onces
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	for ( ;; )
	{ 	
		vTaskDelayUntil(&xNextWakeTime, BATTERY_CHARGING_TASK_FREQ / portTICK_PERIOD_MS);
		
		// before getting into it, grab the percentages of each of the  batteries
		int lion_one_percentage = get_battery_percentage(LION_ONE);
		int lion_two_percentage = get_battery_percentage(LION_TWO);
		int life_po_bank_one_percentage = get_battery_percentage(LIFE_PO_BANK_ONE);
		int life_po_bank_two_percentage = get_battery_percentage(LIFE_PO_BANK_TWO);
		
		// every five minutes we want to check whether or not we're in the end of
		// life case
		// send end of life messages if both batteries are critical
		if (lion_one_percentage <= critical && lion_two_percentage <= critical)
		{
			int end_of_life = 1;
			
			// we want to be very sure that both are actually critical
			for (int i = 0; i < 3; i++)
			{
				int recalc_lion_one_percentage = get_battery_percentage(LION_ONE);
				int recalc_lion_two_percentage = get_battery_percentage(LION_TWO);
				
				if (!(recalc_lion_one_percentage <= critical 
					  && recalc_lion_two_percentage <= critical)) 
				{
					end_of_life = 0;
					break;
				}
				
				// TODO: maybe add a delay here, too
			}
			
			if (end_of_life)
			{
				// TODO: end life :(
			}
			else
			{
				// TODO: send a hi-pri error
			}
		}
		
		// every ninety minutes, we want to run the core battery logic
		if (iteration_count >= 18) 
		{
			// reset the iteration count
			iteration_count = -1;
			
			// TODO: determine whether the battery that's currently charging has struck out
			
			// TODO: have any batteries struck out? -- if so, we want to transition to another
			// battery charging task
			
			/////
			// phase 1: determine the "charging state" and decide whether to go into low power mode
			/////
			
			// the possible states are full charging of one lion battery (full_lion), full charging of one life po 
			// battery (full_life_po), split charging of a lion battery and a life po battery (half_lion_half_life_po), 
			// and split charging of two lion batteries (full_lion_split)
			// note: full/half indicate the proportion of the power for that type of battery
			
			// the general logic is as follows:
			// either lion is low: full_lion -- we also may want to go into low power mode
			// both lion are full: full_life_po -- unless both life po are full, then full_lion_split
			// otherwise (this is the standard outcome): half_lion_half_life_po -- unless both life po are full, 
			//   then full_lion
			
			// if either of the lion's are low
			if (lion_one_percentage < low || lion_two_percentage < low) 
			{
				state = FULL_LION;
				
				if (lion_one_percentage < low && lion_two_percentage < low) 
				{
					// send an error message
					// go into low power mode
				} 
				else if (lion_two_percentage < critical || lion_two_percentage < critical)
				{
					// send a stronger error message
					// go into low power mode
				}
			}
			// if both lion's are full
			else if (lion_one_percentage > high && lion_two_percentage > high)
			{
				// if both life po's aren't also full
				if (!(life_po_bank_two_percentage > high && life_po_bank_two_percentage > high)) 
				{
					state = FULL_LIFE_PO;
				}
				// everything is full!
				else
				{
					state = FULL_LION_SPLIT;
				}
			}
			// it isn't the case that a lion is in danger -- and we have to focus fully there --
			// and it isn't the case that the lions are totally full -- and we can afford to ignore them
			// we should be dividing our power unless both lifepo's are full
			else
			{
				// if both life po's aren't also full
				if (!(life_po_bank_two_percentage > high && life_po_bank_two_percentage > high))
				{
					state = HALF_LION_HALF_LIFE_PO;
				}
				// both life po's are full -- let's just focus on the lions
				else
				{
					state = FULL_LION;
				}
			}
			
			/////
			// phase 2: based on the state determined above -- and in some cases, the batteries that are charging
			//   at the moment -- manipulates the "charging" array
			/////
			
			// the general logic is as follows:
			// full_life_po: set the lower life po to charge
			// full_lion_split: set both lions to charge
			// half_lion_half_life_po: set the lower life po to charge and then run logic for full_lion, below
			// full_lion: try hard to keep charging the current lion -- we want to be aggressive about getting
			//   it to full; we'll make a change however in the following cases:
			//   - it's full
			//   - the other lion has dropped considerably below: this is a case we want to avoid, too
			
			if (state == FULL_LIFE_PO) 
			{
				reset_charging();
				
				// charge the life po bank with the lower percentage
				if (life_po_bank_one_percentage <= life_po_bank_two_percentage)
				{
					charging[LIFE_PO_BANK_ONE] = 1;
				}
				else
				{
					charging[LIFE_PO_BANK_TWO] = 1;
				}
			}
			else if (state == FULL_LION_SPLIT)
			{
				reset_charging();
				
				// charge both lion's
				charging[LION_ONE] = 1;
				charging[LION_TWO] = 1;
			}
			else
			{
				// was charging one or the other
				// will try to stick with the battery we had been charging previously
				if (charging[LION_ONE] ^ charging[LION_TWO])
				{
					// grab the percentage of the battery that's current charging
					int charging_percentage;
					int other_percentage;
					if (charging[LION_ONE]) 
					{
						charging_percentage = lion_one_percentage;
						other_percentage = lion_two_percentage;
					}
					else
					{
						charging_percentage = lion_two_percentage;
						other_percentage = lion_one_percentage;
					}
					
					// either the battery that's currently charging is full,
					// or it's significantly higher than the other batter
					// in these cases, we switch
					if ((charging_percentage >= high && other_percentage < high) ||
						(other_percentage <= charging_percentage - difference))
					{
						charging[LION_ONE] = !charging[LION_ONE];
						charging[LION_TWO] = !charging[LION_TWO];
					}
					
					// otherwise we do nothing else -- we want to keep charging the lions that 
					// we're charging
				}
				// was charging either both or neither of the lions
				else
				{
					// charge the lion with the the lower percentage
					if (lion_one_percentage <= lion_two_percentage)
					{
						charging[LION_ONE] = 1;
						charging[LION_TWO] = 0;
					}
					else
					{
						charging[LION_ONE] = 0;
						charging[LION_TWO] = 1;
					}
				}
				
				// start out with neither life po charging
				charging[LIFE_PO_BANK_ONE] = 0;
				charging[LIFE_PO_BANK_TWO] = 0;
				
				// if we're splitting half, also add the life po bank with the lower
				// percentage
				if (state == HALF_LION_HALF_LIFE_PO)
				{
					// charge the life po bank with the lower percentage
					if (life_po_bank_one_percentage <= life_po_bank_two_percentage)
					{
						charging[LIFE_PO_BANK_ONE] = 1;
					}
					else
					{
						charging[LIFE_PO_BANK_TWO] = 1;
					}
				}
			}
			
			/////
			// phase 3: apply the decisions we've made about which batteries to charge!
			/////
			
			for (int i = 0; i < 4; i++)
			{
				set_battery_charge(i, charging[i]);
			}
		}
		
		iteration_count++;
	}
	
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}
