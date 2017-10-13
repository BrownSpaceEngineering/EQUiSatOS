/*
* package_transmission.c
*
* Created: 12/6/2016 8:47:44 PM
* Author: jleiken
*/
#include "package_transmission.h"

void init_msg_buffer(char* buffer) {
	
	
	// [TEST] Assertions regarding constants
	assert_transmission_constants();
}

void assert_transmission_constants(void) {
	// check things will fit in buffer (one space for \0)
	int top_length = MSG_PREAMBLE_LENGTH + MSG_HEADER_LENGTH + ERROR_PACKET_SIZE * ERROR_PACKETS;
	assert(top_length + ATTITUDE_DATA_PACKET_SIZE * ATTITUDE_DATA_PACKETS < MSG_BUFFER_SIZE - 1);
	assert(top_length + TRANSMIT_DATA_PACKET_SIZE * TRANSMIT_DATA_PACKETS < MSG_BUFFER_SIZE - 1);
	assert(top_length + FLASH_DATA_PACKET_SIZE * FLASH_DATA_PACKETS < MSG_BUFFER_SIZE - 1);
	
	// check that starts line up with data sizes
	assert(MSG_PREAMBLE_LENGTH == START_HEADER);
	assert(MSG_PREAMBLE_LENGTH + MSG_HEADER_LENGTH == START_ERRORS);
	assert(MSG_PREAMBLE_LENGTH + MSG_HEADER_LENGTH + ERROR_PACKET_SIZE * ERROR_PACKETS == START_DATA);
}

/**
 * Writes the given data to the preamble of the message buffer.
 */
void write_preamble(char* buffer, uint32_t timestamp, uint32_t states, uint8_t data_len) {
	uint8_t index = START_PREAMBLE;
	
	// re-write callsign
	buffer[index++] = 'K';
	buffer[index++] = '1';
	buffer[index++] = 'A';
	buffer[index++] = 'D';
	
	write_bytes_and_shift(buffer,	timestamp,		sizeof(timestamp),	&index); // 4 byte timestamp
	write_bytes_and_shift(buffer,	states,			sizeof(states),		&index); // 4 byte state string
	write_bytes_and_shift(buffer,	ERROR_PACKETS,	1,					&index); // 1 byte error packet #
	write_bytes_and_shift(buffer,	data_len,		sizeof(data_len),	&index); // 1 byte data packet #
	
	assert(index == START_HEADER);
	
	// just in case we miss a \0... it should normally be written at the end of data
	buffer[MSG_BUFFER_SIZE - 1] = '\0'; 
}

// WARNING: for writing the big structs to the buffer, we CAN'T always use sizeof, because the types are defined
// as pointers to arrays (see Sensor_Structs to see what I mean).

