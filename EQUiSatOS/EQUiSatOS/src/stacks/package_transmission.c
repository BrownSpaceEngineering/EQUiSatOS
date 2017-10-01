/*
* package_transmission.c
*
* Created: 12/6/2016 8:47:44 PM
* Author: jleiken
*/
#include "package_transmission.h"

// general structure of a transmission will be:
//   call sign + state stuff + data + checksum
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

/**
 * Writes the given data to the preamble of the message buffer.
 */
void write_preamble(uint32_t timestamp, uint32_t states, uint8_t data_len) {
	uint8_t index = START_PREAMBLE; // start off end of callsign
	write_bytes_and_shift(buffer, timestamp, sizeof(timestamp), &index); // 4 byte timestamp
	write_bytes_and_shift(buffer, states, sizeof(states), &index); // 4 byte state string
	write_bytes_and_shift(buffer, ERROR_PACKETS, 1, &index); // 1 byte error packet #
	write_bytes_and_shift(buffer, data_len, sizeof(data_len), &index); // 1 byte data packet #
	assert(index == START_HEADER);
}

void write_header(idle_data_t *idle_data) {
	uint8_t index = START_HEADER;
	//write_bytes_and_shift(buffer, idle_data);
	
	
	// TODO
	
	
	assert(index == START_ERRORS);
}

void write_errors(equistack* error_stack) {
	uint8_t index = START_ERRORS;
	
	// TODO
	
	
	assert(index == START_DATA);
}

void write_attitude_data(equistack* attitude_stack) {
	uint8_t index = START_DATA;
	// TODO
	assert(index == START_DATA + ATTITUDE_DATA_PACKET_SIZE * ATTITUDE_DATA_PACKETS);
}

void write_transmit_data(equistack* transmit_stack) {
	uint8_t index = START_DATA;
	// TODO
	assert(index == START_DATA + TRANSMIT_DATA_PACKET_SIZE * TRANSMIT_DATA_PACKETS);
}

void write_flash_data(equistack* flash_stack) {
	uint8_t index = START_DATA;
	// TODO
	assert(index == START_DATA + FLASH_DATA_PACKET_SIZE * FLASH_DATA_PACKETS);
}

void write_bytes_and_shift(char *data, void *input, size_t num_bytes, uint8_t *index) {
	memcpy(data[*index], (char*) input, num_bytes);
	*index += num_bytes;
}




uint8_t package_msg_arr(void *header, uint8_t *errors, uint8_t error_len, void *data, uint8_t data_len) {
	int index = 5;
	char *headerChrs = (char*) header;
	char *dataChrs = (char*) data;
	
	char *str [4];
	int len = sprintf(str, "%f", get_current_timestamp());
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
		buffer[index + i] = headerChrs[i];
		saferInIn++;
		// what was I doing here?
		/*char *istr [6];
		int len = sprintf(istr, "%f", headerChrs[i]);
		for(int in = 0; in < len; in++) {
			buffer[saferInIn + index] = istr[in];
			saferInIn++;
		}*/
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
		buffer[index + i] = dataChrs[i];
		saferInIn++;
		// seriously wtf was I doing
		/*char *istr [6];
		int len = sprintf(istr, "", dataChrs[i]);
		for(int in = 0; in < len; in++) {
			buffer[saferInIn + index] = istr[in];
			saferInIn++;
		}*/
	}
	index += saferInIn;
	
	buffer[index] = CHECKSUM;
	buffer[index + 1] = NULL;
	return 1;
}