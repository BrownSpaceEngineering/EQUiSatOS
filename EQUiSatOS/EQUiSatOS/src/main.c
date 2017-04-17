#include "runnable_configurations/flatsat.h"
#include "testing_functions/watchdog_task_test.h"
#include "main.h"

int main(void)
{
	// Initialize the SAM system
	system_init();
	
	//watch_test();
	flatsat_run();
	//radioTest();
	
}