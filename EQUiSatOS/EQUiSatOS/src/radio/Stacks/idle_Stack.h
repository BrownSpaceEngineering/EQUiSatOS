/*
 * idle_Stack.h
 *
 * Created: 10/18/2016 9:46:57 PM
 *  Author: rj16
 */ 

// TODO: "#ifdef... #endif"?

#ifndef IDLE_STACK_H_
#define IDLE_STACK_H_

#define IDLE_STACK_MAX 4
#include <asf.h>
#include "State_Structs.h"

typedef struct idle_Stack
{
	idle_data_t*     data[IDLE_STACK_MAX];
	int16_t     size;
	int16_t     top_index;
	int16_t     bottom_index;	
	SemaphoreHandle_t mutex;
} idle_Stack;

void idle_Stack_Init(idle_Stack*);
idle_data_t* idle_Stack_Top(idle_Stack*);
void idle_Stack_Push(idle_Stack*, idle_data_t*);

#endif