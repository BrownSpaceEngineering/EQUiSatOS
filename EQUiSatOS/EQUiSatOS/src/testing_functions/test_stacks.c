/*
 * test_stacks.c
 *
 * Created: 3/1/2017 9:27:37 PM
 *  Author: rj16
 */ 

#include "test_stacks.h"

void standard_case()
{
	equistack* test = equistack_Init(sizeof(test_data), 4);
	
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
	
	staged = equistack_Stage(test);
	fill_test(staged, 3);
	assert(test->top_index == 1);
	assert(test->bottom_index == 0);
	assert(test->cur_size == 2);
	assert(equistack_Get(test, 0) != NULL);
	assert((((test_data*)equistack_Get(test, 0))->one_data[0]).values[0] == 2);
	assert(equistack_Get(test, 1) != NULL);
	assert((((test_data*)equistack_Get(test, 1))->one_data[0]).values[0] == 1);
	assert(equistack_Get(test, 2) == NULL);

	staged = equistack_Stage(test);
	fill_test(staged, 4);
	assert(test->top_index == 2);
	assert(test->bottom_index == 0);
	assert(test->cur_size == 3);
	assert(equistack_Get(test, 0) != NULL);
	assert((((test_data*)equistack_Get(test, 0))->one_data[0]).values[0] == 3);
	assert(equistack_Get(test, 1) != NULL);
	assert((((test_data*)equistack_Get(test, 1))->one_data[0]).values[0] == 2);
	assert(equistack_Get(test, 2) != NULL);
	assert((((test_data*)equistack_Get(test, 2))->one_data[0]).values[0] == 1);
	assert(equistack_Get(test, 3) == NULL);
	
	staged = equistack_Stage(test);
	fill_test(staged, 5);
	assert(test->top_index == 3);
	assert(test->bottom_index == 1);
	assert(test->cur_size == 3);
	assert(equistack_Get(test, 0) != NULL);
	assert((((test_data*)equistack_Get(test, 0))->one_data[0]).values[0] == 4);
	assert(equistack_Get(test, 1) != NULL);
	assert((((test_data*)equistack_Get(test, 1))->one_data[0]).values[0] == 3);
	assert(equistack_Get(test, 2) != NULL);
	assert((((test_data*)equistack_Get(test, 2))->one_data[0]).values[0] == 2);
	assert(equistack_Get(test, 3) == NULL);
	
	staged = equistack_Stage(test);
	fill_test(staged, 6);
	assert(test->top_index == 0);
	assert(test->bottom_index == 2);
	assert(test->cur_size == 3);
	assert(equistack_Get(test, 0) != NULL);
	assert((((test_data*)equistack_Get(test, 0))->one_data[0]).values[0] == 5);
	assert(equistack_Get(test, 1) != NULL);
	assert((((test_data*)equistack_Get(test, 1))->one_data[0]).values[0] == 4);
	assert(equistack_Get(test, 2) != NULL);
	assert((((test_data*)equistack_Get(test, 2))->one_data[0]).values[0] == 3);
	assert(equistack_Get(test, 3) == NULL);
	
	staged = equistack_Stage(test);
	fill_test(staged, 7);
	assert(test->top_index == 1);
	assert(test->bottom_index == 3);
	assert(test->cur_size == 3);
	assert(equistack_Get(test, 0) != NULL);
	assert((((test_data*)equistack_Get(test, 0))->one_data[0]).values[0] == 6);
	assert(equistack_Get(test, 1) != NULL);
	assert((((test_data*)equistack_Get(test, 1))->one_data[0]).values[0] == 5);
	assert(equistack_Get(test, 2) != NULL);
	assert((((test_data*)equistack_Get(test, 2))->one_data[0]).values[0] == 4);
	assert(equistack_Get(test, 3) == NULL);
	
	for (int i = 0; i < 1000; i++) {
		staged = equistack_Stage(test);
		fill_test(staged, i + 8);
		assert(equistack_Get(test, 0) != NULL);
		assert((((test_data*)equistack_Get(test, 0))->one_data[0]).values[0] == i + 7);
		assert(equistack_Get(test, 1) != NULL);
		assert((((test_data*)equistack_Get(test, 1))->one_data[0]).values[0] == i + 6);
		assert(equistack_Get(test, 2) != NULL);
		assert((((test_data*)equistack_Get(test, 2))->one_data[0]).values[0] == i + 5);
		assert(equistack_Get(test, 3) == NULL);
	}
	
	return;
}

