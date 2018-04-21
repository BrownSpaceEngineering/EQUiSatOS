/*
* package_transmission.c
*
* Created: 12/6/2016 8:47:44 PM
* Author: mckenna
*/
#include "package_transmission.h"

void assert_transmission_constants(void) {
	// NOTE: if these are correct, they may be optimized out (which is fine)

	// check that starts line up with data sizes
	configASSERT(MSG_PREAMBLE_LENGTH == START_CUR_DATA);
	configASSERT(MSG_PREAMBLE_LENGTH + MSG_CUR_DATA_LEN == START_DATA);

	configASSERT(MSG_PREAMBLE_LENGTH + MSG_CUR_DATA_LEN + IDLE_DATA_PACKETS * IDLE_DATA_PACKET_SIZE +
		IDLE_DATA_NUM_ERRORS * ERROR_PACKET_SIZE + IDLE_DATA_PADDING_SIZE == START_PARITY);

	configASSERT(MSG_PREAMBLE_LENGTH + MSG_CUR_DATA_LEN + ATTITUDE_DATA_PACKETS * ATTITUDE_DATA_PACKET_SIZE +
		ATTITUDE_DATA_NUM_ERRORS * ERROR_PACKET_SIZE + ATTITUDE_DATA_PADDING_SIZE == START_PARITY);

	configASSERT(MSG_PREAMBLE_LENGTH + MSG_CUR_DATA_LEN + FLASH_DATA_PACKETS * FLASH_DATA_PACKET_SIZE +
		FLASH_DATA_NUM_ERRORS * ERROR_PACKET_SIZE + FLASH_DATA_PADDING_SIZE == START_PARITY);

	configASSERT(MSG_PREAMBLE_LENGTH + MSG_CUR_DATA_LEN + FLASH_CMP_DATA_PACKETS * FLASH_CMP_DATA_PACKET_SIZE +
		FLASH_CMP_DATA_NUM_ERRORS * ERROR_PACKET_SIZE + FLASH_CMP_DATA_PADDING_SIZE == START_PARITY);

	configASSERT(MSG_PREAMBLE_LENGTH + MSG_CUR_DATA_LEN + LOW_POWER_DATA_PACKETS * LOW_POWER_DATA_PACKET_SIZE +
		LOW_POWER_DATA_NUM_ERRORS * ERROR_PACKET_SIZE + LOW_POWER_DATA_PADDING_SIZE == START_PARITY);

	// check things will fit in buffer (one space for \0)
	configASSERT(START_PARITY + MSG_PARITY_LENGTH <= MSG_BUFFER_SIZE - 1);
}

/* read actual sensors and write to the given current data buffer, which must be passed to
   a call to write_packet to be copied into the main message buffer */
void read_current_data(uint8_t* cur_data_buf, uint32_t timestamp) {
	uint8_t buf_index = 0;

	uint32_t secs_to_next_flash = get_time_of_next_flash() - timestamp;
	if (secs_to_next_flash > 0xff) {
		secs_to_next_flash = 0xff;
	}
	write_bytes_and_shift(cur_data_buf, &buf_index,	&secs_to_next_flash,	1);

	uint8_t reboot_count = cache_get_reboot_count();
	write_bytes_and_shift(cur_data_buf, &buf_index,	&reboot_count,			1);

	read_lion_volts_batch((uint8_t*) (cur_data_buf + buf_index));
	buf_index += sizeof(lion_volts_batch);
	
	// the next couple readings use IR power on, so turn it on for them
	// to speed up the readings (otherwise they have to wait for it to come on)
	bool got_semaphore = enable_ir_pow_if_necessary();
	{
		// we read all battery board inputs at once,
		// but we write to two different message buffer locations, so we
		// shift past the lion currents and lion temp to the ad7991_ctrlbrd_batch location
		read_ad7991_batbrd((uint8_t*) (cur_data_buf + buf_index),
		(uint8_t*) (cur_data_buf + buf_index + sizeof(lion_current_batch) + sizeof(lion_temps_batch)));
		buf_index += sizeof(lion_current_batch);

		en_and_read_lion_temps_batch((uint8_t*) (cur_data_buf + buf_index));
		buf_index += sizeof(lion_temps_batch);
		buf_index += sizeof(panelref_lref_batch); // jump over what we wrote before

		// use temp buffer because writing 2 bytes of data to what's actually a 1-byte
		// pointer results in a segfault :P
		bat_charge_dig_sigs_batch dig_sigs;
		read_bat_charge_dig_sigs_batch(&dig_sigs);
		write_bytes_and_shift(cur_data_buf, &buf_index, &dig_sigs, sizeof(bat_charge_dig_sigs_batch));
	}
	// try to disable IR power because we're done and no one else is using it
	disable_ir_pow_if_necessary(got_semaphore);

	read_lifepo_volts_batch((uint8_t*) (cur_data_buf + buf_index));
}

