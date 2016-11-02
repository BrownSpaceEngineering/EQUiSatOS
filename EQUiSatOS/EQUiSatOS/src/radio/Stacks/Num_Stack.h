/*
 * Num_Stack.h
 *
 * Created: 10/18/2016 9:46:57 PM
 *  Author: rj16
 */ 

// TODO: "#ifdef... #endif"?
#ifndef _NUM_STACK_H_
#define _NUM_STACK_H_

#define NUM_STACK_MAX 4
#include <asf.h>

typedef struct Num_Stack
{
	int16_t     data[NUM_STACK_MAX];
	int16_t     size;
	int16_t     top_index;
	int16_t     bottom_index;	
} Num_Stack;

<<<<<<< HEAD
void Stack_Init();
int Stack_Top();
int Stack_Bottom();
void Stack_Add_Front();
void Stack_Remove_Front();
void Stack_Remove_Back();
int * Iterate();

#endif
=======
void Stack_Init(Num_Stack*);
int16_t Stack_Top(Num_Stack*);
int16_t Stack_Bottom(Num_Stack*);
void Stack_Remove_Front(Num_Stack*);
void Stack_Remove_Back(Num_Stack*);
void Stack_Iterate(Num_Stack*, int16_t, int16_t);
void Stack_Add_Front(Num_Stack*, int16_t);
>>>>>>> origin/rtos
