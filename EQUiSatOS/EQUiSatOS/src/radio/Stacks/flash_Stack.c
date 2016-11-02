/*
 * flash_Stack.c
 *
 * Created: 11/1/2016 9:20:30 PM
 *  Author: jleiken
 */ 

// Basic implementation from
// http://groups.csail.mit.edu/graphics/classes/6.837/F04/cpp_notes/stack1.html
#include "flash_Stack.h"

void flash_Stack_Init(flash_Stack* S)
{
	S->size = 0;
	S->top_index = -1;
	S->bottom_index = -1;
}

flash_data_t* flash_Stack_Top(flash_Stack* S)
{
	// Could also be S->size == 0
	// TODO: Think about whether we need second conditional
	if (S->top_index != -1 && S->top_index < S->size)
	{
		return S->data[S->top_index];
	}
	else
	{
		// TODO:
		// Return dummy
		return NULL;
	}
}

// Overwrites the bottom value if need be
void flash_Stack_Push(flash_Stack* S, flash_data_t* val)
{
	S->top_index = (S->top_index + 1) % FLASH_STACK_MAX;
	
	// something was overwritten
	// could also be
	// if (size == max_size)
	if (S->bottom_index == S->top_index)
	{
		S->bottom_index = (S->bottom_index + 1) % FLASH_STACK_MAX;
	}
	else
	{
		if (S->bottom_index == -1)
		{
			S->bottom_index = 0;
		}
		
		S->size++;
	}
	
	
	S->data[S->top_index] = val;
}