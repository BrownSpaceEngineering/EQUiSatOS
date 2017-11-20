/*
* package_transmission.c
*
* Created: 12/6/2016 8:47:44 PM
* Author: jleiken
*/
#include "package_transmission.h"

void assert_transmission_constants(void) {
	// check things will fit in buffer (one space for \0)
	int top_length = MSG_PREAMBLE_LENGTH + MSG_CUR_DATA_LEN + ERROR_PACKET_SIZE * ERROR_PACKETS;
	assert(top_length + ATTITUDE_DATA_PACKET_SIZE * ATTITUDE_DATA_PACKETS < MSG_BUFFER_SIZE - 1);
	assert(top_length + FLASH_DATA_PACKET_SIZE * FLASH_DATA_PACKETS < MSG_BUFFER_SIZE - 1);
	assert(top_length + FLASH_CMP_DATA_PACKET_SIZE * FLASH_DATA_PACKETS < MSG_BUFFER_SIZE - 1);

	// check that starts line up with data sizes
	assert(MSG_PREAMBLE_LENGTH == START_CUR_DATA);
	assert(MSG_PREAMBLE_LENGTH + MSG_CUR_DATA_LEN == START_ERRORS);
	assert(MSG_PREAMBLE_LENGTH + MSG_CUR_DATA_LEN + ERROR_PACKET_SIZE * ERROR_PACKETS == START_DATA);
}

/**
 * Writes the given data to the preamble of the message buffer.
 */
void write_preamble(uint8_t* buffer, uint32_t timestamp, uint8_t states, uint8_t data_len) {
	uint8_t index = START_PREAMBLE;

	// write callsign
	buffer[index++] = 'K';
	buffer[index++] = '1';
	buffer[index++] = 'A';
	buffer[index++] = 'D';

	write_bytes_and_shift(buffer,	&timestamp,			sizeof(timestamp),				&index); // 4 byte timestamp
	write_bytes_and_shift(buffer,	&states,			sizeof(states),					&index); // 1 byte state string
	write_bytes_and_shift(buffer,	&data_len,			sizeof(data_len),				&index); // 1 byte data packet #

	assert(index == START_CUR_DATA);

	// just in case we miss a \0... it should normally be written at the end of data
	buffer[MSG_BUFFER_SIZE - 1] = '\0';
}

/* read actual sensors to write to message buffer - saves memory */
void write_current_data(uint8_t* buffer, uint32_t timestamp) {
	uint8_t index = START_CUR_DATA;
	
	uint32_t secs_to_next_flash = get_time_of_next_flash() - timestamp;
	if (secs_to_next_flash > 0xff) {
		secs_to_next_flash = 0xff;
	}
	write_bytes_and_shift(buffer, &secs_to_next_flash, 1, &index);

	read_lion_volts_batch((uint8_t*) (buffer + index));
	index += sizeof(lion_volts_batch);

	read_lion_current_batch((uint8_t*) (buffer + index));
	index += sizeof(lion_current_batch);
	
	read_lion_temps_batch((uint8_t*) (buffer + index));
	index += sizeof(lion_temps_batch);

	read_bat_charge_volts_batch((uint8_t*) (buffer + index));
	index += sizeof(bat_charge_volts_batch);
	
	read_lifepo_volts_batch((uint8_t*) (buffer + index));
	index += sizeof(lifepo_volts_batch);

	read_bat_charge_dig_sigs_batch((uint16_t*) (buffer + index));
	index += sizeof(bat_charge_dig_sigs_batch);

	assert(index == START_ERRORS);
}

void write_error(uint8_t* buffer, sat_error_t* error, uint32_t timestamp, uint8_t* index) {
	// we have to fill up the error section, so either write the error or NULL bytes
	if (error != NULL) {
		// we calculate an offset timestamp for each error (relative to the current packet)
		// this saves space, but we need to reduce the resolution to ERROR_TIME_BUCKET_SIZE
		uint32_t time_offset = (timestamp - error->timestamp) / ERROR_TIME_BUCKET_SIZE;
		// if the time offset is too large, note this as the max time offset (0xff = 1111 1111)
		// otherwise, it could be any value the last 8 bits.
		if (time_offset > 0xff) {
			time_offset = 0xff;
		}
		write_bytes_and_shift(buffer, &time_offset,				1, index);
		write_bytes_and_shift(buffer, &(error->eloc),			1, index);
		write_bytes_and_shift(buffer, &(error->ecode),			1, index);
	} else {
		write_value_and_shift(buffer, 0,	ERROR_PACKET_SIZE,	index);
	}
}

