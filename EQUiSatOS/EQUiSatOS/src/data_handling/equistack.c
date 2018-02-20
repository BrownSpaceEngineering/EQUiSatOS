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
#include "errors.h"

/**
 * Takes in a pointer to an equistack to construct and a pointer to a data array to use, the size
 * of the structs the stack holds, the maximum number of elements the stack will hold, and a pointer
 * to the mutex.
 */
equistack* equistack_Init(equistack* S, void* data, size_t data_size, uint16_t max_size, SemaphoreHandle_t mutex)
{
	S->top_index = -1;
	S->bottom_index = -1;
	S->cur_size = 0;
	S->max_size = max_size;
	S->data_size = data_size;
	S->mutex = mutex;
	S->data = data;
	return S;
}

// Returns a pointer to the nth most recent element (relative to the "top" of the equistack)
void* equistack_Get(equistack* S, int16_t n)
{
	bool got_mutex = true;
	if (!xSemaphoreTake(S->mutex, (TickType_t) EQUISTACK_MUTEX_WAIT_TIME_TICKS)) {
		// log error, but continue on because we're just reading
		log_error(ELOC_EQUISTACK_GET, ECODE_EQUISTACK_MUTEX_TIMEOUT, true);
		got_mutex = false;
	}

	// We want to ignore the bottom index which is currently staged and maybe
	// being overwritten
	// We also don't want to be able to get the oldest element if the stack
	// is full because that will be the one that's being overwritten
	if (n < S->cur_size)
	{
		int get_index = (S->top_index - n) % S->max_size;
		if (get_index < 0)
		{
			get_index += S->max_size;
		}

		if (got_mutex) xSemaphoreGive(S->mutex);
		return S->data + S->data_size * get_index;
	}
	else
	{
		if (got_mutex) xSemaphoreGive(S->mutex);
		return NULL;
	}
}

// Helper function to get elements relative to the bottom of the equistack 
// (note the bottommost element (#0) will most likely to be overwritten soon)
// Returns the nth element from the current bottom of the stack.
// (i.e., if cur_size = 6, equistack_Get(S, 1) == equistack_Get_From_Bottom(S, 4))
void* equistack_Get_From_Bottom(equistack* S, int16_t n) {
	return equistack_Get(S, S->cur_size - 1 - n);
}

// Only meant to be used when the stack is first being created
void* equistack_Initial_Stage(equistack* S)
{
	return S->data;
}

// Returns the next pointer and "finalizes" the previous one (staging)
// Overwrites the bottom value if need be
// NOTE: if fails to obtain mutex (timeout), returns previous pointer
// (does not add onto stack)
void* equistack_Stage(equistack* S)
{
	void *staged_pointer = NULL;
	if (xSemaphoreTake(S->mutex, (TickType_t) EQUISTACK_MUTEX_WAIT_TIME_TICKS))
	{
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

		staged_pointer = S->data + S->data_size*((S->top_index + 1) % S->max_size);
		clear_existing_data(staged_pointer, S->data_size);
		xSemaphoreGive(S->mutex);
	} else {
		// log error if mutex can't be obtained, and give pointer to previously staged 
		// struct (the one currently at the top index; 
		// note that the above staged_pointer is using an incremented top_index)
		// TODO: check
		log_error(ELOC_EQUISTACK_PUT, ECODE_EQUISTACK_MUTEX_TIMEOUT, true);
		staged_pointer = S->data + (S->data_size)*((S->top_index + 1) % S->max_size);
	}
	return staged_pointer; // return pointer to staged data
}

// Has the same effect as equistack_Stage, but writes the provided
// data (assumed to be the size of S->data_size) to the staged index
// before incrementing the stack top. (for use with stacks using primitives)
// (returns next pointer so can be used with equistack_Stage)
void* equistack_Push(equistack* S, void* data) {
	void* staged_pointer = S->data; // if this is an initial stage, simply copy to start
	if (S->top_index >= 0) {
		// otherwise, grab the pointer to the staging area (data just past the top index)
		staged_pointer = S->data + S->data_size*((S->top_index + 1) % S->max_size);
	}
	memcpy(staged_pointer, data, S->data_size); // copy data to that staging area
	return equistack_Stage(S); // confirm ("finalize") the data at the staging area
}

void clear_existing_data(void* ptr, size_t slot_size)
{
	// convert the pointer to a char pointer to iterate over bytes
	char* byte_ptr = (char*) ptr;
	for (uint16_t i = 0; i < slot_size; i++)
	{
		// set memory value to zero
		*(byte_ptr + i) = 0;
	}
}

/* clears equistack completely - DON'T USE BESIDES FOR TESTING */
void __equistack_Clear(equistack* S) {
	S->top_index = -1;
	S->bottom_index = -1;
	S->cur_size = 0;
}
