// Basic implementation from 
// http://groups.csail.mit.edu/graphics/classes/6.837/F04/cpp_notes/stack1.html
#include "Num_Stack.h"


void Stack_Init(Num_Stack* S)
{
	S->size = 0;
	S->top_index = -1;
	S->bottom_index = -1;
}

int16_t Stack_Top(Num_Stack* S)
{
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
		return -1;
	}
}

int16_t Stack_Bottom(Num_Stack* S)
{
	// Could also be S->size == 0
	// TODO: Think about whether we need second conditional
	if (S->bottom_index != -1 && S->bottom_index < S->size)
	{
		return S->data[S->bottom_index];
	}
	else
	{
		// Return dummy
		return -1;
	}
}

// Overwrites the bottom value if need be
void Stack_Add_Front(Num_Stack* S, int16_t val)
{
	S->top_index = (S->top_index + 1) % NUM_STACK_MAX;
	
	// something was overwritten
	// could also be
	// if (size == max_size)
	if (S->bottom_index == S->top_index)
	{
		S->bottom_index = (S->bottom_index + 1) % NUM_STACK_MAX;
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
}

void Stack_Remove_Front(Num_Stack* S)
{
	if (S->size > 0)
	{
		S->size--;
		
		if (S->size == 0)
		{
			S->bottom_index = -1;
			S->top_index = -1;
		}
		else
		{
			S->top_index = S->top_index - 1;
			
			if (S->top_index < 0)
			{
				S->top_index = S->top_index + NUM_STACK_MAX;
			}
		}
	}
}

void Stack_Remove_Back(Num_Stack* S)
{
	if (S->size > 0)
	{
		S->size--;
		
		if (S->size == 0)
		{
			S->bottom_index = -1;
			S->top_index = -1;
		}
		else
		{	
			S->bottom_index = (S->bottom_index + 1) % NUM_STACK_MAX;
		}
	}
}

// TODO: This may change in the future
// Iterate only for a couple elements?

void Stack_Iterate(Num_Stack* S, int16_t n, int16_t Return_Array)
{
	// if (n < S->size)
	// {
	//	int result[S->size];
	//	for (int i = S->top_index; i > ; i--)
	//	{
	//		result[i] = S->data[i % NUM_STACK_MAX];
	//	}
		
	//	return result;
	// }
	// TODO: Do we want dummies here?
	// else
	// {
		// return 0;	
	// }
	if(n < S->size)
	{
		Num_Stack copy_S;
		Stack_Init(&copy_S);
		copy_S.bottom_index = S->bottom_index;
		copy_S.top_index = S->top_index;
		copy_S.size = S->size;
		for(int i = 0; i < S->size; i++)
		{
			copy_S.data[i] = S->data[i];
		}
		int16_t result[n];
		int16_t my_ind = copy_S.top_index;
		for(int i = 0; i < n; i++)
		{
			result[i] = Stack_Top(&copy_S);
			Stack_Remove_Front(&copy_S);
		}
		Return_Array = result;
	}
	else
	{
		// TODO: Return like anything but null?
		Return_Array = NULL;
	}
}
