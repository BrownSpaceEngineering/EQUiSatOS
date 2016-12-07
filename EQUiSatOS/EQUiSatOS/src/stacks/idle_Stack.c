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
	S->mutex = xSemaphoreCreateMutex();

	return S;
}

idle_data_t* idle_Stack_Top(idle_Stack* S) // TODO: Remove the element you return
{
	// TODO:  More precise mutexing
	xSemaphoreTake(S->mutex, (TickType_t) MUTEX_WAIT_TIME_TICKS);

	if (S->top_index != -1)
	{
		// top of full stack is right before staged index
		return S->data[S->top_index];
	}
	else
	{
		// TODO:
		// Return dummy
		return NULL;
	}

	xSemaphoreGive(S->mutex);
}

// Overwrites the bottom value if need be
idle_data_t* idle_Stack_Stage(idle_Stack* S, idle_data_t* val)
{
	// TODO: Change if needed
	xSemaphoreTake(S->mutex, (TickType_t) 10);
	S->top_index = (S->top_index + 1) % IDLE_STACK_MAX;

	// something was overwritten
	// could also be
	// if (size == max_size)
	if (S->bottom_index == S->top_index)
	{
		S->bottom_index = (S->bottom_index + 1) % IDLE_STACK_MAX;
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
	idle_data_t* staged_pointer = S->data[(S->top_index + 1) % IDLE_STACK_MAX];
	clear_existing_data(staged_pointer, sizeof(idle_data_t));

	xSemaphoreGive(S->mutex);
	return staged_pointer; // return pointer to staged data
}
