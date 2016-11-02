#include "main.h"

int main(void)
{
	/*
	// Initialize the SAM system
	system_init();
		
	Num_Stack s1;	
	
	Stack_Init(&s1);
	
	Stack_Add_Front(&s1, 1);
	// [1, -, -, -]
		
	Stack_Add_Front(&s1, 2);
	// [1, 2, -, -]
		
	Stack_Add_Front(&s1, 3);
	// [1, 2, 3, -]
		
	Stack_Add_Front(&s1, 4);
	// [1, 2, 3, 4]
		
	Stack_Add_Front(&s1, 5);
	// [5, 2, 3, 4]
		
	Stack_Add_Front(&s1, 6);
	// [5, 6, 3, 4]
		
	Stack_Remove_Back(&s1);
	// [5, 6, -, 4]
		
	Stack_Remove_Front(&s1);
	// [5, -, -, 4]
	
	Stack_Remove_Front(&s1);
	// [-, -, -, 4]
	
	Stack_Remove_Back(&s1);
	// [-, -, -, -]
		
	Stack_Add_Front(&s1, 1);
	// [1, -, -, -]
	
	Stack_Add_Front(&s1, 2);
	// [1, 2, -, -,]
	
	int top_1 = Stack_Top(&s1);
	int bottom_1 = Stack_Bottom(&s1);
	
	Stack_Remove_Back(&s1);
	// [-, 2, -, -]
		
	int top_2 = Stack_Top(&s1);
	int bottom_2 = Stack_Bottom(&s1);
		
	Stack_Remove_Back(&s1);
	// [-, -, -, -]
		
	int top_3 = Stack_Top(&s1);
	int bottom_3 = Stack_Bottom(&s1);
	*/
	
	system_init();
	
	Num_Stack test;
	Stack_Init(&test);
	
	Stack_Remove_Back(&test);
	for(int i = 1; i < 8; i++)
	{
		Stack_Add_Front(&test, i);
	}
	Stack_Remove_Front(&test);
	
	int16_t returner[2];
	Stack_Iterate(&test, 2, &returner);
	
	//runit();
	runit_2();
}