// forward declarations
void write_preamble(uint8_t* buffer, uint8_t* buf_index, uint32_t timestamp, uint8_t states, uint8_t data_len, uint8_t num_errors);
void write_data_section(uint8_t* buffer, uint8_t* buf_index, msg_data_type_t msg_type, int num_data);
void write_errors_unsafe(uint8_t* buffer, uint8_t* buf_index, int count, uint32_t timestamp);
void write_parity(uint8_t* buffer, uint8_t* buf_index);

void write_packet(uint8_t* msg_buffer, msg_data_type_t msg_type, uint32_t current_timestamp, const uint8_t* cur_data_buf) {

	uint8_t num_data, size_data, num_packet_errors, padding_size;

	bool got_error_stack_mutex = xSemaphoreTake(error_equistack.mutex, EQUISTACK_MUTEX_WAIT_TIME_TICKS);
	if (!got_error_stack_mutex) {
		log_error(ELOC_RADIO, ECODE_EQUISTACK_MUTEX_TIMEOUT, false);
	}

	// determine constants based on message type
	switch(msg_type)
	{
		case IDLE_DATA:
			num_data =			IDLE_DATA_PACKETS;
			size_data =			IDLE_DATA_PACKET_SIZE;
			num_packet_errors = IDLE_DATA_NUM_ERRORS;
			padding_size =		IDLE_DATA_PADDING_SIZE;
			break;

		case ATTITUDE_DATA:
			num_data =			ATTITUDE_DATA_PACKETS;
			size_data =			ATTITUDE_DATA_PACKET_SIZE;
			num_packet_errors = ATTITUDE_DATA_NUM_ERRORS;
			padding_size =		ATTITUDE_DATA_PADDING_SIZE;
			break;

		case FLASH_DATA:
			num_data =			FLASH_DATA_PACKETS;
			size_data =			FLASH_DATA_PACKET_SIZE;
			num_packet_errors = FLASH_DATA_NUM_ERRORS;
			padding_size =		FLASH_DATA_PADDING_SIZE;
			break;

		case FLASH_CMP_DATA:
			num_data =			FLASH_CMP_DATA_PACKETS;
			size_data =			FLASH_CMP_DATA_PACKET_SIZE;
			num_packet_errors = FLASH_CMP_DATA_NUM_ERRORS;
			padding_size =		FLASH_CMP_DATA_PADDING_SIZE;
			break;

		case LOW_POWER_DATA:
			num_data =			LOW_POWER_DATA_PACKETS;
			size_data =			LOW_POWER_DATA_PACKET_SIZE;
			num_packet_errors = LOW_POWER_DATA_NUM_ERRORS;
			padding_size =		LOW_POWER_DATA_PADDING_SIZE;
			break;

		default:
			// this is a problem
			configASSERT(false);
	};

	/* write sections of message */
	
	// clear buffer
	memset(msg_buffer, 0, MSG_BUFFER_SIZE);

	// configure state string
	uint8_t state_string = 0;
	state_string |=  msg_type			& 0b111;			// 3 LSB of msg_type (5 types)
	state_string |= (get_sat_state()	& 0b111)	<< 3;	// three LSB of satellite state
	state_string |= (flash_killed & 0x1)			<< 6;	// whether flash is currently killed
	state_string |= (cache_get_prog_mem_rewritten() & 0x1) << 7;	// whether program mem rewritten on last reboot
	
	// incremented index in buffer
	uint8_t buf_index = 0;

	// number of errors here is totoal number of errors currently in stack
	write_preamble(msg_buffer, &buf_index, current_timestamp, state_string, num_data * size_data, error_equistack.cur_size);
	configASSERT(buf_index == START_CUR_DATA);

	// copy current data buffer to this message buffer (it should've been written using a call to read_current_data)
	memcpy(msg_buffer + buf_index, cur_data_buf, MSG_CUR_DATA_LEN);
	buf_index = START_DATA;

	write_data_section(msg_buffer, &buf_index, msg_type, num_data);
	// note that the start of errors/padding is determined dynamically by buf_index
	configASSERT(buf_index == START_DATA + size_data*num_data);
	configASSERT (buf_index < START_PARITY);

	// we have the mutex, so write errors with it!
	write_errors_unsafe(msg_buffer, &buf_index, num_packet_errors, current_timestamp);
	configASSERT(buf_index == START_DATA + size_data*num_data + num_packet_errors*ERROR_PACKET_SIZE);
	configASSERT (buf_index <= START_PARITY);
	
	if (got_error_stack_mutex) xSemaphoreGive(error_equistack.mutex);

	write_value_and_shift(msg_buffer, &buf_index, 0, padding_size);
	configASSERT(buf_index == START_PARITY);

	write_parity(msg_buffer, &buf_index);
	configASSERT(buf_index == MSG_SIZE);
}

