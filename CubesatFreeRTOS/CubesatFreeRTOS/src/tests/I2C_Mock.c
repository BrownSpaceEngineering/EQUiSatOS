#include "I2C_Mock.h"

// define here
int no_stop_calls = 0;
const struct test_case *i2c_mock_test = NULL;
uint8_t* i2c_mock_next_read_bytes = NULL;
uint8_t* i2c_mock_written_bytes = NULL;
int written_bytes_len = 0;
uint8_t i2c_mock_read_addr;
uint8_t i2c_mock_write_addr;

void set_i2c_mock_test_case(const struct test_case *_test) {
	i2c_mock_test = _test;
}

void cleanup_i2c_mock_test_case(void) {
	free(i2c_mock_written_bytes);
	written_bytes_len = 0;
	no_stop_calls = 0;
}

void i2c_write_mock(struct i2c_master_packet* packet_address) {	
	int old_written_bytes_len = written_bytes_len;
	written_bytes_len += packet_address->data_length;
	
	uint8_t * new_written_bytes = (uint8_t *)malloc(written_bytes_len);
	
	// set each element of the new buffer
	for(int i=0; i<old_written_bytes_len; i++) {
		new_written_bytes[i] = i2c_mock_written_bytes[i];
	}
	for(int i=old_written_bytes_len; i<written_bytes_len; i++) {
		new_written_bytes[i] = packet_address->data[i];
	}
	free(i2c_mock_written_bytes);
	i2c_mock_written_bytes = new_written_bytes;
	
	i2c_mock_write_addr = packet_address->address;
}


void i2c_write_mock_no_stop(struct i2c_master_packet* packet_address) {
	no_stop_calls++;
	i2c_write_mock(packet_address);
}

void i2c_read_mock(struct i2c_master_packet* packet_address) {
	packet_address->data = i2c_mock_next_read_bytes;
	i2c_mock_read_addr = packet_address->address;
}

void i2c_mock_set_next_read_bytes(uint8_t* _next_read_bytes) {
	i2c_mock_next_read_bytes = _next_read_bytes;
}