void write_errors(uint8_t* buffer, equistack* priority_equistack, equistack* normal_equistack, uint32_t timestamp) {
	uint8_t index = START_ERRORS;

	// note: _PACKETS should be less than _STACK_MAX, or we're wasting space!
	for (int i = 0; i < PRIORITY_ERROR_PACKETS; i++) {
		sat_error_t* error = (sat_error_t*) equistack_Get(priority_equistack, i);
		write_error(buffer, error, timestamp, &index);
	}
	for (int i = 0; i < NORMAL_ERROR_PACKETS; i++) {
		sat_error_t* error = (sat_error_t*) equistack_Get(normal_equistack, i);
		write_error(buffer, error, timestamp, &index);
	}

	assert(index == START_DATA);
}

void write_idle_data(uint8_t* buffer, equistack* idle_stack) {
	uint8_t index = START_DATA;

	// note: _PACKETS should be less than _STACK_MAX, or we're wasting space!
	for (int i = 0; i < IDLE_DATA_PACKETS; i++) {
		idle_data_t *idle_data = (idle_data_t*) equistack_Get(idle_stack, i);

		if (idle_data != NULL) {
			write_bytes_and_shift(buffer, &(idle_data->satellite_history),		sizeof(satellite_history_batch),	&index);
			write_bytes_and_shift(buffer, &(idle_data->reboot_count),			sizeof(uint8_t),					&index);
			write_bytes_and_shift(buffer, idle_data->lion_volts_data,			sizeof(lion_volts_batch),			&index);
			write_bytes_and_shift(buffer, idle_data->lion_current_data,			sizeof(lion_current_batch),			&index);
			write_bytes_and_shift(buffer, idle_data->lion_temps_data,			sizeof(lion_temps_batch),			&index);
			write_bytes_and_shift(buffer, idle_data->bat_charge_volts_data,		sizeof(bat_charge_volts_batch),		&index);
			write_bytes_and_shift(buffer, &(idle_data->bat_charge_dig_sigs_data),sizeof(bat_charge_dig_sigs_batch),	&index);
			write_bytes_and_shift(buffer, &(idle_data->rail_5v_data),			sizeof(rail_5v_batch),				&index);
			write_bytes_and_shift(buffer, &(idle_data->radio_temp_data),		sizeof(radio_temp_batch),			&index);
			write_bytes_and_shift(buffer, &(idle_data->radio_volts_data),		sizeof(radio_volts_batch),			&index);
			write_bytes_and_shift(buffer, &(idle_data->radio_current_data),		sizeof(radio_current_batch),		&index);
			write_bytes_and_shift(buffer, &(idle_data->proc_temp_data),			sizeof(proc_temp_batch),			&index);
			write_bytes_and_shift(buffer, idle_data->ir_amb_temps_data,			sizeof(ir_ambient_temps_batch),		&index);
			write_bytes_and_shift(buffer, &(idle_data->timestamp),				4 /* uint32_t */,					&index);
		} else {
			// write all 0s to buffer for idle data
			write_value_and_shift(buffer, 0, IDLE_DATA_PACKET_SIZE, &index);
		}
	}

	assert(index == START_DATA + IDLE_DATA_PACKET_SIZE * IDLE_DATA_PACKETS);
	write_value_and_shift(buffer, 0, IDLE_DATA_PADDING_SIZE, &index);
	assert(index == START_PARITY);
}