/************************************************************************/
/* Message section helpers												*/
/************************************************************************/

/**
 * Writes the given data to the preamble of the message buffer.
 */
void write_preamble(uint8_t* buffer, uint8_t* buf_index, uint32_t timestamp, uint8_t states, uint8_t data_len, uint8_t num_errors) {
	*buf_index = START_PREAMBLE; // to be sure

	// write callsign
	buffer[(*buf_index)++] = 'W';
	buffer[(*buf_index)++] = 'L';
	buffer[(*buf_index)++] = '9';
	buffer[(*buf_index)++] = 'X';
	buffer[(*buf_index)++] = 'Z';
	buffer[(*buf_index)++] = 'E';

	write_bytes_and_shift(buffer, buf_index,	&timestamp,			sizeof(timestamp)); // 4 byte timestamp
	write_bytes_and_shift(buffer, buf_index,	&states,			sizeof(states)); // 1 byte state string
	write_bytes_and_shift(buffer, buf_index,	&data_len,			sizeof(data_len)); // 1 byte data packet size
	write_bytes_and_shift(buffer, buf_index,	&num_errors,		sizeof(num_errors)); // 1 byte number of errors
}

static void write_error(uint8_t* buffer, uint8_t* buf_index, sat_error_t* err, uint32_t timestamp) {
	// we have to fill up the error section, so either write the error or NULL bytes
	// (it should be caught above but be extra-safe)
	if (err != NULL) {
		// we calculate an offset timestamp for each error (relative to the current packet)
		// this saves space, but we need to reduce the resolution to ERROR_TIME_BUCKET_SIZE
		uint32_t time_offset = (timestamp - err->timestamp) / ERROR_TIME_BUCKET_SIZE;
		// if the time offset is too large, note this as the max time offset (0xff = 1111 1111)
		// otherwise, it could be any value the last 8 bits.
		if (time_offset > 0xff) {
			time_offset = 0xff;
		}
		write_bytes_and_shift(buffer, buf_index,	&(err->ecode),		1); // includes priority bit
		write_bytes_and_shift(buffer, buf_index,	&(err->eloc),		1);
		write_bytes_and_shift(buffer, buf_index,	&time_offset,		1);
	} else {
		write_value_and_shift(buffer, buf_index,	0,	ERROR_PACKET_SIZE);
	}
}

// make sure not to call this function (i.e. write_*_packet) more than once or it will
// re-iterate over the errors!
void write_errors_unsafe(uint8_t* buffer, uint8_t* buf_index, int count, uint32_t timestamp) {
	// when writing errors, iterate through the error equistack across transmissions, 
	// so each subsequent transmission is farther down the stack (until it wraps around)
	// NOTE: Because of RTOS, the size of the stack may change at any point here, but by the nature
	// of Equistacks, it will only INCREASE, so we don't need to worry about it here (but we do anyways) 
	static int error_index = 0;

	for (int errors_written = 0; errors_written < count; errors_written++) {
		configASSERT(error_index >= 0);
		
		// wrap around if necessary
		if (error_index >= error_equistack.cur_size) {
			error_index = 0;
		}
		
		if (errors_written < error_equistack.cur_size) {
			sat_error_t* err = (sat_error_t*) equistack_Get_Unsafe(&error_equistack, error_index);
			write_error(buffer, buf_index, err, timestamp);
			error_index++;
		} else {
			write_value_and_shift(buffer, buf_index, 0, ERROR_PACKET_SIZE);
		}
	}
}

