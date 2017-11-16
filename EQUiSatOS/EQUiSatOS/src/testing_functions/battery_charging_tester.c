/*
 * battery_charging_tester.c
 *
 * Created: 11/2/2017 8:30:13 PM
 *  Author: rj16
 */ 

#include "battery_charging_tester.h"

// Note: we need the test flag to be set as true

// The goals of testing are as follow:
//  - run single iterations of battery logic to assert its correctness in isolation
//  - run many iterations of the battery logic to assert that over time, the battery
//    logic will converge to a good state (it will be a challenge to run this simulation 
//    really well)

// will lead to full_lion state
// no lion history, so will pick the lower lion
void run_unit_test_one()
{
	int lion_one_percentage = low - 2;
	int lion_two_percentage = low - 4;
	int lion_bank_one_percentage = 80;
	int lion_bank_two_percentage = 84;
	
	batt_charging[0] = 0;
	batt_charging[1] = 0;
	batt_charging[2] = 1;
	batt_charging[3] = 0;
		
	battery_logic(lion_one_percentage, lion_two_percentage, lion_bank_one_percentage, lion_bank_two_percentage);
		
	assert(batt_charging[0] == 0);
	assert(batt_charging[1] == 1);
	assert(batt_charging[2] == 0);	
	assert(batt_charging[3] == 0);
}

// will lead to full_lion state
// weird lion history, so will pick the lower lion
void run_unit_test_two()
{
	int lion_one_percentage = low - 2;
	int lion_two_percentage = 100;
	int lion_bank_one_percentage = 80;
	int lion_bank_two_percentage = 84;
	
	batt_charging[0] = 1;
	batt_charging[1] = 1;
	batt_charging[2] = 1;
	batt_charging[3] = 0;
	
	battery_logic(lion_one_percentage, lion_two_percentage, lion_bank_one_percentage, lion_bank_two_percentage);
	
	assert(batt_charging[0] == 1);
	assert(batt_charging[1] == 0);
	assert(batt_charging[2] == 0);
	assert(batt_charging[3] == 0);
}

// will lead to full_lion state
// some lion history, will stick with the currently charging lion
void run_unit_test_three()
{
	int lion_one_percentage = low - 8;
	int lion_two_percentage = low - difference + 1;
	int lion_bank_one_percentage = 80;
	int lion_bank_two_percentage = 84;
	
	batt_charging[0] = 0;
	batt_charging[1] = 1;
	batt_charging[2] = 1;
	batt_charging[3] = 1;
	
	battery_logic(lion_one_percentage, lion_two_percentage, lion_bank_one_percentage, lion_bank_two_percentage);
	
	assert(batt_charging[0] == 0);
	assert(batt_charging[1] == 1);
	assert(batt_charging[2] == 0);
	assert(batt_charging[3] == 0);
}

// will lead to full_lion state
// some lion history but will bail
void run_unit_test_four()
{
	int lion_one_percentage = low - 8;
	int lion_two_percentage = lion_one_percentage - difference;
	int lion_bank_one_percentage = 80;
	int lion_bank_two_percentage = 84;
	
	batt_charging[0] = 0;
	batt_charging[1] = 1;
	batt_charging[2] = 1;
	batt_charging[3] = 1;
	
	battery_logic(lion_one_percentage, lion_two_percentage, lion_bank_one_percentage, lion_bank_two_percentage);
	
	assert(batt_charging[0] == 1);
	assert(batt_charging[1] == 0);
	assert(batt_charging[2] == 0);
	assert(batt_charging[3] == 0);
}

// will lead to full_life_po state
// will pick lower life_po
void run_unit_test_five()
{
	int lion_one_percentage = high;
	int lion_two_percentage = lion_one_percentage - difference;
	int lion_bank_one_percentage = 80;
	int lion_bank_two_percentage = 84;
		
	batt_charging[0] = 0;
	batt_charging[1] = 1;
	batt_charging[2] = 1;
	batt_charging[3] = 1;
	
	battery_logic(lion_one_percentage, lion_two_percentage, lion_bank_one_percentage, lion_bank_two_percentage);
	
	assert(batt_charging[0] == 1);
	assert(batt_charging[1] == 0);
	assert(batt_charging[2] == 0);
	assert(batt_charging[3] == 0);
}

void run_core_logic_unit_tests() 
{
	run_unit_test_one();
	run_unit_test_two();
	run_unit_test_three();	
	run_unit_test_four();
}

void run_convergence_tests()
{
	// TODO: fill this out
}

void run_battery_charging_tests()
{
	run_core_logic_unit_tests();
	run_convergence_tests();
}


