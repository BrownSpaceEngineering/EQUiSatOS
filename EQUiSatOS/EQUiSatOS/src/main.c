//#include "runnable_configurations/flatsat.h"
#include "runnable_configurations/isItOn.h"
#include "main.h"
#include "processor_drivers\USART_Commands.h"
#include "processor_drivers\PWM_Commands.h"

int main(void)
{
	// Initialize the SAM system
	system_init();
	//USART_init();

	/* TESTS */
	configure_pwm(P_ANT_DRV3, P_ANT_DRV3_MUX);
	set_pulse_width_fraction(3, 4);
	disable_pwm();
	//test_equistack();
	//assertConstantDefinitions();

	run_rtos();
}
