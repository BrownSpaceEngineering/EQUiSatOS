/*
 * Num_Stack.c
 *
 * Created: 11/8/2016 6:07:08 PM
 *  Author: rj16
 */ 
#include "Num_Stack.h"

void Num_Stack_Init(Num_Stack* S)
{
	S->size = 0;
	S->top_index = -1;
	S->bottom_index = -1;
	S->mutex = xSemaphoreCreateMutex();
}

int16_t Num_Stack_Top(Num_Stack* S)
{
	// TODO:  More precise mutexing
	xSemaphoreTake(S->mutex, (TickType_t) MUTEX_WAIT_TIME_TICKS);
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
	
	xSemaphoreGive(S->mutex);
}

// Overwrites the bottom value if need be
void Num_Stack_Test_Push(Num_Stack* S, int16_t val)
{
	// TODO: Change if needed
	xSemaphoreTake(S->mutex, (TickType_t) 10);
	S->top_index = (S->top_index + 1) % IDLE_STACK_MAX;
	
	// something was overwritten
	// could also be
	// if (size == max_size)
	if (S->bottom_index == S->top_index)
	{
		vPortFree(&(S->data[S->top_index]));
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
	xSemaphoreGive(S->mutex);
}