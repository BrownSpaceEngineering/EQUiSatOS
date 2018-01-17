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

void run_core_logic_unit_tests(void) 
{
	// TODO: fill this out
}

void run_convergence_tests(void)
{
	// TODO: fill this out
}

void run_battery_charging_tests(void)
{
	run_core_logic_unit_tests();
	run_convergence_tests();
}


