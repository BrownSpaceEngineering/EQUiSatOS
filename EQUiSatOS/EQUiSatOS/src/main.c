//#include "runnable_configurations/flatsat.h"
#include "runnable_configurations/isItOn.h"
#include "main.h"

int main(void)
{
	// Initialize the SAM system
	//system_init();

	//flatsat_run();
	//isItOnInit();
	//resetCount();
	//readCount();
	//radioTest();

	Num_Stack test;
	Stack_Init(&test);

	Stack_Remove_Back(&test);
	for(int i = 1; i < 8; i++)
	{
		Stack_Add_Front(&test, i);
	}
	Stack_Remove_Front(&test);

	//runit();
	runit_2();
}
