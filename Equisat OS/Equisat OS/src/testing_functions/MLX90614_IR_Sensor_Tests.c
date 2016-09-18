#include "MLX90614_IR_Sensor_Tests.h"

float MLX90614_float_comparison_epsilon = 0.0000001;

void run_test_MLX90614_init(const struct test_case *test) {
	set_i2c_mock_test_case(test);
	MLX90614_init(&i2c_write_mock, &i2c_write_mock_no_stop, &i2c_read_mock);
	cleanup_i2c_mock_test_case();
}

void run_test_MLX90614_read(const struct test_case *test) {
	run_test_MLX90614_init(test);
	uint8_t read_values[6] = {
		0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
	};
	i2c_mock_set_next_read_bytes(read_values);
	uint8_t read_buffer[6] = {0, 0, 0, 0, 0, 0};
	uint8_t device_addr = 0xAA;
	uint8_t mem_addr = 0xBB;
	read_MLX90614(device_addr, mem_addr, read_buffer);
	for(int i=0; i<6; i++) {
		test_assert_true(test, read_buffer[i] == read_values[i], "MLX90614 read byte incorrect.");
	}
	test_assert_true(test, i2c_mock_written_bytes[0] == mem_addr, "MLX90614 memory address byte incorrect.");
	test_assert_true(test, i2c_mock_write_addr == device_addr, "MLX90614 write address byte incorrect.");
	test_assert_true(test, i2c_mock_read_addr == device_addr, "MLX90614 read address byte incorrect.");
	test_assert_true(test, no_stop_calls == 1, "MLX90614 did not use no stop write.");
	cleanup_i2c_mock_test_case();
}

void run_test_write_MLX90614_eeprom(const struct test_case *test) {
	run_test_MLX90614_init(test);
	uint8_t write_values[2] = {
		0x09, 0x0A
	};
	uint8_t device_addr = 0x77;
	uint8_t mem_addr = 0x88;
	write_MLX90614_eeprom(device_addr, mem_addr, write_values);
	test_assert_true(test, i2c_mock_written_bytes[0] == mem_addr, "MLX90614 memory address byte incorrect.");
	test_assert_true(test, i2c_mock_written_bytes[1] == write_values[0], "MLX90614 first written byte incorrect.");
	test_assert_true(test, i2c_mock_written_bytes[2] == write_values[1], "MLX90614 second written byte incorrect.");
	//test_assert_true(test, i2c_mock_written_bytes[3] == PUT CRC VAL HERE WHEN FIX, "MLX90614 third written byte incorrect.");
	test_assert_true(test, i2c_mock_write_addr == device_addr, "MLX90614 write address byte incorrect.");
	cleanup_i2c_mock_test_case();
}

void run_test_MLX90614_read2ByteValue(const struct test_case *test) {
	run_test_MLX90614_init(test);
	uint8_t read_values[6] = {
		0x06, 0x07
	};
	i2c_mock_set_next_read_bytes(read_values);
	uint8_t device_addr = 0x77;
	uint8_t mem_addr = 0x88;
	uint16_t result = MLX90614_read2ByteValue(device_addr, mem_addr, 0);
	test_assert_true(test, i2c_mock_written_bytes[0] == mem_addr, "MLX90614 memory address byte incorrect.");
	test_assert_true(test, 0x0706 == result, "MLX90614 two byte lsb first read value incorrect.");
	cleanup_i2c_mock_test_case();

	result = MLX90614_read2ByteValue(device_addr, mem_addr, 1);
	test_assert_true(test, i2c_mock_written_bytes[0] == mem_addr, "MLX90614 memory address byte incorrect.");
	test_assert_true(test, 0x0607 == result, "MLX90614 two byte lsb last read value incorrect.");
	cleanup_i2c_mock_test_case();
}

void run_test_MLX90614_readRawIRData(const struct test_case *test) {
	run_test_MLX90614_init(test);
	uint8_t device_addr = 0x85;
	MLX90614_readRawIRData(device_addr, 0);
	test_assert_true(test, i2c_mock_written_bytes[0] == 0x04, "MLX90614 memory address byte incorrect for IR1.");
	cleanup_i2c_mock_test_case();

	MLX90614_readRawIRData(device_addr, 1);
	test_assert_true(test, i2c_mock_written_bytes[0] == 0x05, "MLX90614 memory address byte incorrect for IR2.");
	cleanup_i2c_mock_test_case();
}

void run_test_MLX90614_dataToTemp(const struct test_case *test) {
	test_assert_true(test, abs(dataToTemp(0x0096) - -270.15) < MLX90614_float_comparison_epsilon, "MLX90614 data to temp conversion incorrect.");
}

