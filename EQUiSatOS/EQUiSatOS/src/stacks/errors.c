/*
 * errors.c
 *
 * Created: 12/11/2016 2:20:14 PM
 *  Author: jleiken
 */ 
// Basic implementation from
// http://groups.csail.mit.edu/graphics/classes/6.837/F04/cpp_notes/stack1.html
#include "errors.h"

void free_error_data(error_data_t* es)
{
	for (int i = 0; i < sizeof(es->errors); i++)
	{
		vPortFree(&(es->errors[i]));
	}
	vPortFree(&(es->errors));
	vPortFree(&es);
}

void error_Stack_Init(error_Stack* S)
{
	S->size = 0;
	S->top_index = -1;
	S->bottom_index = -1;
}

error_data_t* error_Stack_Top(error_Stack* S)
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
void error_Stack_Push(error_Stack* S, error_data_t* val)
{
	S->top_index = (S->top_index + 1) % ERROR_STACK_MAX;
	
	// something was overwritten
	// could also be
	// if (size == max_size)
	if (S->bottom_index == S->top_index)
	{
		S->bottom_index = (S->bottom_index + 1) % ERROR_STACK_MAX;
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