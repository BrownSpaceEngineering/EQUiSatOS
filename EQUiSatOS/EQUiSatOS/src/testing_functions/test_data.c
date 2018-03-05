/*
 * test_data.c
 *
 * Created: 3/1/2017 9:54:37 PM
 *  Author: rj16
 */ 

#include "test_data.h"

void fill_test(test_data* id, uint16_t val) 
{
	for (int i = 0; i < 6; i++)
	{
		fill_one(&(id->one_data[i]), val);
	}
	
	for (int i = 0; i < 6; i++)
	{
		fill_two(&(id->two_data[i]), val);
	}
	
	for (int i = 0; i < 6; i++)
	{
		fill_three(&(id->three_data[i]), val);
	}
}

void longest_same_seq_len_test(void) {
	uint8_t arr1[] = {0};
	configASSERT(0 == longest_same_seq_len(arr1, 0));
	configASSERT(1 == longest_same_seq_len(arr1, 1));
	uint8_t arr2[] = {0,0};
	configASSERT(2 == longest_same_seq_len(arr2, 2));
	uint8_t arr3[] = {1, 5, 12, 182};
	configASSERT(1 == longest_same_seq_len(arr3, 4));
	uint8_t arr4[] = {1, 5, 12, 12};
	configASSERT(2 == longest_same_seq_len(arr4, 4));
	uint8_t arr5[] = {1, 255, 255, 255};
	configASSERT(3 == longest_same_seq_len(arr5, 4));
	uint8_t arr6[] = {10, 10, 0, 10, 10};
	configASSERT(2 == longest_same_seq_len(arr6, 5));
	uint8_t arr7[] = {10, 10, 10, 10, 10};
	configASSERT(5 == longest_same_seq_len(arr7, 5));
	uint8_t arr8[] = {5, 5, 5, 6, 12, 12, 13, 6, 6, 6};
	configASSERT(3 == longest_same_seq_len(arr8, 10));
	uint8_t arr9[] = {123, 24, 12, 31, 31, 13, 12};
	configASSERT(2 == longest_same_seq_len(arr9, 7));
}