void run_test_MLX90614_readTempC(const struct test_case *test) {
	run_test_MLX90614_init(test);
	uint8_t read_values[6] = {
		0x01, 0x02
	};
	i2c_mock_set_next_read_bytes(read_values);
	uint8_t device_addr = 0x79;
	test_assert_true(test, abs(MLX90614_readTempC(device_addr, 0) - -262.89) < MLX90614_float_comparison_epsilon, "MLX90614 data to temp conversion incorrect for reading Celsius temperature."); // data to temp of 0x0201
	test_assert_true(test, i2c_mock_written_bytes[0] == 0x07, "MLX90614 memory address byte incorrect for ambient.");
	cleanup_i2c_mock_test_case();

	MLX90614_readTempC(device_addr, 1);
	test_assert_true(test, abs(MLX90614_readTempC(device_addr, 0) - -262.89) < MLX90614_float_comparison_epsilon, "MLX90614 data to temp conversion incorrect for reading Celsius temperature."); // data to temp of 0x0201
	test_assert_true(test, i2c_mock_written_bytes[0] == 0x06, "MLX90614 memory address byte incorrect for non-ambient.");
	cleanup_i2c_mock_test_case();
}

void run_test_MLX90614_getAddress(const struct test_case *test) {
	// MLX90614_getAddress(uint8_t device_addr)
	// runs MLX90614_read2ByteValue(device_addr, MLX90614_SMBUS, false)
	uint8_t read_values[2] = {
		0x55, 0x66
	};
	i2c_mock_set_next_read_bytes(read_values);
	uint8_t device_addr = 0x33;
	uint16_t addr = MLX90614_getAddress(device_addr);
	test_assert_true(test, i2c_mock_written_bytes[0] == 0x2E, "MLX90614 memory address byte incorrect for device address check.");
	test_assert_true(test, addr == 0x5566, "MLX90614 retrun value incorrect for device address check.");
	cleanup_i2c_mock_test_case();
}

void run_test_MLX90614_setAddress(const struct test_case *test) {
	uint8_t read_values[2] = {
		0x00, 0x22
	};
	uint8_t device_addr = 0x22;
	uint8_t new_addr = 0x78;
	MLX90614_setAddress(device_addr, new_addr);
	test_assert_true(test, i2c_mock_written_bytes[0] == 0x2E, "MLX90614 set address memory address byte incorrect.");
	test_assert_true(test, i2c_mock_written_bytes[1] == 0x2E, "MLX90614 set address memory address byte incorrect.");
	test_assert_true(test, i2c_mock_written_bytes[2] == read_values[0], "MLX90614 set address first written byte incorrect.");
	test_assert_true(test, i2c_mock_written_bytes[3] == read_values[1], "MLX90614 set address second written byte incorrect.");
	//test_assert_true(test, i2c_mock_written_bytes[4] == PUT CRC VAL HERE WHEN FIX, "MLX90614 set address third written byte incorrect.");
	test_assert_true(test, i2c_mock_write_addr == device_addr, "MLX90614 set address write address byte incorrect.");
	cleanup_i2c_mock_test_case();
}

void run_MLX90614_tests(void) {
	/* Define Test Cases */
	DEFINE_TEST_CASE(test_MLX90614_init,
					NULL,
					run_test_MLX90614_init,
					NULL,
					"Testing MLX90614 I2C initialization");
	DEFINE_TEST_CASE(test_MLX90614_read,
					NULL,
					run_test_MLX90614_read,
					NULL,
					"Testing MLX90614 I2C read functionality");
	DEFINE_TEST_CASE(test_MLX90614_read2ByteValue,
					NULL,
					run_test_MLX90614_read2ByteValue,
					NULL,
					"Testing MLX90614 read 2 byte value functionality");
	DEFINE_TEST_CASE(test_MLX90614_readRawIRData,
					NULL,
					run_test_MLX90614_readRawIRData,
					NULL,
					"Testing MLX90614 read raw IR data functionality");
	DEFINE_TEST_CASE(test_MLX90614_dataToTemp,
					NULL,
					run_test_MLX90614_dataToTemp,
					NULL,
					"Testing MLX90614 data to temperature conversion functionality");
	DEFINE_TEST_CASE(test_MLX90614_readTempC,
					NULL,
					run_test_MLX90614_readTempC,
					NULL,
					"Testing MLX90614 Celsius read temperature functionality");
	DEFINE_TEST_CASE(test_MLX90614_getAddress,
					NULL,
					run_test_MLX90614_getAddress,
					NULL,
					"Testing MLX90614 get address functionality");
	DEFINE_TEST_CASE(test_write_MLX90614_eeprom,
					NULL,
					run_test_write_MLX90614_eeprom,
					NULL,
					"Testing MLX90614 EEPROM write functionality");
	DEFINE_TEST_CASE(test_MLX90614_setAddress,
					NULL,
					run_test_MLX90614_setAddress,
					NULL,
					"Testing MLX90614 set address functionality");
	
	/* Put test case addresses in an array */
	DEFINE_TEST_ARRAY(i2c_tests) = {
		&test_MLX90614_init,
		&test_MLX90614_read,
		&test_MLX90614_read2ByteValue,
		&test_MLX90614_readRawIRData,
		&test_MLX90614_dataToTemp,
		&test_MLX90614_readTempC,
		&test_MLX90614_getAddress,
		&test_write_MLX90614_eeprom,
		&test_MLX90614_setAddress
	};

	/* Define the test suite */
	DEFINE_TEST_SUITE(i2c_test_suite, i2c_tests, "MLX90614 test suite");

	/* Run all tests in the suite*/
	test_suite_run(&i2c_test_suite);
}