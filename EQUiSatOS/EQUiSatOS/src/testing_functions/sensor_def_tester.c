/*
 * sensor_def_tester.c
 *
 * Created: 3/12/18 12:27:58 AM
 *  Author: jleiken
 */ 

#include "sensor_def_tester.h"

void test_truncation(void) {
	uint8_t res = truncate_16t(150, S_GYRO);
	assert(res == 128);
	res = truncate_16t(16492, S_ACCEL);
	assert(res == 192);
	res = truncate_16t(-204, S_MAG);
	assert(res == 111);
	res = truncate_16t(953, S_L_SNS);
	assert(res == 86);
	res = truncate_16t(51967, S_LED_SNS);
	assert(res == 202);
	res = truncate_16t(3700, S_LF_VOLT);
	assert(res == 202);
	res = truncate_16t(979, S_LF_SNS_REG);
	assert(res == 3);
	res = truncate_16t(2175, S_LF_SNS_FLASH);
	assert(res == 232);
	res = truncate_16t(10, S_LF_OSNS_FLASH);
	assert(res == 2);
	res = truncate_16t(402, S_LF_OSNS_REG);
	assert(res == 111);
	res = truncate_16t(3665, S_LREF);
	assert(res == 200);
	res = truncate_16t(13000, S_IR_AMB);
	assert(res == 36);
}