void big_case()
{	
	equistack* bigTest = equistack_Init(sizeof(test_data), 20);
	
	assert(bigTest->cur_size == 0);
	assert(bigTest->top_index == -1);
	assert(bigTest->bottom_index == -1);
	assert(equistack_Get(bigTest, 0) == NULL);
	
	test_data* staged = equistack_Initial_Stage(bigTest);
	fill_test(staged, 1);
	assert(bigTest->top_index == -1);
	assert(bigTest->bottom_index == -1);
	assert(bigTest->cur_size == 0);
	assert(equistack_Get(bigTest, 0) == NULL);
	
	staged = equistack_Stage(bigTest);
	fill_test(staged, 2);
	assert(bigTest->top_index == 0);
	assert(bigTest->bottom_index == 0);
	assert(bigTest->cur_size == 1);
	assert(equistack_Get(bigTest, 0) != NULL);
	assert((((test_data*)equistack_Get(bigTest, 0))->one_data[0]).values[0] == 1);
	assert(equistack_Get(bigTest, 1) == NULL);
	
	staged = equistack_Stage(bigTest);
	fill_test(staged, 3);
	assert(bigTest->top_index == 1);
	assert(bigTest->bottom_index == 0);
	assert(bigTest->cur_size == 2);
	assert(equistack_Get(bigTest, 0) != NULL);
	assert((((test_data*)equistack_Get(bigTest, 0))->one_data[0]).values[0] == 2);
	assert(equistack_Get(bigTest, 1) != NULL);
	assert((((test_data*)equistack_Get(bigTest, 1))->one_data[0]).values[0] == 1);
	assert(equistack_Get(bigTest, 2) == NULL);

	staged = equistack_Stage(bigTest);
	fill_test(staged, 4);
	assert(bigTest->top_index == 2);
	assert(bigTest->bottom_index == 0);
	assert(bigTest->cur_size == 3);
	assert(equistack_Get(bigTest, 0) != NULL);
	assert((((test_data*)equistack_Get(bigTest, 0))->one_data[0]).values[0] == 3);
	assert(equistack_Get(bigTest, 1) != NULL);
	assert((((test_data*)equistack_Get(bigTest, 1))->one_data[0]).values[0] == 2);
	assert(equistack_Get(bigTest, 2) != NULL);
	assert((((test_data*)equistack_Get(bigTest, 2))->one_data[0]).values[0] == 1);
	assert(equistack_Get(bigTest, 3) == NULL);
	
	staged = equistack_Stage(bigTest);
	fill_test(staged, 5);
	assert(bigTest->top_index == 3);
	assert(bigTest->bottom_index == 0);
	assert(bigTest->cur_size == 4);
	assert(equistack_Get(bigTest, 0) != NULL);
	assert((((test_data*)equistack_Get(bigTest, 0))->one_data[0]).values[0] == 4);
	assert(equistack_Get(bigTest, 1) != NULL);
	assert((((test_data*)equistack_Get(bigTest, 1))->one_data[0]).values[0] == 3);
	assert(equistack_Get(bigTest, 2) != NULL);
	assert((((test_data*)equistack_Get(bigTest, 2))->one_data[0]).values[0] == 2);
	
	staged = equistack_Stage(bigTest);
	fill_test(staged, 6);
	assert(bigTest->top_index == 4);
	assert(bigTest->bottom_index == 0);
	assert(bigTest->cur_size == 5);
	assert(equistack_Get(bigTest, 0) != NULL);
	assert((((test_data*)equistack_Get(bigTest, 0))->one_data[0]).values[0] == 5);
	assert(equistack_Get(bigTest, 1) != NULL);
	assert((((test_data*)equistack_Get(bigTest, 1))->one_data[0]).values[0] == 4);
	assert(equistack_Get(bigTest, 2) != NULL);
	assert((((test_data*)equistack_Get(bigTest, 2))->one_data[0]).values[0] == 3);
	
	staged = equistack_Stage(bigTest);
	fill_test(staged, 7);
	assert(bigTest->top_index == 5);
	assert(bigTest->bottom_index == 0);
	assert(bigTest->cur_size == 6);
	assert(equistack_Get(bigTest, 0) != NULL);
	assert((((test_data*)equistack_Get(bigTest, 0))->one_data[0]).values[0] == 6);
	assert(equistack_Get(bigTest, 1) != NULL);
	assert((((test_data*)equistack_Get(bigTest, 1))->one_data[0]).values[0] == 5);
	assert(equistack_Get(bigTest, 2) != NULL);
	assert((((test_data*)equistack_Get(bigTest, 2))->one_data[0]).values[0] == 4);
	
	for (int i = 0; i < 1000; i++) {
		staged = equistack_Stage(bigTest);
		fill_test(staged, i + 8);
		assert(equistack_Get(bigTest, 0) != NULL);
		assert((((test_data*)equistack_Get(bigTest, 0))->one_data[0]).values[0] == i + 7);
		assert(equistack_Get(bigTest, 1) != NULL);
		assert((((test_data*)equistack_Get(bigTest, 1))->one_data[0]).values[0] == i + 6);
		assert(equistack_Get(bigTest, 2) != NULL);
		assert((((test_data*)equistack_Get(bigTest, 2))->one_data[0]).values[0] == i + 5);
	}
	
	assert(equistack_Get(bigTest, 20) == NULL);
	assert(equistack_Get(bigTest, 19) == NULL);
	assert(equistack_Get(bigTest, 18) != NULL);	
	assert(equistack_Get(bigTest, 17) != NULL);
	assert(equistack_Get(bigTest, 16) != NULL);
	assert(equistack_Get(bigTest, 15) != NULL);
	assert(equistack_Get(bigTest, 14) != NULL);
	return;
}

void test_equistack() 
{
	standard_case();
	big_case();
	return;
}