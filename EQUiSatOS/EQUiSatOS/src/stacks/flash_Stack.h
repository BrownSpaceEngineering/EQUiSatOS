/*
 * flash_Stack.h
 *
 * Created: 10/18/2016 9:46:57 PM
 *  Author: rj16
 */ 

#ifndef FLASH_STACK_H_
#define FLASH_STACK_H_

#define FLASH_STACK_MAX 4
#include "State_Structs.h"
#include "equistacks.h"

typedef struct flash_Stack
{
	flash_data_t*     data[FLASH_STACK_MAX];
	int16_t     top_index;
	int16_t     bottom_index;
	int16_t     size;	
	SemaphoreHandle_t mutex;
} flash_Stack;

flash_Stack* flash_Stack_Init(void);
flash_data_t* flash_Stack_Get(flash_Stack* S, int16_t n);
flash_data_t* flash_Stack_Stage(flash_Stack* S);

#endif