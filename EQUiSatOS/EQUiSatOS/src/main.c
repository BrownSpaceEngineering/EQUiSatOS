//#include "runnable_configurations/flatsat.h"
#include "runnable_configurations/isItOn.h"
#include "main.h"
#include "processor_drivers\USART_Commands.h"
#include "processor_drivers\PWM_Commands.h"
#include "processor_drivers\Direct_Pin_Commands.h"

int main(void)
{
	// Initialize the SAM system
	system_init();
	//USART_init();
	init_errors(); // init error stack? (TODO: do this here or start of RTOS?)

	/* TESTS */
// 	configure_pwm(P_ANT_DRV1, P_ANT_DRV1_MUX);
// 	init_tc();
// 	while(get_count() < 3) {
// 			set_pulse_width_fraction(3, 4);		
// 	}
	//disable_pwm();		
	//test_equistack();
	//assert_rtos_constants();
	//assert_transmission_constants();

	run_rtos();
}
