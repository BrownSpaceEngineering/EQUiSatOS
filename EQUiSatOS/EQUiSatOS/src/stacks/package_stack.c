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
uint8_t to_char_arr(void *input, int sizeof_input, int state, char *buffer)
{
	char callsign[] = "BSErocks";
	// possibly move definition of callsign into .h file
	int checksum = CHECKSUM;
	char *charindex = (char*)input;
	// 8 is the length of callsign
	// alternatively use sizeof(callsign) / sizeof(callsign[1])
	for(int i = 0; i < 8; i++)
	{
		buffer[i] = callsign[i];
	}
	int index = 8;
	buffer[index] = state;
	index++;
	for(int i = index; i <= sizeof_input + index; i++)
	{
		buffer[i] = charindex[i - index];
	}
	index += sizeof_input;
	buffer[index] = checksum;
	buffer[index + 1] = NULL;
	return 1;
}