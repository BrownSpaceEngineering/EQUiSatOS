/*
 * test_structs.h
 *
 * Created: 3/1/2017 9:37:28 PM
 *  Author: rj16
 */ 

#ifndef TEST_STRUCTS_H
#define TEST_STRUCTS_H

#include <asf.h>
#include <inttypes.h>

typedef struct one_batch
{
	uint16_t values[12];
} one_batch;

void fill_one(one_batch*, uint16_t); 

typedef struct two_batch
{
	uint16_t values[16];	
} two_batch;

void fill_two(two_batch*, uint16_t);

typedef struct three_batch
{
	uint16_t values[12];	
} three_batch;

void fill_three(three_batch*, uint16_t);

#endif