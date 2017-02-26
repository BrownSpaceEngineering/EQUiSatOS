/*
 * struct_tests.h
 *
 * Created: 11/13/2016 12:01:43 PM
 *  Author: jleiken
 */ 


#ifndef STRUCT_TESTS_H_
#define STRUCT_TESTS_H_

#include <asf.h>
#include "../stacks/State_Structs.h"
#include "../stacks/Sensor_Structs.h"

idle_data_t init_test_struct(void);
void test_free(void *pvParameters);

#endif /* STRUCT_TESTS_H_ */