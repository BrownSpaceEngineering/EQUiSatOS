/*
* package_stack.c
*
* Created: 12/6/2016 8:47:44 PM
*  Author: jleiken
*/
#include "package_stack.h"

// general structure of a transmission will be:
//   callsign + statebit + data + checksum
// this definitely won't be the final implementation (callsign and checksum need to be changed)
// decide on which state is which int
// also return state

// buffer definitely needs to have enough space for all the data
uint8_t to_char_arr(void *input, uint32_t time, int state, int *errors, char *buffer)
{
	char callsign[] = "k1ad";
	// possibly move definition of callsign into .h file
	int checksum = CHECKSUM;
	// number of bytes that can be transmitted in one second
	int size_allocated = 1200;
	char *charindex = (char*)input;
	// 4 bytes is the length of callsign
	// alternatively use sizeof(callsign) / sizeof(callsign[1])
	int index = 1;
	for(int i = 0; i < 4; i++)
	{
		buffer[i] = callsign[i];
		index++;
	}
	
	// MUST FIX, TOTAL PLACEHOLDER
	for(int i = index; i < index + 4; i++)
	{
		buffer[i] = time;
		index++;
	}
	
	buffer[index] = state;
	index++;
	
	// ALMOST FOR SURE WILL REQUIRE FIX
	for(int i = index; i < index + 8; i++)
	{
		buffer[i] = errors[i];
		index++;
	}
	
	for(int i = index; i <= size_allocated + index; i++)
	{
		buffer[i] = charindex[i - index];
		index++;
	}
	
	buffer[index] = checksum;
	buffer[index + 1] = NULL;
	return 1;
}