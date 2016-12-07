/*
 * equistacks.c
 *
 * Created: 12/6/2016 10:19:49 PM
 *  Author: mcken
 */ 

#include "equistacks.h"

void clear_existing_data(void* ptr, size_t slot_size)
{
	// convert the pointer to a char pointer to iterate over bytes
	char* byte_ptr = (char*) ptr;
	for (int16_t i = 0; i < slot_size; i++)
	{
		// set memory value to zero
		*(byte_ptr + i) = 0;
	}
}