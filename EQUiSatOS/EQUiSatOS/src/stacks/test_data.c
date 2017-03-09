/*
 * test_data.c
 *
 * Created: 3/1/2017 9:54:37 PM
 *  Author: rj16
 */ 

#include "test_data.h"

void free_test(test_data* id)
{
	for (int i = 0; i < sizeof(id->one_data); i++)
	{
		free_one(&(id->one_data[i]));
	}
	vPortFree(&(id->one_data));
	
	for (int i = 0; i < sizeof(id->two_data); i++)
	{
		free_two(&(id->two_data[i]));
	}
	vPortFree(&(id->two_data));
	
	for (int i = 0; i < sizeof(id->three_data); i++)
	{
		free_three(&(id->three_data[i]));
	}
	vPortFree(&(id->three_data));
}

void fill_test(test_data* id, uint16_t val) 
{
	for (int i = 0; i < sizeof(id->one_data) / sizeof(uint16_t); i++)
	{
		fill_one(&(id->one_data[i]), val);
	}
	
	for (int i = 0; i < sizeof(id->two_data) / sizeof(uint16_t); i++)
	{
		fill_two(&(id->two_data[i]), val);
	}
	
	for (int i = 0; i < sizeof(id->three_data) / sizeof(uint16_t); i++)
	{
		fill_three(&(id->three_data[i]), val);
	}
}