/*
 * boot_Stack.h
 *
 * Created: 10/18/2016 9:46:57 PM
 *  Author: rj16
 */ 

// TODO: "#ifdef... #endif"?

#ifndef BOOT_STACK_H_
#define BOOT_STACK_H_

#define BOOT_STACK_MAX 4
#include <asf.h>
#include "State_Structs.h"

typedef struct boot_Stack
{
	boot_data_t*     data[BOOT_STACK_MAX];
	int16_t     size;
	int16_t     top_index;
	int16_t     bottom_index;	
} boot_Stack;

void boot_Stack_Init(boot_Stack*);
boot_data_t* boot_Stack_Top(boot_Stack*);
void boot_Stack_Push(boot_Stack*, boot_data_t*);

#endif