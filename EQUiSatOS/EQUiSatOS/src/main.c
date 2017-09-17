//#include "runnable_configurations/flatsat.h"
#include "runnable_configurations/isItOn.h"
#include "main.h"
#include "processor_drivers\USART_Commands.h"

int main(void)
{
	// Initialize the SAM system
	system_init();
	USART_init();

	/* TESTS */
	//test_equistack();
	//assertConstantDefinitions();

	run_rtos();
}
