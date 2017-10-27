//#include "runnable_configurations/flatsat.h"
#include "main.h"
#include "runnable_configurations/flatsat.h"
#include "runnable_configurations/bat_testing.h"

int main(void)
{
	// Initialize the SAM system
	system_init();
	//init_tc();
	USART_init();
	//flatsat_run(); //calls flatsat init

	//init_errors(); // init error stack? (TODO: do this here or start of RTOS?)

	bat_testing_run();

	/* Start and init tracing */
	vTraceEnable(TRC_START);

	run_rtos();

	/* TESTS */
	/*configure_pwm(P_ANT_DRV1, P_ANT_DRV1_MUX);
	init_tc();
	while(get_count() < 3) {
			set_pulse_width_fraction(3, 4);
	}*/

	//disable_pwm();
	//test_equistack();
	//assert_rtos_constants();
	//assert_transmission_constants();
}
