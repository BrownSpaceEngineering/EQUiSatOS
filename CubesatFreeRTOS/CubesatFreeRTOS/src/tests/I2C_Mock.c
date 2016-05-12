#include "I2C_Mock.h"

int is_configured = 0;

int no_stop_calls = 0;

const struct test_case *test = NULL;

uint8_t* next_read_bytes = NULL;

uint8_t* written_bytes = NULL;
int written_bytes_len = 0;

void set_i2c_mock_test_case(const struct test_case *_test) {
	test = _test;
}

void cleanup_i2c_mock_test_case(void) {
	free(written_bytes);
	written_bytes_len = 0;
	no_stop_calls = 0;
}

void configure_i2c_master_mock(void) {
	is_configured = 1;
}

void i2c_write_mock(struct i2c_master_packet* packet_address) {
	test_assert_true(test, is_configured, "I2C not configured.");
	
	int old_written_bytes_len = written_bytes_len;
	written_bytes_len += packet_address->data_length;
	
	uint8_t * new_written_bytes = (uint8_t *)malloc(written_bytes_len);
	
	// set each element of the new buffer
	for(int i=0; i<old_written_bytes_len; i++) {
		new_written_bytes[i] = written_bytes[i];
	}
	for(int i=old_written_bytes_len; i<written_bytes_len; i++) {
		new_written_bytes[i] = packet_address->data[i];
	}


	free(written_bytes);
	written_bytes = new_written_bytes;
}

uint8_t* get_written_bytes(void) {
	return written_bytes;
}

void i2c_write_mock_no_stop(struct i2c_master_packet* packet_address) {
	test_assert_true(test, is_configured, "I2C not configured.");
	i2c_write_mock(packet_address);
}

void i2c_read_mock(struct i2c_master_packet* packet_address) {
	packet_address->data = next_read_bytes;
}

void i2c_mock_set_next_read_bytes(uint8_t* _next_read_bytes) {
	next_read_bytes = _next_read_bytes;
}

int get_no_stop_call_count(void) {
	return no_stop_calls;
}