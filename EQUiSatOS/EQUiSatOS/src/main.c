#include "main.h"
#include "runnable_configurations/flatsat.h"
#include "runnable_configurations/bat_testing.h"
#include "runnable_configurations/antenna_pwm.h"
#include "data_handling/package_transmission.h"

static void run_tests(void) {
	//mram_test(); // make sure to not run init_persistent_storage() before this
	//pwm_test();
	//sensor_read_tests();
	//actuallyFlashingFlashBurstTest(); // as the name implies, this does ACTUALLY FLASH THE SATELLITES BIG LEDS
	//flatsat_run(); //calls flatsat init
	//bat_testing_run();
	//test_equistack();
	//assert_rtos_constants();
	//assert_transmission_constants();

	//system_test();
}

void set_charging_states(bool lion1, bool lion2, bool lifepo_b1, bool lifepo_b2) {
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
	set_charging_states(true, true, true, true);
	system_test(true);
	run_tests();

	run_rtos();
}
