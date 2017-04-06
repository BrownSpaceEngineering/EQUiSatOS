/*
 * equistack.c
 *
 * Created: 3/1/2017 8:46:35 PM
 *  Author: mcken
 */ 

//////////////////////////////////////////////////////////////////////////
// Equistacks are described here:
// To the user, equistacks can be thought of as 'normal' stacks that hold 
// some number of data structs except for a couple exceptions:
// - When structs are added past the defined max length, they replace the
//   oldest elements.
// - The "get" method will return a pointer to the nth most recent data struct
//   where n is user input.
// - The stack supports a model of editing in which the user can only modify
//   the stack by "staging." When the user stages, they get a pointer to the
//   next element to be edited. This is the staged element; the user is free
//   to edit it. The staged element, however, is not considered "part of the
//   stack." When the user queries the stack, it will never be returned. But
//   the element that was previously staged will become a "part of the stack"
//   that can be accessed with "get."
// - The "initial stage" method will return a pointer to the first element in
//   the array of structs. It's only meant to be used when the user is first
//   interacting with the equistack. 
//////////////////////////////////////////////////////////////////////////

#include "equistack.h"

// Takes in the size of the structs the stack holds and the
// maximum number of elements the stack will hold
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

// Returns a pointer to the nth most recent element
void* equistack_Get(equistack* S, int16_t n)
{
	xSemaphoreTake(S->mutex, (TickType_t) MUTEX_WAIT_TIME_TICKS);

	// We want to ignore the bottom index which is currently staged and maybe
	// being overwritten
	// We also don't want to be able to get the oldest element if the stack
	// is full because that will be the one that's being overwritten
	if (n < S->cur_size)
	{
		xSemaphoreGive(S->mutex);
		
		// TODO: Remove this
		int get_index = (S->top_index - n) % S->max_size;
		if (get_index < 0) 
		{
			get_index = get_index += S->max_size;
		}
		 
		return S->data[get_index];
	}
	else
	{
		xSemaphoreGive(S->mutex);
		return NULL;
	}
}

// Only meant to be used when the stack is first being created
void* equistack_Initial_Stage(equistack* S)
{
	return S->data[0];
}

// Returns the next pointer and "finalizes" the previous one (staging)
// Overwrites the bottom value if need be
void* equistack_Stage(equistack* S)
{
	xSemaphoreTake(S->mutex, (TickType_t) 10);
	
	S->top_index = (S->top_index + 1) % S->max_size;
	
	if (S->bottom_index == (S->top_index + 1) % S->max_size)
	{
		S->bottom_index = (S->bottom_index + 1) % S->max_size;
	}
	else
	{
		// It doesn't make sense for the current size to be the max size because
		// at least one value is always being rewritten
		if (S->cur_size < S->max_size - 1) 
		{
			S->cur_size++;
		}
		
		if (S->bottom_index == -1)
		{
			S->bottom_index = 0;
		}
	}
	
	// TODO: Remove this
	int staged_index = (S->top_index + 1) % S->max_size;
	void* staged_pointer = S->data[(S->top_index + 1) % S->max_size];
	clear_existing_data(staged_pointer, S->data_size);
	
	xSemaphoreGive(S->mutex);
	return staged_pointer; // return pointer to staged data
}

void clear_existing_data(void* ptr, size_t slot_size)
{
	// convert the pointer to a char pointer to iterate over bytes
	char* byte_ptr = (char*) ptr;
	for (int16_t i = 0; i < slot_size; i++)
	{
		// set memory value to zero
		*(byte_ptr + i) = 0;
	}
}
