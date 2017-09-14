//#include "runnable_configurations/flatsat.h"
#include "runnable_configurations/isItOn.h"
#include "main.h"

int main(void)
{
	// Initialize the SAM system
	//system_init();
	
	//flatsat_run();
	isItOnInit();
	resetCount();
	readCount();
	//radioTest();
	return 0;
}