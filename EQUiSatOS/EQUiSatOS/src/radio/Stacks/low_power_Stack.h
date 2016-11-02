/*
 * low_power_Stack.h
 *
 * Created: 10/18/2016 9:46:57 PM
 *  Author: rj16
 */ 

// TODO: "#ifdef... #endif"?

#ifndef LOW_POWER_STACK_H_
#define LOW_POWER_STACK_H_

#define LOW_POWER_STACK_MAX 4
#include <asf.h>
#include "State_Structs.h"

typedef struct low_power_Stack
{
	low_power_data_t*     data[LOW_POWER_STACK_MAX];
	int16_t     size;
	int16_t     top_index;
	int16_t     bottom_index;	
} low_power_Stack;

void low_power_Stack_Init(low_power_Stack*);
low_power_data_t* low_power_Stack_Top(low_power_Stack*);
void low_power_Stack_Push(low_power_Stack*, low_power_data_t*);

#endif