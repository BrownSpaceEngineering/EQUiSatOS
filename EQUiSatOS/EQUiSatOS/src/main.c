//#include "runnable_configurations/flatsat.h"
#include "runnable_configurations/isItOn.h"
#include "main.h"
#include "processor_drivers\USART_Commands.h"

int main(void)
{
	// Initialize the SAM system
	system_init();
<<<<<<< bf15b42176a30acd3859a4335cf6b82a2d1c0050
	// runit_2();
	test_equistack();
}
=======
	USART_init();
	runit_2();
/*	test_equistack();*/
}
>>>>>>> Finish constant testing
