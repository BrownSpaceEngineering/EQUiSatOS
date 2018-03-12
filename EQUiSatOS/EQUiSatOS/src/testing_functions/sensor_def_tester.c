/*
 * sensor_def_tester.c
 *
 * Created: 3/12/18 12:27:58 AM
 *  Author: jleiken
 */ 

#include "sensor_def_tester.h"

void test_truncation(void) {
	assert(truncate_16t(150, S_GYRO) == 128);
	assert(truncate_16t(16492, S_ACCEL) == 192);
	assert(truncate_16t(-204, S_MAG) == 10);
	assert(truncate_16t(953, S_L_SNS) == 86);
	assert(truncate_16t(51967, S_LED_SNS) == 202);
	assert(truncate_16t(3700, S_LF_VOLT) == 208);
	assert(truncate_16t(979, S_LF_SNS_REG) == 3);
	assert(truncate_16t(2175, S_LF_SNS_FLASH) == 233);
	assert(truncate_16t(10, S_LF_OSNS_FLASH) == 2);
	assert(truncate_16t(402, S_LF_OSNS_REG) == 112);
	assert(truncate_16t(3665, S_LREF) == 206);
	assert(truncate_16t(13000, S_IR_AMB) == 41);
}