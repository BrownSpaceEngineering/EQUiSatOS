/*
 * test_structs.c
 *
 * Created: 3/1/2017 9:40:12 PM
 *  Author: rj16
 */ 

#include "test_structs.h"

void fill_one(one_batch* one, uint16_t val) {
	for (int i = 0; i < 12; i++) {
		one->values[i] = val;
	}
}

void fill_two(two_batch* two, uint16_t val) {
	for (int i = 0; i < 12; i++) {
		two->values[i] = val;
	}
}

void fill_three(three_batch* three, uint16_t val) {
	for (int i = 0; i < 12; i++) {
		three->values[i] = val;
	}
}
