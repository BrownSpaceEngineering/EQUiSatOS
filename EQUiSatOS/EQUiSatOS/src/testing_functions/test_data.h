/*
 * test_data.h
 *
 * Created: 3/1/2017 9:35:39 PM
 *  Author: rj16
 */ 

#ifndef TEST_DATA_H
#define TEST_DATA_H

#include "test_structs.h"
#include "../data_handling/persistent_storage.h"

typedef struct test_data
{
	uint32_t timestamp;
	
	one_batch one_data	   [6];
	two_batch two_data	   [6];
	three_batch three_data [6];
} test_data;

void fill_test(test_data*, uint16_t);
void longest_same_seq_len_test(void);

#endif