void write_header(char* buffer, idle_data_t *idle_data) {
	uint8_t index = START_HEADER;
	
	if (idle_data != NULL) {
		// TODO: drop two LSB (see write_bytes_and_shift_truncating)
		write_bytes_and_shift(buffer, idle_data->lion_volts_data,		sizeof(lion_volts_batch),			&index);
		write_bytes_and_shift(buffer, idle_data->lion_current_data,		sizeof(lion_current_batch),			&index);
		write_bytes_and_shift(buffer, idle_data->led_temps_data,		sizeof(led_temps_batch),			&index);
		write_bytes_and_shift(buffer, idle_data->lifepo_current_data,	sizeof(lifepo_current_batch),		&index);
		write_bytes_and_shift(buffer, idle_data->ir_data,				sizeof(ir_batch),					&index);
		write_bytes_and_shift(buffer, idle_data->diode_data,			sizeof(diode_batch),				&index);
		write_bytes_and_shift(buffer, idle_data->bat_temp_data,			sizeof(bat_temp_batch),				&index);
		write_bytes_and_shift(buffer, idle_data->ir_temps_data,			sizeof(ir_temps_batch),				&index);
		write_bytes_and_shift(buffer, idle_data->radio_temp_data,		sizeof(radio_temp_batch),			&index);
		if (idle_data->imu_data != NULL) {
			write_bytes_and_shift(buffer, idle_data->imu_data->accelerometer,	6 /* uint16_t[3] */,		&index);
			write_bytes_and_shift(buffer, idle_data->imu_data->gyro,			6 /* uint16_t[3] */,		&index);	
		} else {
			index += 12; // **** MAKE SURE TO DOUBLE-CHECK THIS WITH ABOVE ****
			log_error(ELOC_PACKAGE_TRANS, ECODE_NULL_IMU_DATA);
		}
		write_bytes_and_shift(buffer, idle_data->magnetometer_data,		sizeof(magnetometer_batch),			&index);
		write_bytes_and_shift(buffer, idle_data->led_current_data,		sizeof(led_current_batch),			&index);
		write_bytes_and_shift(buffer, idle_data->radio_volts_data,		sizeof(radio_volts_batch),			&index);
		write_bytes_and_shift(buffer, idle_data->bat_charge_volts_data,	sizeof(bat_charge_volts_batch),		&index);
		write_bytes_and_shift(buffer, idle_data->bat_charge_dig_sigs_data,	sizeof(bat_charge_dig_sigs_batch),	&index);
		write_bytes_and_shift(buffer, idle_data->digital_out_data,		sizeof(digital_out_batch),			&index);
	} else {
		// write all 0s to buffer for idle data
		write_value_and_shift(buffer, 0, MSG_HEADER_LENGTH, &index);
		log_error(ELOC_PACKAGE_TRANS, ECODE_NULL_IDLE_DATA);
	}
		
	assert(index == START_ERRORS);
}

void write_errors(char* buffer, equistack* error_stack) {
	uint8_t index = START_ERRORS;	
	
	// note: _PACKETS should be less than _STACK_MAX, or we're wasting space!
	for (int i = 0; i < ERROR_PACKETS; i++) {
		sat_error_t *error = (sat_error_t*) equistack_Get(error_stack, i);
		
		// we have to fill up the error section, so either write the error or it's null equivalent
		if (error != NULL) {
			write_bytes_and_shift(buffer, *error,			sizeof(sat_error_t), &index);
		} else {
			write_bytes_and_shift(buffer, ECODE_NO_ERROR,	sizeof(sat_error_t), &index);
		}
	}
	
	assert(index == START_DATA);
}

void write_attitude_data(char* buffer, equistack* attitude_stack) {
	uint8_t index = START_DATA;
	
	// note: _PACKETS should be less than _STACK_MAX, or we're wasting space!
	for (int i = 0; i < ATTITUDE_DATA_PACKETS; i++) {
		attitude_data_t *attitude_data = (attitude_data_t*) equistack_Get(attitude_stack, i);
		
		// we have to fill up the entire section, so either write the data or its null equivalent
		if (attitude_data != NULL) {
			write_bytes_and_shift(buffer, attitude_data->ir_data,			sizeof(ir_batch)	* 1 /* [1] */,		&index);
			write_bytes_and_shift(buffer, attitude_data->diode_data,		sizeof(diode_batch) * 1 /* [1] */,	&index);
			if (attitude_data->imu_data != NULL) {
				write_bytes_and_shift(buffer, attitude_data->imu_data->accelerometer,	30 /* uint16_t[3][5] */,	&index);
				write_bytes_and_shift(buffer, attitude_data->imu_data->gyro,			30 /* uint16_t[3][5] */,	&index);
			} else {
				index += 60; // **** MAKE SURE TO DOUBLE-CHECK THIS WITH ABOVE ****
				log_error(ELOC_PACKAGE_TRANS, ECODE_NULL_IMU_DATA);
			}
			write_bytes_and_shift(buffer, attitude_data->magnetometer_data,	sizeof(magnetometer_batch) * 1 /* [1] */,	&index); // TODO: which magnetometer?
			write_bytes_and_shift(buffer, attitude_data->timestamp,			4 /* uint32_t */,							&index); 
			
		} else {
			// overwrite entire section with 0s
			write_value_and_shift(buffer, 0, ATTITUDE_DATA_PACKET_SIZE, &index);
		}
	}
	
	assert(index == START_DATA + ATTITUDE_DATA_PACKET_SIZE * ATTITUDE_DATA_PACKETS);
	buffer[index] = '\0';
}

