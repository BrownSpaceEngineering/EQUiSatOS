#include "main.h"

int main(void)
{	
	system_init();
	
	Num_Stack test;
	Stack_Init(&test);
	
	Stack_Remove_Back(&test);
	for(int i = 1; i < 8; i++)
	{
		Stack_Add_Front(&test, i);
	}
	Stack_Remove_Front(&test);
	
	runit();
	//runit_2();
}

