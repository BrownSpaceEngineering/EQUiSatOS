#include "main.h"
#include "runnable_configurations/flatsat.h"
#include "runnable_configurations/bat_testing.h"
#include "runnable_configurations/antenna_pwm.h"
#include "data_handling/package_transmission.h"

void run_tests(void) {
	//mram_test(); // make sure to not run init_persistent_storage() before this
	//pwm_test();
	//sensor_read_tests();
	//actuallyFlashingFlashBurstTest(); // as the name implies, this does ACTUALLY FLASH THE SATELLITES BIG LEDS
	//flatsat_run(); //calls flatsat init
	//bat_testing_run();
	//test_equistack();
	//assert_transmission_constants();
	//pointer_typecast_test();
	//longest_same_seq_len_test();
	//radioTest();

	//system_test();
}

// function to run tests while RTOS scheduler is running (called in testing_tasks.h)
// (note they will be run periodically according to TESTING_TASK_FREQ)
// IMPORTANT: 
// When running these to run you'll need to:
//	- make sure RUN_TESTING_TASKS is defined
//  - customize these in config.h
//		- OVERRIDE_INIT_SAT_STATE to be the starting state AND/OR
//		- OVERRIDE_INIT_TASK_STATES to set the startup state of each task
//		- OVERRIDE_STATE_HOLD_INIT to define whether to NOT change states automatically
//  - (make sure RTOS is started)
void run_rtos_tests(void) {
	vTaskDelay(2000); // don't be a CPU hog
}

static void set_charging_states(bool lion1, bool lion2, bool lifepo_b1, bool lifepo_b2) {
	setup_pin(true, P_L1_RUN_CHG);
	set_output(lion1, P_L1_RUN_CHG);

	setup_pin(true, P_L2_RUN_CHG);
	set_output(lion2, P_L2_RUN_CHG);

	setup_pin(true, P_LF_B1_RUNCHG);
	set_output(lifepo_b1, P_LF_B1_RUNCHG);

	setup_pin(true, P_LF_B2_RUNCHG);
	set_output(lifepo_b2, P_LF_B2_RUNCHG);
}

int main(void)
{
	global_init();
	#ifndef RELEASE
		//system_test(false);
		set_charging_states(false, true, false, false); // WARNING: try to only charge one at a time
		run_tests();
	#endif

	run_rtos();
}