void write_attitude_data(uint8_t* buffer, equistack* attitude_stack) {
	uint8_t index = START_DATA;

	// note: _PACKETS should be less than _STACK_MAX, or we're wasting space!
	for (int i = 0; i < ATTITUDE_DATA_PACKETS; i++) {
		attitude_data_t *attitude_data = (attitude_data_t*) equistack_Get(attitude_stack, i);

		// we have to fill up the entire section, so either write the data or its null equivalent
		if (attitude_data != NULL) {
			write_bytes_and_shift(buffer, attitude_data->ir_obj_temps_data,	sizeof(ir_object_temps_batch)	/* [1] */,		&index);
			write_bytes_and_shift(buffer, attitude_data->diode_data,		sizeof(pdiode_batch)			/* [1] */,		&index);
			write_bytes_and_shift(buffer, attitude_data->accelerometer_data,sizeof(accelerometer_batch) * 2 /* [2] */,		&index);
			write_bytes_and_shift(buffer, attitude_data->gyro_data,			sizeof(gyro_batch)				/* [2] */,		&index);
			write_bytes_and_shift(buffer, attitude_data->magnetometer_data,	sizeof(magnetometer_batch)		/* [1] */,		&index);
			write_bytes_and_shift(buffer, &attitude_data->timestamp,		4 /* uint32_t */,								&index);
		} else {
			// overwrite entire section with 0s
			write_value_and_shift(buffer, 0, ATTITUDE_DATA_PACKET_SIZE, &index);
		}
	}

	assert(index == START_DATA + ATTITUDE_DATA_PACKET_SIZE * ATTITUDE_DATA_PACKETS);
	write_value_and_shift(buffer, 0, ATTITUDE_DATA_PADDING_SIZE, &index);
	assert(index == START_PARITY);
}

void write_flash_data(uint8_t* buffer, equistack* flash_stack) {
	uint8_t index = START_DATA;

	// note: _PACKETS should be less than _STACK_MAX, or we're wasting space!
	for (int i = 0; i < FLASH_DATA_PACKETS; i++) {
		flash_data_t *flash_data = (flash_data_t*) equistack_Get(flash_stack, i);

		// we have to fill up the entire section, so either write the data or its null equivalent
		if (flash_data != NULL) {
			write_bytes_and_shift(buffer, flash_data->led_temps_data,		sizeof(led_temps_batch)		* FLASH_DATA_ARR_LEN /* [10] */,	&index);
			write_bytes_and_shift(buffer, flash_data->lifepo_volts_data,	sizeof(lifepo_volts_batch)	* FLASH_DATA_ARR_LEN /* [10] */,	&index);
			write_bytes_and_shift(buffer, flash_data->lifepo_current_data,	sizeof(lifepo_current_batch)* FLASH_DATA_ARR_LEN /* [10] */,	&index);
			write_bytes_and_shift(buffer, flash_data->led_current_data,		sizeof(led_current_batch)	* FLASH_DATA_ARR_LEN /* [10] */,	&index);
			write_bytes_and_shift(buffer, flash_data->gyro_data,			sizeof(gyro_batch)			* FLASH_DATA_ARR_LEN /* [10] */,	&index);
			write_bytes_and_shift(buffer, &flash_data->timestamp,			4 /* uint_32_t */,												&index);

		} else {
			// overwrite entire section with 0s
			write_value_and_shift(buffer, 0, FLASH_DATA_PACKET_SIZE, &index);
		}
	}

	assert(index == START_DATA + FLASH_DATA_PACKET_SIZE * FLASH_DATA_PACKETS);
	write_value_and_shift(buffer, 0, FLASH_DATA_PADDING_SIZE, &index);
	assert(index == START_PARITY);
}

