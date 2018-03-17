#ifndef SYSTEM_TEST_H_
#define SYSTEM_TEST_H_

#include "../global.h"
#include "../testing_functions/rtos_system_test.h"
#include "../sensor_drivers/sensor_read_commands.h"

void tca_shifts_test(void);
void sensor_read_tests(void);
void system_test(bool printFloats); 
void lf_sns_test(void);

#endif
