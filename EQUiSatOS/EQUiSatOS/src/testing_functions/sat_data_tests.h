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
void test_message_packaging(void);

#endif /* MESSAGE_PACKAGING_TESTS_H_ */