void write_idle_data_packet(uint8_t* buffer, uint8_t* buf_index, idle_data_t* idle_data);
void write_attitude_data_packet(uint8_t* buffer, uint8_t* buf_index, attitude_data_t* attitude_data);
void write_flash_data_packet(uint8_t* buffer, uint8_t* buf_index, flash_data_t* flash_data);
void write_flash_cmp_data_packet(uint8_t* buffer, uint8_t* buf_index, flash_cmp_data_t* flash_cmp_data);
void write_low_power_data_packet(uint8_t* buffer, uint8_t* buf_index, low_power_data_t* low_power_data);

/* writes the data section corresponding to msg_type, and returns the end of this (the start of the error section).
   The packet equistack associated with msg_type should not be empty. 
   NOTE: this function's thread safety depends on the transmit task being higher priority 
   than the data reading tasks (which it is) */
void write_data_section(uint8_t* buffer, uint8_t* buf_index, msg_data_type_t msg_type, int num_data) {
	*buf_index = START_DATA; // to be certain

	// note: _PACKETS should be less than _STACK_MAX, or we're wasting space!
	int packets_written = 0;
	int equi_i = 0; // equistack iterator
	bool retransmit = false;
	while (packets_written < num_data) {
		// write a packet according to message type, noting whether the particular packet
		// was transmittable and its size
		bool transmittable = false;
		int equistack_size;
		size_t equistack_data_size;

		// for each type, grab the current index (note that msg_type is constant),
		// and write a packet and note it "transmitted" if we're either retransmitting right now,
		// or the current packet is not transmitted otherwise.
		// Also, skip any NULL packets (empty check is below)
		// Also note the above variables
		switch (msg_type) {
			case IDLE_DATA: ;
				idle_data_t* idle_data = (idle_data_t*) equistack_Get(&idle_readings_equistack, equi_i);
				if (idle_data != NULL && (retransmit || !idle_data->transmitted)) {
					write_idle_data_packet(buffer, buf_index, idle_data);
					idle_data->transmitted = true;
					transmittable = true;
				}
				equistack_size = idle_readings_equistack.cur_size;
				equistack_data_size = IDLE_DATA_PACKET_SIZE;
				break;

			case ATTITUDE_DATA: ;
				attitude_data_t* attitude_data = (attitude_data_t*) equistack_Get(&attitude_readings_equistack, equi_i);
				if (attitude_data != NULL && (retransmit || !attitude_data->transmitted)) {
					write_attitude_data_packet(buffer, buf_index, attitude_data);
					attitude_data->transmitted = true;
					transmittable = true;
				}
				equistack_size = attitude_readings_equistack.cur_size;
				equistack_data_size = ATTITUDE_DATA_PACKET_SIZE;
				break;

			case FLASH_DATA: ;
				flash_data_t* flash_data = (flash_data_t*) equistack_Get(&flash_readings_equistack, equi_i);
				if (flash_data != NULL && (retransmit || !flash_data->transmitted)) {
					write_flash_data_packet(buffer, buf_index, flash_data);
					flash_data->transmitted = true;
					transmittable = true;
				}
				equistack_size = flash_readings_equistack.cur_size;
				equistack_data_size = FLASH_DATA_PACKET_SIZE;
				break;

			case FLASH_CMP_DATA: ;
				flash_cmp_data_t* flash_cmp_data = (flash_cmp_data_t*) equistack_Get(&flash_cmp_readings_equistack, equi_i);
				if (flash_cmp_data != NULL && (retransmit || !flash_cmp_data->transmitted)) {
					write_flash_cmp_data_packet(buffer, buf_index, flash_cmp_data);
					flash_cmp_data->transmitted = true;
					transmittable = true;
				}
				equistack_size = flash_cmp_readings_equistack.cur_size;
				equistack_data_size = FLASH_CMP_DATA_PACKET_SIZE;
				break;

			case LOW_POWER_DATA: ;
				low_power_data_t* low_power_data = (low_power_data_t*) equistack_Get(&low_power_readings_equistack, equi_i);
				if (low_power_data != NULL && (retransmit || !low_power_data->transmitted)) {
					write_low_power_data_packet(buffer, buf_index, low_power_data);
					low_power_data->transmitted = true;
					transmittable = true;
				}
				equistack_size = low_power_readings_equistack.cur_size;
				equistack_data_size = LOW_POWER_DATA_PACKET_SIZE;
				break;

			default:
				configASSERT(false);
		}

		// check for completely empty equistack
		// (write out zeros and continue because otherwise we'll infinite loop)
		if (equistack_size == 0) {
			write_value_and_shift(buffer, buf_index, 0, equistack_data_size);
			packets_written++;
			continue;
		}

		if (!retransmit && !transmittable) {
			// if we're not retransmitting and it was already transmitted,
			// skip it (don't note a write)
			equi_i++;
		} else {
			// move along the equistack, noting (above) that this packet will be transmitted
			packets_written++;
			equi_i++;
		}

		if (equi_i >= equistack_size) {
			// if we reach the end of the list, it means there are either not enough
			// packets in the equistack to fill a message, or there were too many already-transmitted packets (for the first time).
			// In this case, stop caring about whether they were transmitted, and wrap back around to try again
			equi_i = 0;
			retransmit = true;

			// if this has happened twice or more (we went through not skipping transmitted packets),
			// then keep looping around the equistack and re-writing until we write all we need
		}
	}
}

