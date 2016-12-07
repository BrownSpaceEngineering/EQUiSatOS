/*
 * low_power_Stack.c
 *
 * Created: 11/1/2016 9:19:41 PM
 *  Author: jleiken
 */ 

// Basic implementation from
// http://groups.csail.mit.edu/graphics/classes/6.837/F04/cpp_notes/stack1.html
#include "low_power_Stack.h"

void low_power_Stack_Init(low_power_Stack* S)
{
	S->size = 0;
	S->top_index = -1;
	S->bottom_index = -1;
}

low_power_data_t* low_power_Stack_Top(low_power_Stack* S)
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
void low_power_Stack_Push(low_power_Stack* S, low_power_data_t* val)
{
	S->top_index = (S->top_index + 1) % LOW_POWER_STACK_MAX;
	
	// something was overwritten
	// could also be
	// if (size == max_size)
	if (S->bottom_index == S->top_index)
	{
		S->bottom_index = (S->bottom_index + 1) % LOW_POWER_STACK_MAX;
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
