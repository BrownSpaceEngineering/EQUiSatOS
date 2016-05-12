/*
 * IncFile1.h
 *
 * Created: 4/26/2015 3:34:57 PM
 *  Author: Daniel
 */ 


#ifndef I2C_MOCK_H
#define I2C_MOCK_H

#include <asf.h>
// #include <I2C_Commands.h>

void set_i2c_mock_test_case(const struct test_case *_test);
void cleanup_i2c_mock_test_case(void);

uint8_t* get_written_bytes(void);

void configure_i2c_master_mock(void);
void i2c_write_mock(struct i2c_master_packet* packet_address);
void i2c_write_mock_no_stop(struct i2c_master_packet* packet_address);
void i2c_read_mock(struct i2c_master_packet* packet_address);
void i2c_mock_set_next_read_bytes(uint8_t* _next_read_bytes);
int get_no_stop_call_count(void);

#endif /* I2C_MOCK_H */