/*
 * equistack.c
 *
 * Created: 3/1/2017 8:46:35 PM
 *  Author: mcken
 */ 
// Basic implementation from
// http://groups.csail.mit.edu/graphics/classes/6.837/F04/cpp_notes/stack1.html
#include "equistack.h"

equistack* equistack_Init(size_t data_size, uint16_t max_size)
{
	equistack* S = pvPortMalloc(sizeof(equistack));
	S->top_index = -1;
	S->bottom_index = -1;
	S->cur_size = 0;
	S->max_size = max_size;
	S->data_size = data_size;
	S->mutex = xSemaphoreCreateMutex();
	
	for (int i = 0; i < max_size; i++)
	{
		S->data[i] = pvPortMalloc(data_size);
	}
	
	return S;
}

void* equistack_Get(equistack* S, int16_t n)
{
	xSemaphoreTake(S->mutex, (TickType_t) MUTEX_WAIT_TIME_TICKS);

	// We want to ignore the bottom index which is currently staged and maybe
	// being overwritten
	if (n < S->cur_size - 1)
	{
		return S->data[(S->top_index - n) % S->max_size];
	}
	else
	{
		return NULL;
	}
	
	xSemaphoreGive(S->mutex);
}

void* equistack_Initial_Stage(equistack* S)
{
	return S->data[0];
}

// Overwrites the bottom value if need be
void* equistack_Stage(equistack* S)
{
	xSemaphoreTake(S->mutex, (TickType_t) 10);
	
	S->top_index = (S->top_index + 1) % S->max_size;
	
	if (S->bottom_index == S->top_index)
	{
		S->bottom_index = (S->bottom_index + 1) % S->max_size;
	}
	else
	{
		S->cur_size++;
		
		if (S->bottom_index == -1)
		{
			S->bottom_index = 0;
		}
	}
	
	equistack* staged_pointer = S->data[(S->top_index + 1) % S->max_size];
	clear_existing_data(staged_pointer, S->data_size);
	
	xSemaphoreGive(S->mutex);
	return staged_pointer; // return pointer to staged data
}