/************************************************************************/
/* Functions to translate packet types to corresponding message format  */
/************************************************************************/
void write_idle_data_packet(uint8_t* buffer, uint8_t* buf_index, idle_data_t* idle_data) {
	write_bytes_and_shift(buffer, buf_index,	&(idle_data->satellite_history),		sizeof(satellite_history_batch));
	write_bytes_and_shift(buffer, buf_index,	idle_data->lion_volts_data,				sizeof(lion_volts_batch));
	write_bytes_and_shift(buffer, buf_index,	idle_data->lion_current_data,			sizeof(lion_current_batch));
	write_bytes_and_shift(buffer, buf_index,	idle_data->lion_temps_data,				sizeof(lion_temps_batch));
	write_bytes_and_shift(buffer, buf_index,	idle_data->panelref_lref_data,			sizeof(panelref_lref_batch));
	write_bytes_and_shift(buffer, buf_index,	&(idle_data->bat_charge_dig_sigs_data),	sizeof(bat_charge_dig_sigs_batch));
	write_bytes_and_shift(buffer, buf_index,	&(idle_data->radio_temp_data),			sizeof(radio_temp_batch));
	write_bytes_and_shift(buffer, buf_index,	&(idle_data->imu_temp_data),			sizeof(imu_temp_batch));
	write_bytes_and_shift(buffer, buf_index,	idle_data->ir_amb_temps_data,			sizeof(ir_ambient_temps_batch));
	write_bytes_and_shift(buffer, buf_index,	&(idle_data->timestamp),				4 /* uint32_t */);
}

void write_attitude_data_packet(uint8_t* buffer, uint8_t* buf_index, attitude_data_t* attitude_data) {
	write_bytes_and_shift(buffer, buf_index,	attitude_data->ir_obj_temps_data,	sizeof(ir_object_temps_batch)	/* [1] */);
	write_bytes_and_shift(buffer, buf_index,	&(attitude_data->pdiode_data),		sizeof(pdiode_batch)			/* [1] */);
	write_bytes_and_shift(buffer, buf_index,	attitude_data->accelerometer_data,	sizeof(accelerometer_batch) * 2 /* [2] */);
	write_bytes_and_shift(buffer, buf_index,	attitude_data->gyro_data,			sizeof(gyro_batch)				/* [1] */);
	write_bytes_and_shift(buffer, buf_index,	attitude_data->magnetometer_data,	sizeof(magnetometer_batch) * 2	/* [2] */);
	write_bytes_and_shift(buffer, buf_index,	&attitude_data->timestamp,			4 /* uint32_t */);
}

void write_flash_data_packet(uint8_t* buffer, uint8_t* buf_index, flash_data_t* flash_data) {
	write_bytes_and_shift(buffer, buf_index,	flash_data->led_temps_data,			sizeof(led_temps_batch)			* FLASH_DATA_ARR_LEN /* [7] */);
	write_bytes_and_shift(buffer, buf_index,	flash_data->lifepo_bank_temps_data,	sizeof(lifepo_bank_temps_batch)	* FLASH_DATA_ARR_LEN /* [7] */);
	write_bytes_and_shift(buffer, buf_index,	flash_data->lifepo_current_data,	sizeof(lifepo_current_batch)	* FLASH_DATA_ARR_LEN /* [7] */);
	write_bytes_and_shift(buffer, buf_index,	flash_data->lifepo_volts_data,		sizeof(lifepo_volts_batch)		* FLASH_DATA_ARR_LEN /* [7] */);
	write_bytes_and_shift(buffer, buf_index,	flash_data->led_current_data,		sizeof(led_current_batch)		* FLASH_DATA_ARR_LEN /* [7] */);
	write_bytes_and_shift(buffer, buf_index,	flash_data->gyro_data,				sizeof(gyro_batch)				* FLASH_DATA_ARR_LEN /* [7] */);
	write_bytes_and_shift(buffer, buf_index,	&flash_data->timestamp,				4 /* uint_32_t */);
}

