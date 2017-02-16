/*
 * stack_tests2.c
 *
 * Created: 12/13/2016 8:55:17 PM
 *  Author: rj16
 */ 

#include "stack_tests.h"

idle_Stack *s;

void Idle_Stack_Test()
{
	s = idle_Stack_Init();
	
	assert(s->size == 0);
	assert(idle_Stack_Get(s, 0) == NULL);
	
	idle_data_t *zero; 
	zero = idle_Stack_Stage(s);
	zero = create_dummy_batch();
	assert(idle_Stack_Get(s, 0) == NULL);
	assert(s->size == 0);
	
	idle_data_t *one;
	one = idle_Stack_Stage(s);
	one = create_dummy_batch();
	assert(idle_Stack_Get(s, 0) == zero);
	assert(idle_Stack_Get(s, 1) == NULL);
	assert(s->size == 1);
	
	idle_data_t *two;
	two = idle_Stack_Stage(s);
	two = create_dummy_batch();
	assert(idle_Stack_Get(s, 0) == one);
	assert(idle_Stack_Get(s, 1) == zero);
	assert(idle_Stack_Get(s, 2) == NULL);
	assert(s->size == 2);
	
	idle_data_t *last = two;
	for (int i = 0; i < 5000; i++)
	{
		idle_data_t *new;
		new = idle_Stack_Stage(s);
		new = create_dummy_batch();
		
		assert(idle_Stack_Get(s, 0) == last);
		assert(s->size <= IDLE_STACK_MAX);
		
		last = new;
	}
	
	assert(true);
}