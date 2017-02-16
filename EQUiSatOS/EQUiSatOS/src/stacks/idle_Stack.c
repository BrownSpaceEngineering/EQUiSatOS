/*
 * idle_Stack.c
 *
 * Created: 11/1/2016 9:19:41 PM
 *  Author: jleiken
 */ 

// Basic implementation from
// http://groups.csail.mit.edu/graphics/classes/6.837/F04/cpp_notes/stack1.html
#include "idle_Stack.h"

idle_Stack* idle_Stack_Init()
{
	idle_Stack* S = pvPortMalloc(sizeof(idle_Stack));
	S->top_index = -1;
	S->bottom_index = -1;
	S->size = 0;
	S->mutex = xSemaphoreCreateMutex();
	
	for (int i = 0; i < IDLE_STACK_MAX; i++)
	{
		S->data[i] = pvPortMalloc(sizeof(idle_data_t));
	}
	
	return S;
}

idle_data_t* idle_Stack_Get(idle_Stack* S, int16_t n)
{
	xSemaphoreTake(S->mutex, (TickType_t) MUTEX_WAIT_TIME_TICKS);

	// We want to ignore the bottom index which is currently staged and maybe
	// being overwritten	
	if (n < S->size - 1)
	{
		return S->data[(S->top_index - n) % IDLE_STACK_MAX];
	}
	else
	{
		return NULL;
	}
	
	xSemaphoreGive(S->mutex);
}

idle_data_t* idle_Stack_Initial_Stage(idle_Stack* S)
{
	return S->data[0];
}

// Overwrites the bottom value if need be
idle_data_t* idle_Stack_Stage(idle_Stack* S)
{
	xSemaphoreTake(S->mutex, (TickType_t) 10);
	
	S->top_index = (S->top_index + 1) % IDLE_STACK_MAX;
	
	if (S->bottom_index == S->top_index)
	{
		S->bottom_index = (S->bottom_index + 1) % IDLE_STACK_MAX;
	}
	else
	{
		S->size++;
		
		if (S->bottom_index == -1)
		{
			S->bottom_index = 0;
		}
	}
	
	idle_data_t* staged_pointer = S->data[(S->top_index + 1) % IDLE_STACK_MAX];
	clear_existing_data(staged_pointer, sizeof(idle_data_t));
	
	xSemaphoreGive(S->mutex);
	return staged_pointer; // return pointer to staged data
}