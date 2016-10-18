#include "runnable_configurations/flatsat.h"
#include "main.h"

int main(void)
{
	// Initilize the SAM system
	system_init();
	flatsat_run();
}