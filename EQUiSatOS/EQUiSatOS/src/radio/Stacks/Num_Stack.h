/*
 * Num_Stack.h
 *
 * Created: 10/18/2016 9:46:57 PM
 *  Author: rj16
 */ 

// TODO: "#ifdef... #endif"?

#define NUM_STACK_MAX 4

typedef struct Num_Stack
{
	int         data[NUM_STACK_MAX];
	int16_t     size;
	int16_t     top_index;
	int16_t     bottom_index;	
} Num_Stack;

void Stack_Init();
int Stack_Top();
int Stack_Bottom();
void Stack_Add_Front();
void Stack_Remove_Front();
void Stack_Remove_Back();
int * Iterate();
