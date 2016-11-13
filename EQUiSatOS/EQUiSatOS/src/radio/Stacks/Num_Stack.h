/*
 * Num_Stack.h
 *
 * Created: 11/8/2016 6:07:26 PM
 *  Author: rj16
 */ 


#ifndef NUM_STACK_H_
#define NUM_STACK_H_

#define MUTEX_WAIT_TIME_TICKS 10
#define IDLE_STACK_MAX 4
#include <asf.h>
#include "State_Structs.h"

typedef struct Num_Stack
{
	int16_t           data[IDLE_STACK_MAX];
	int16_t           size;
	int16_t           top_index;
	int16_t           bottom_index;
	SemaphoreHandle_t mutex;
} Num_Stack;

void Num_Stack_Init(Num_Stack*);
int16_t Num_Stack_Top(Num_Stack*);
void Num_Stack_Push(Num_Stack*, int16_t);

#endif /* NUM_STACK_H_ */