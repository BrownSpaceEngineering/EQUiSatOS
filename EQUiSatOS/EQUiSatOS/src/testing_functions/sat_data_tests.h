/*
 * sat_data_tests.h
 *
 * Created: 1/18/2018 23:14:26
 *  Author: mcken
 */ 

#ifndef MESSAGE_PACKAGING_TESTS_H_
#define MESSAGE_PACKAGING_TESTS_H_

#include <global.h>
#include "../data_handling/package_transmission.h"

void populate_equistack(equistack* stack);
void populate_equistacks(void);
void clear_equistacks(void);
void test_message_packaging(void);
void stress_test_message_packaging(void);
void print_sample_transmissions(void);
void print_sample_transmission(uint8_t* msg_buf, msg_data_type_t type, uint32_t current_timestamp, uint8_t* cur_data_buf);

#endif /* MESSAGE_PACKAGING_TESTS_H_ */