/*
 * flash_Stack.c
 *
 * Created: 11/1/2016 9:20:30 PM
 *  Author: jleiken
 */

// Basic implementation from
// http://groups.csail.mit.edu/graphics/classes/6.837/F04/cpp_notes/stack1.html
#include "flash_Stack.h"

flash_Stack* flash_Stack_Init()
{
	flash_Stack* S = pvPortMalloc(sizeof(flash_Stack));
	S->top_index = -1;
	S->bottom_index = -1;
	S->mutex = xSemaphoreCreateMutex();

	return S;
}

flash_data_t* flash_Stack_Top(flash_Stack* S) // TODO: Remove the element you return
{
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
flash_data_t* flash_Stack_Stage(flash_Stack* S, flash_data_t* val)
{
	// TODO: Change if needed
	xSemaphoreTake(S->mutex, (TickType_t) 10);
	S->top_index = (S->top_index + 1) % FLASH_STACK_MAX;

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
	}

	S->data[S->top_index] = val;
	flash_data_t* staged_pointer = S->data[(S->top_index + 1) % FLASH_STACK_MAX];
	clear_existing_data(staged_pointer, sizeof(flash_data_t));

	xSemaphoreGive(S->mutex);
	return staged_pointer; // return pointer to staged data
}
