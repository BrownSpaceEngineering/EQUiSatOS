/*
 * test_stacks.c
 *
 * Created: 3/1/2017 9:27:37 PM
 *  Author: rj16
 */ 

#include "test_stacks.h"

static void basic_push_usage(void) {
	equistack basic_test_actual;
	equistack *test = &basic_test_actual;
	StaticSemaphore_t inner_mutex;
	SemaphoreHandle_t mutex = xSemaphoreCreateMutexStatic(&inner_mutex);
	int _data_arr[4];
	equistack_Init(test, &_data_arr, sizeof(int), 4, mutex);
	
	assert(test->top_index == -1);
	assert(test->bottom_index == -1);
	assert(test->cur_size == 0);
	assert(equistack_Get(test, 0) == NULL);
	assert(equistack_Get(test, 1) == NULL);
	assert(equistack_Get(test, 2) == NULL);
	
	int data0 = 12;
	equistack_Push(test, &data0);
	
	assert(test->top_index == 0);
	assert(test->bottom_index == 0);
	assert(test->cur_size == 1);
	assert(		 equistack_Get(test, 0) != NULL);
	assert(*((int*) equistack_Get(test, 0)) == 12);
	assert(equistack_Get(test, 1) == NULL);
	assert(equistack_Get(test, 2) == NULL);
	
	int data1 = -10;
	equistack_Push(test, &data1);
	
	assert(test->top_index == 1);
	assert(test->bottom_index == 0);
	assert(test->cur_size == 2);
	assert(		 equistack_Get(test, 0) != NULL);
	assert(*((int*) equistack_Get(test, 0)) == -10);
	assert(		 equistack_Get(test, 1) != NULL);
	assert(*((int*) equistack_Get(test, 1)) == 12);
	assert(equistack_Get(test, 2) == NULL);
	
	int data2 = 143;
	equistack_Push(test, &data2);
	
	assert(test->top_index == 2);
	assert(test->bottom_index == 0);
	assert(test->cur_size == 3);
	assert(		 equistack_Get(test, 0) != NULL);
	assert(*((int*) equistack_Get(test, 0)) == 143);
	assert(		 equistack_Get(test, 1) != NULL);
	assert(*((int*) equistack_Get(test, 1)) == -10);
	assert(		 equistack_Get(test, 2) != NULL);
	assert(*((int*) equistack_Get(test, 2)) == 12);
	
	int data0new = -1231;
	equistack_Push(test, &data0new);
	
	assert(test->top_index == 3);
	assert(test->bottom_index == 1);
	assert(test->cur_size == 3);
	assert(		 equistack_Get(test, 0) != NULL);
	assert(*((int*) equistack_Get(test, 0)) == -1231);
	assert(		 equistack_Get(test, 1) != NULL);
	assert(*((int*) equistack_Get(test, 1)) == 143);
	assert(		 equistack_Get(test, 2) != NULL);
	assert(*((int*) equistack_Get(test, 2)) == -10);
	
	int data1new = 15;
	equistack_Push(test, &data1new);
	
	assert(test->top_index == 0);
	assert(test->bottom_index == 2);
	assert(test->cur_size == 3);
	assert(		 equistack_Get(test, 0) != NULL);
	assert(*((int*) equistack_Get(test, 0)) == 15);
	assert(		 equistack_Get(test, 1) != NULL);
	assert(*((int*) equistack_Get(test, 1)) == -1231);
	assert(		 equistack_Get(test, 2) != NULL);
	assert(*((int*) equistack_Get(test, 2)) == 143);
	
	int data2new = 16;
	equistack_Push(test, &data2new);
	
	assert(test->top_index == 1);
	assert(test->bottom_index == 3);
	assert(test->cur_size == 3);
	assert(		 equistack_Get(test, 0) != NULL);
	assert(*((int*) equistack_Get(test, 0)) == 16);
	assert(		 equistack_Get(test, 1) != NULL);
	assert(*((int*) equistack_Get(test, 1)) == 15);
	assert(		 equistack_Get(test, 2) != NULL);
	assert(*((int*) equistack_Get(test, 2)) == -1231);
	
	int zero = 0;
	int one = 1;
	equistack_Push(test, &zero);
	equistack_Push(test, &one);
	for (int i = 2; i < 1000; i++) {
		equistack_Push(test, &i);
		assert(		 equistack_Get(test, 0) != NULL);
		assert(*((int*) equistack_Get(test, 0)) == i);
		assert(		 equistack_Get(test, 1) != NULL);
		assert(*((int*) equistack_Get(test, 1)) == i - 1);
		assert(		 equistack_Get(test, 2) != NULL);
		assert(*((int*) equistack_Get(test, 2)) == i - 2);
	}
}

static void standard_case(void)
{
	equistack test_actual;
	equistack* test = &test_actual;
	StaticSemaphore_t inner_mutex;
	SemaphoreHandle_t mutex = xSemaphoreCreateMutexStatic(&inner_mutex);
	test_data _data_arr[4];
	equistack_Init(test, &_data_arr, sizeof(test_data), 4, mutex);
	
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

static void big_case(void)
{	
	int n = 1000;
	
	equistack bigTestActual;
	equistack* bigTest = &bigTestActual;
	test_data _data_arr[20];
	StaticSemaphore_t inner_mutex;
	SemaphoreHandle_t mutex = xSemaphoreCreateMutexStatic(&inner_mutex);
	equistack_Init(bigTest, &_data_arr, sizeof(test_data), 20, mutex);
	
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
	
	for (int i = 0; i < n; i++) {
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

static void multi_add(void)
{
	int n = 30000;
	
	equistack bigTestActual;
	equistack* bigTest = &bigTestActual;
	test_data _data_arr[20];
	StaticSemaphore_t inner_mutex;
	SemaphoreHandle_t mutex = xSemaphoreCreateMutexStatic(&inner_mutex);
	equistack_Init(bigTest, &_data_arr, sizeof(test_data), 20, mutex);
	
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
	
	for (int i = 0; i < n; i++) {
		staged = equistack_Stage(bigTest);
		fill_test(staged, i + 8);
		assert(equistack_Get(bigTest, 0) != NULL);
		assert((((test_data*)equistack_Get(bigTest, 0))->one_data[0]).values[0] == i + 7);
		assert(equistack_Get(bigTest, 1) != NULL);
		assert((((test_data*)equistack_Get(bigTest, 1))->one_data[0]).values[0] == i + 6);
		assert(equistack_Get(bigTest, 2) != NULL);
		assert((((test_data*)equistack_Get(bigTest, 2))->one_data[0]).values[0] == i + 5);
		
		//char str[80];
		//sprintf(str, "%u\n\r", i);
		//print(str);
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

static void multi_create(void) 
{
	print("\n\r");
	int n = 100;
	for (int i = 0; i < n; i++) {
		print("-");
	}
	
	print("\r");
	for (int i = 0; i < n; i++) {
		standard_case();
		big_case();
		print("|");
	}
	
 	return;
}

void test_equistack(void) {
	basic_push_usage();
	standard_case();
	big_case();
	multi_add();
	multi_create();
	return;
}