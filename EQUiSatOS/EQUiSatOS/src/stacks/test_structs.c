/*
 * test_structs.c
 *
 * Created: 3/1/2017 9:40:12 PM
 *  Author: rj16
 */ 

#include "test_structs.h"

void free_one(one_batch* one){
	vPortFree(&(one->values));
	vPortFree(&one);
}

void fill_one(one_batch* one, uint16_t val) {
	int one_length = 0;
	if (sizeof one > 0) {
		one_length = sizeof one->values / sizeof one->values[0];
	}
	
	for (int i = 0; i < one_length; i++) {
		one->values[i] = val;
	}
}

void free_two(two_batch* two){
	vPortFree(&(two->values));
	vPortFree(&two);
}

void fill_two(two_batch* two, uint16_t val) {
	int two_length = 0;
	if (sizeof two > 0) {
		two_length = sizeof two->values / sizeof two->values[0];
	}
	
	for (int i = 0; i < two_length; i++) {
		two->values[i] = val;
	}
}

void free_three(three_batch* three){
	vPortFree(&(three->values));
	vPortFree(&three);
}

void fill_three(three_batch* three, uint16_t val) {
	int three_length = 0;
	if (sizeof three > 0) {
		three_length = sizeof three->values / sizeof three->values[0];
	}
	
	for (int i = 0; i < three_length; i++) {
		three->values[i] = val;
	}
}