void write_transmit_data(char* buffer, equistack* transmit_stack) {
	uint8_t index = START_DATA;
	
	// note: _PACKETS should be less than _STACK_MAX, or we're wasting space!
	for (int i = 0; i < TRANSMIT_DATA_PACKETS; i++) {
		transmit_data_t *transmit_data = (transmit_data_t*) equistack_Get(transmit_stack, i);
		
		// we have to fill up the entire section, so either write the data or its null equivalent
		if (transmit_data != NULL) {
			write_bytes_and_shift(buffer, transmit_data->radio_temp_data,		sizeof(radio_temp_batch)	* 2 /* [2] */,		&index);
			write_bytes_and_shift(buffer, transmit_data->lion_volts_data,		sizeof(lion_volts_batch)	* 1 /* [1] */,		&index);
			write_bytes_and_shift(buffer, transmit_data->lion_current_data,		sizeof(lion_current_batch)	* 1 /* [1] */,		&index); 
			write_bytes_and_shift(buffer, transmit_data->timestamp,				4 /* uint_32_t */,							&index);
			
			} else {
			// overwrite entire section with 0s
			write_value_and_shift(buffer, 0, TRANSMIT_DATA_PACKET_SIZE, &index);
		}
	}
	
	assert(index == START_DATA + TRANSMIT_DATA_PACKET_SIZE * TRANSMIT_DATA_PACKETS);
	buffer[index] = '\0';
}

void write_flash_data(char* buffer, equistack* flash_stack) {
	uint8_t index = START_DATA;
	
	// note: _PACKETS should be less than _STACK_MAX, or we're wasting space!
	for (int i = 0; i < FLASH_DATA_PACKETS; i++) {
		flash_data_t *flash_data = (flash_data_t*) equistack_Get(flash_stack, i);
		
		// we have to fill up the entire section, so either write the data or its null equivalent
		if (flash_data != NULL) {
			write_bytes_and_shift(buffer, flash_data->led_temps_data,		sizeof(led_temps_batch)		* 1 /* [1] */,			&index); 
			write_bytes_and_shift(buffer, flash_data->lifepo_volts_data,	sizeof(lifepo_volts_batch)	* 2 /* [2] */,			&index); 
			write_bytes_and_shift(buffer, flash_data->lifepo_current_data,	sizeof(lifepo_current_batch)* 2 /* [2] */,			&index); 
			write_bytes_and_shift(buffer, flash_data->led_current_data,		sizeof(led_current_batch)	* 2 /* [2] */,			&index); 
			write_bytes_and_shift(buffer, flash_data->timestamp,			4 /* uint_32_t */,									&index);
			
		} else {
			// overwrite entire section with 0s
			write_value_and_shift(buffer, 0, FLASH_DATA_PACKET_SIZE, &index);
		}
	}
	
	assert(index == START_DATA + FLASH_DATA_PACKET_SIZE * FLASH_DATA_PACKETS);
	buffer[index] = '\0';
}

/* Writes num_bytes from input to data, and shifts the value at index up by num_bytes */
void write_bytes_and_shift(char *data, void *input, size_t num_bytes, uint8_t *index) {
	memcpy(data[*index], (char*) input, num_bytes);
	*index += num_bytes;
}

/* Writes num_bytes copies of value to data, and shifts the value at index up by num_bytes */
void write_value_and_shift(char *data, char value, size_t num_bytes, uint8_t *index) {
	for (int i = 0; i < num_bytes; i++) {
		data[*index] = value;
		(*index)++;
	}
}