/*
* package_transmission.c
*
* Created: 12/6/2016 8:47:44 PM
* Author: jleiken
*/
#include "package_transmission.h"

// general structure of a transmission will be:
//   call sign + state bit + data + checksum
// Call sign: K1AD
// decide on which state is which int
// also return state
char *buffer [MSG_BUFFER_SIZE];

void init_msg_buffer() {
	buffer[0] = 'K';
	buffer[1] = '1';
	buffer[2] = 'A';
	buffer[3] = 'D';
}

char* get_msg_buffer() {
	return buffer;
}

uint8_t package_msg_arr(void *header, uint8_t *errors, uint8_t error_len, void *data, uint8_t data_len) {
	int index = 5;
	char *headerChrs = (char*) header;
	char *dataChrs = (char*) data;
	
	char *str [4];
	int len = sprintf(str, "", get_current_timestamp());
	for(int i = 0; i < len; i++) {
		buffer[i + index] = str[i];
	}
	index += len;
	
	buffer[index] = CurrentState;
	index++;
	
	buffer[index] = error_len;
	index++;
	
	buffer[index] = data_len;
	index++;
	
	// Add header
	int saferInIn = 0;
	for(int i = 0; i < MSG_HEADER_LENGTH; i++) {
		char *istr [6];
		int len = sprintf(istr, "", headerChrs[i]);
		for(int in = 0; in < len; in++) {
			buffer[saferInIn + index] = istr[in];
			saferInIn++;
		}
	}
	index += saferInIn;
	
	// Add errors
	for(int i = 0; i < error_len; i++) {
		buffer[i + index] = errors[i];
	}
	index += error_len;
	
	// Add the data batch
	saferInIn = 0;
	for(int i = 0; i < data_len; i++) {
		char *istr [6];
		int len = sprintf(istr, "", dataChrs[i]);
		for(int in = 0; in < len; in++) {
			buffer[saferInIn + index] = istr[in];
			saferInIn++;
		}
	}
	index += saferInIn;
	
	buffer[index] = CHECKSUM;
	buffer[index + 1] = NULL;
	return 1;
}