void write_flash_cmp_data_packet(uint8_t* buffer, uint8_t* buf_index, flash_cmp_data_t* flash_cmp_data) {
	/* NOTE: Though these are the same types as in flash_data_t, they are those values AVERAGED */
	write_bytes_and_shift(buffer, buf_index,	flash_cmp_data->led_temps_avg_data,			sizeof(led_temps_batch)			/* [1] */);
	write_bytes_and_shift(buffer, buf_index,	flash_cmp_data->lifepo_bank_temps_avg_data,	sizeof(lifepo_bank_temps_batch)	/* [1] */);
	write_bytes_and_shift(buffer, buf_index,	flash_cmp_data->lifepo_current_avg_data,	sizeof(lifepo_current_batch)	/* [1] */);
	write_bytes_and_shift(buffer, buf_index,	flash_cmp_data->lifepo_volts_avg_data,		sizeof(lifepo_volts_batch)		/* [1] */);
	write_bytes_and_shift(buffer, buf_index,	flash_cmp_data->led_current_avg_data,		sizeof(led_current_batch)		/* [1] */);
	write_bytes_and_shift(buffer, buf_index,	flash_cmp_data->mag_before_data,			sizeof(magnetometer_batch)		/* [1] */);
	write_bytes_and_shift(buffer, buf_index,	&flash_cmp_data->timestamp,					4 /* uint_32_t */);
}

void write_low_power_data_packet(uint8_t* buffer, uint8_t* buf_index, low_power_data_t* low_power_data) {
	write_bytes_and_shift(buffer, buf_index,	&(low_power_data->satellite_history),			sizeof(satellite_history_batch));
	write_bytes_and_shift(buffer, buf_index,	low_power_data->lion_volts_data,				sizeof(lion_volts_batch));
	write_bytes_and_shift(buffer, buf_index,	low_power_data->lion_current_data,				sizeof(lion_current_batch));
	write_bytes_and_shift(buffer, buf_index,	low_power_data->lion_temps_data,				sizeof(lion_temps_batch));
	write_bytes_and_shift(buffer, buf_index,	low_power_data->panelref_lref_data,				sizeof(panelref_lref_batch));
	write_bytes_and_shift(buffer, buf_index,	&(low_power_data->bat_charge_dig_sigs_data),	sizeof(bat_charge_dig_sigs_batch));
	write_bytes_and_shift(buffer, buf_index,	low_power_data->ir_obj_temps_data,				sizeof(ir_object_temps_batch));
	write_bytes_and_shift(buffer, buf_index,	low_power_data->gyro_data,						sizeof(gyro_batch));
	write_bytes_and_shift(buffer, buf_index,	&(low_power_data->timestamp),					4 /* uint32_t */);
}

/* writes error correction bytes. Must be called after full message before it was written, obviously. */
void write_parity(uint8_t* buffer, uint8_t* buf_index) {
	// encode using Reed-Solomon (START_PARITY is the number of bytes in buffer before parity section)
	// NOTE we don't encode the callsign
	#ifdef USE_REED_SOLOMON
		encode_data(buffer + CALLSIGN_SIZE, START_PARITY - CALLSIGN_SIZE, buffer + CALLSIGN_SIZE);
	#endif
	*buf_index = *buf_index + MSG_PARITY_LENGTH;
}

/* Writes num_bytes from input to data, and shifts the value at buf_index up by num_bytes */
void write_bytes_and_shift(uint8_t *data, uint8_t* buf_index, void *input, size_t num_bytes) {
	memcpy(data + *buf_index, (char*) input, num_bytes);
	*buf_index += num_bytes;
}

/* Writes num_bytes copies of value to data, and shifts the value at buf_index up by num_bytes */
void write_value_and_shift(uint8_t *data, uint8_t* buf_index, char value, size_t num_bytes) {
	for (uint i = 0; i < num_bytes; i++) {
		data[*buf_index] = value;
		(*buf_index)++;
	}
}
