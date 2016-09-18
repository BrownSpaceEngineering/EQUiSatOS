#ifndef HMC5883L_MAGNETOMETER_COMMANDS_TESTS_H
#define HMC5883L_MAGNETOMETER_COMMANDS_TESTS_H

#include <HMC5883L_Magnetometer_Commands.h>
#include "I2C_Mock.h"

void run_test_HMC5883L_init(const struct test_case *test);
void run_test_HMC5883L_read(const struct test_case *test);
void run_test_HMC5883L_getXYZ(const struct test_case *test);
void run_test_HMC5883L_computeCompassDir(const struct test_case *test);

void run_HMC5883L_tests(void);

#endif /* HMC5883L_MAGNETOMETER_COMMANDS_TESTS_H */