void write_flash_cmp(uint8_t* buffer, equistack* flash_cmp_stack) {
	uint8_t index = START_DATA;

	// note: _PACKETS should be less than _STACK_MAX, or we're wasting space!
	for (int i = 0; i < FLASH_CMP_DATA_PACKETS; i++) {
		flash_cmp_data_t *flash_cmp = (flash_cmp_data_t*) equistack_Get(flash_cmp_stack, i);

		// we have to fill up the entire section, so either write the data or its null equivalent
		if (flash_cmp != NULL) {
			/* NOTE: Though these are the same types as in flash_data_t, they are those values AVERAGED */
			write_bytes_and_shift(buffer, flash_cmp->led_temps_avg_data,		sizeof(led_temps_batch)			/* [1] */,			&index);
			write_bytes_and_shift(buffer, flash_cmp->lifepo_volts_avg_data,		sizeof(lifepo_volts_batch)		/* [1] */,			&index);
			write_bytes_and_shift(buffer, flash_cmp->lifepo_current_avg_data,	sizeof(lifepo_current_batch)	/* [1] */,			&index);
			write_bytes_and_shift(buffer, flash_cmp->led_current_avg_data,		sizeof(led_current_batch)		/* [1] */,			&index);
			write_bytes_and_shift(buffer, flash_cmp->gyro_avg_data,				sizeof(gyro_batch)				/* [1] */,			&index);
			write_bytes_and_shift(buffer, &flash_cmp->timestamp,				4 /* uint_32_t */,									&index);

		} else {
			// overwrite entire section with 0s
			write_value_and_shift(buffer, 0, FLASH_CMP_DATA_PACKET_SIZE, &index);
		}
	}

	assert(index == START_DATA + FLASH_CMP_DATA_PACKET_SIZE * FLASH_CMP_DATA_PACKETS);
	write_value_and_shift(buffer, 0, FLASH_CMP_DATA_PADDING_SIZE, &index);
	assert(index == START_PARITY);
}

void write_low_power_data(	uint8_t* buffer, equistack *low_power_stack) {
	uint8_t index = START_DATA;

	// note: _PACKETS should be less than _STACK_MAX, or we're wasting space!
	for (int i = 0; i < LOW_POWER_DATA_PACKETS; i++) {
		low_power_data_t *low_power_data = (low_power_data_t*) equistack_Get(low_power_stack, i);

		if (low_power_data != NULL) {
			write_bytes_and_shift(buffer, &(low_power_data->satellite_history),			sizeof(satellite_history_batch),	&index);
			write_bytes_and_shift(buffer, low_power_data->lion_volts_data,				sizeof(lion_volts_batch),			&index);
			write_bytes_and_shift(buffer, low_power_data->lion_current_data,			sizeof(lion_current_batch),			&index);
			write_bytes_and_shift(buffer, low_power_data->lion_temps_data,				sizeof(lion_temps_batch),			&index);
			write_bytes_and_shift(buffer, low_power_data->bat_charge_volts_data,		sizeof(bat_charge_volts_batch),		&index);
			write_bytes_and_shift(buffer, &(low_power_data->bat_charge_dig_sigs_data),sizeof(bat_charge_dig_sigs_batch),	&index);
			write_bytes_and_shift(buffer, low_power_data->ir_obj_temps_data,			sizeof(ir_object_temps_batch),		&index);
			write_bytes_and_shift(buffer, low_power_data->gyro_data,					sizeof(gyro_batch),					&index);
			write_bytes_and_shift(buffer, &(low_power_data->timestamp),					4 /* uint32_t */,					&index);
		} else {
			// write all 0s to buffer for idle data
			write_value_and_shift(buffer, 0, LOW_POWER_DATA_PACKET_SIZE, &index);
		}
	}

	assert(index == START_DATA + LOW_POWER_DATA_PACKET_SIZE * LOW_POWER_DATA_PACKETS);
	write_value_and_shift(buffer, 0, LOW_POWER_DATA_PADDING_SIZE, &index);
	assert(index == START_PARITY);
}

/* writes error correction bytes. Must be called after full message before it was written, obviously. */
void write_parity(uint8_t* buffer) {
	// encode using Reed-Solomon (START_PARITY is the number of bytes in buffer before parity section)
	// NOTE we don't encode the callsign
	encode_data(buffer + CALLSIGN_SIZE, START_PARITY - CALLSIGN_SIZE, buffer + CALLSIGN_SIZE);
}

/* Writes num_bytes from input to data, and shifts the value at index up by num_bytes */
void write_bytes_and_shift(uint8_t *data, void *input, size_t num_bytes, uint8_t *index) {
	// TODO: verify correctness
	memcpy(data, (char*) input, num_bytes);
	*index += num_bytes;
}

/* Writes num_bytes copies of value to data, and shifts the value at index up by num_bytes */
void write_value_and_shift(uint8_t *data, char value, size_t num_bytes, uint8_t *index) {
	for (uint i = 0; i < num_bytes; i++) {
		data[*index] = value;
		(*index)++;
	}
}
