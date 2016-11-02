/*
 * flash_Stack.h
 *
 * Created: 10/18/2016 9:46:57 PM
 *  Author: rj16
 */ 

// TODO: "#ifdef... #endif"?

#ifndef FLASH_STACK_H_
#define FLASH_STACK_H_

#define FLASH_STACK_MAX 4
#include <asf.h>
#include "State_Structs.h"

typedef struct flash_Stack
{
	flash_data_t*     data[FLASH_STACK_MAX];
	int16_t     size;
	int16_t     top_index;
	int16_t     bottom_index;	
} flash_Stack;

void flash_Stack_Init(flash_Stack*);
flash_data_t* flash_Stack_Top(flash_Stack*);
void flash_Stack_Push(flash_Stack*, flash_data_t*);

#endif