/*
 * data_utils.c
 *
 * Created: 11/19/2017 17:16:31
 *  Author: mcken
 */ 

#include "global.h"
#include "data_utils.h"

void increment_all_uint8(uint8_t* int_arr, uint8_t length)
{
	for(uint8_t i = 0; i < length; i++)
	{
		int_arr[i] = int_arr[i] + 1;
	}
}

void set_all_uint8(uint8_t* int_arr, uint8_t length, uint8_t value)
{
	for(uint8_t i = 0; i < length; i++)
	{
		int_arr[i] = value;
	}
}