/*
 * Num_Stack_Test.c
 *
 * Created: 11/1/2016 9:10:36 PM
 *  Author: rj16
 */ 
#include "Num_Stack_Test.h"

void num_stack_test()
{
	// Functions to test:
	// Stack_Init
	// Stack_Top
	// Stack_Add_Front
	system_init();
	
	Num_Stack test;
	Stack_Init(&test);
	Stack_Init(&test);
	
	for(int i = 1; i < 10; i++)
	{
		Stack_Add_Front(&test, i);
	}
	for(int i = 1; i < 3; i++)
	{
		Stack_Top(&test);
	}
	Stack_Top(&test);
	Stack_Add_Front(&test, 77);
	Stack_Top(&test);
	
	runit();	
}