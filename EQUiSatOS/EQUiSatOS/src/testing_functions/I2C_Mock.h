#ifndef I2C_MOCK_H
#define I2C_MOCK_H

#include <global.h>

// declare here (may want to use getters instead?)
extern int no_stop_calls;
extern const struct test_case *i2c_mock_test;
extern uint8_t* i2c_mock_next_read_bytes;
extern uint8_t* i2c_mock_written_bytes;
extern int written_bytes_len;
extern uint8_t i2c_mock_read_addr;
extern uint8_t i2c_mock_write_addr;

void set_i2c_mock_test_case(const struct test_case *_test);
void cleanup_i2c_mock_test_case(void);

void configure_i2c_master_mock(void);
void i2c_write_mock(struct i2c_master_packet* packet_address);
void i2c_write_mock_no_stop(struct i2c_master_packet* packet_address);
void i2c_read_mock(struct i2c_master_packet* packet_address);
void i2c_mock_set_next_read_bytes(uint8_t* _next_read_bytes);

#endif /* I2C_MOCK_H */