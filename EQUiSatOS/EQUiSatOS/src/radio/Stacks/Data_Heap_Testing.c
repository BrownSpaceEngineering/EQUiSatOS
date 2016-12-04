/*
 * Data_Heap_Testing.c
 *
 * Created: 11/29/2016 8:16:59 PM
 *  Author: mcken
 */ 

#include "Data_Heap_Testing.h"

void data_heap_tests(void)
{
	typedef struct basic_struct {
		char* name;
		int val1;
		int vals[5];
	} basic_struct;
	
	data_t_heap* heap1 = create_data_t_heap(sizeof(struct basic_struct), 100);

	// Malloc testing
	// (check that data exists correctly everywhere)
	basic_struct** data_ptrs;
	
	data_ptrs[0] = data_t_malloc(heap1);
	data_ptrs[0]->name = "Number one";
	data_ptrs[0]->val1 = 12;
	init5(data_ptrs[0]->vals, 1, 2, 3, 4, 5);
	
	data_ptrs[1] = data_t_malloc(heap1);
	data_ptrs[1]->name = "Number two";
	data_ptrs[1]->val1 = 5;
	init5(data_ptrs[1]->vals, 5, 4, 3, 2, 1);
		
	data_ptrs[2] = data_t_malloc(heap1);
	data_ptrs[2]->name = "Number three";
	data_ptrs[2]->val1 = 102;
	init5(data_ptrs[2]->vals, 1, 1, 1, 1, 1);
	
	for (int i = 3; i < 100; i++)
	{
		data_ptrs[i] = data_t_malloc(heap1);
		data_ptrs[i]->name = "Some other struct";
		data_ptrs[i]->val1 = i;
		init5(data_ptrs[i]->vals, i + 1, 1, 1, i - 1, 1);
	}
	
	// Changing testing
	// (check that ONLY these values changed in ONLY that location)
	data_ptrs[7]->name = "A special struct";
	data_ptrs[14]->val1 = 42;
	init5(data_ptrs[95]->vals, 0, 0, 1, 0, 0);
	
	data_ptrs[99]->name = "The final frontier";
				
	// No-memory testing
	basic_struct* should_be_null = data_t_malloc(heap1);
	
	// Freeing testing
	// (check that data at each place is wiped)
	data_t_free(data_ptrs[0], heap1);
	data_t_free(data_ptrs[50], heap1);
	data_t_free(data_ptrs[99], heap1);
	
	// Re-malloc testing
	// (make sure memory addresses are correct and that no invalid errors occur)
	data_ptrs[0] = data_t_malloc(heap1); // check that data is still wiped here, and that the actual memory address is the first
	data_ptrs[0]->name = "Brave new struct";
	data_ptrs[0]->val1 = 1002;
	init5(data_ptrs[0]->vals ,5, 6, 7, 8, 9);
		
	data_ptrs[50] = data_t_malloc(heap1);
	data_ptrs[99] = data_t_malloc(heap1);
	
	// Going ham testing
	data_t_free(data_ptrs[0], heap1);
	data_ptrs[0] = data_t_malloc(heap1);
	basic_struct* should_be_null2 = data_t_malloc(heap1);
	basic_struct* should_be_null3 = data_ptrs[0] = data_t_malloc(heap1);
	basic_struct* should_be_null4 = data_ptrs[0] = data_t_malloc(heap1);
	
	data_t_free(data_ptrs[60], heap1);	
	data_t_free(data_ptrs[2], heap1);
	data_t_free(data_ptrs[90], heap1);
	data_t_free(data_ptrs[1], heap1);
	data_ptrs[1] = data_t_malloc(heap1); // Should have memory address corresponding to 1
	data_t_free(data_ptrs[98], heap1);
	data_ptrs[20] = data_t_malloc(heap1); // Should have memory address corresponding to 20
	data_t_free(data_ptrs[99], heap1);
	
	// Complete free test
	for (int i = 0; i < 100; i++)
	{
		data_t_free(data_ptrs[i], heap1);
	}
	
	free_data_t_heap(heap1);
}

void init5(int arr[], int v0, int v1, int v2, int v3, int v4)
{
	arr[0] = v0;
	arr[1] = v1;
	arr[2] = v2;
	arr[3] = v3;
	arr[4] = v4;
}