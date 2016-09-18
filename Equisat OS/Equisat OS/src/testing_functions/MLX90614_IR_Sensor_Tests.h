#ifndef MLX90614_IR_SENSOR_TESTS_H
#define MLX90614_IR_SENSOR_TESTS_H

#include <MLX90614_IR_Sensor.h>
#include "I2C_Mock.h"

void run_test_MLX90614_init(const struct test_case *test);
void run_test_MLX90614_read(const struct test_case *test);
void run_test_read_MLX90614(const struct test_case *test);
void run_test_MLX90614_read2ByteValue(const struct test_case *test);
void run_test_MLX90614_readRawIRData(const struct test_case *test);
void run_test_MLX90614_dataToTemp(const struct test_case *test);
void run_test_MLX90614_readTempC(const struct test_case *test);
void run_test_MLX90614_getAddress(const struct test_case *test);
void run_test_write_MLX90614_eeprom(const struct test_case *test);
void run_test_MLX90614_setAddress(const struct test_case *test);

void run_MLX90614_tests(void);

#endif /* MLX90614_IR_SENSOR_TESTS_H */