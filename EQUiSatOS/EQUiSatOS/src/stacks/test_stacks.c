/*
 * test_stacks.c
 *
 * Created: 3/1/2017 9:27:37 PM
 *  Author: rj16
 */ 

#include "test_stacks.h"

void standard_case()
{
	equistack* test;
	test_data* _data_arr[4];
	equistack_Init(test, _data_arr, sizeof(test_data), 4);
	
	assert(test->cur_size == 0);
	assert(test->top_index == -1);
	assert(test->bottom_index == -1);
	assert(equistack_Get(test, 0) == NULL);
	
	test_data* staged = equistack_Initial_Stage(test);
	fill_test(staged, 1);
	assert(test->top_index == -1);
	assert(test->bottom_index == -1);
	assert(test->cur_size == 0);
	assert(equistack_Get(test, 0) == NULL);
	
	staged = equistack_Stage(test);
	fill_test(staged, 2);
	assert(test->top_index == 0);
	assert(test->bottom_index == 0);
	assert(test->cur_size == 1);
	assert(equistack_Get(test, 0) != NULL);
	assert((((test_data*)equistack_Get(test, 0))->one_data[0]).values[0] == 1);
	assert(equistack_Get(test, 1) == NULL);
	
	return;
}

void test_equistack() 
{
	standard_case();
}