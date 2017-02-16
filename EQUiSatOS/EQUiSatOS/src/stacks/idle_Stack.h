/*
 * idle_Stack.h
 *
 * Created: 10/18/2016 9:46:57 PM
 *  Author: rj16
 */ 

#ifndef IDLE_STACK_H_
#define IDLE_STACK_H_

#define IDLE_STACK_MAX 20
#include "equistacks.h"
#include "State_Structs.h"

typedef struct idle_Stack
{
	idle_data_t*     data[IDLE_STACK_MAX];
	int16_t     top_index;
	int16_t     bottom_index;
	int16_t     size;	
	SemaphoreHandle_t mutex;
} idle_Stack;

idle_Stack* idle_Stack_Init();
idle_data_t* idle_Stack_Get(idle_Stack* S, int16_t n);
idle_data_t* idle_Stack_Initial_Stage(idle_Stack* S);
idle_data_t* idle_Stack_Stage(idle_Stack* S);

#endif