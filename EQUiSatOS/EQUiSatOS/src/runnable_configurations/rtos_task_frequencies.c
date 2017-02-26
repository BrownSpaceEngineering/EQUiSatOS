/*
 * rtos_task_frequencies.c
 *
 * Responsible for making sure that the constants defined in rtos_task_frequencies.h are internally consistent.
 *
 * Created: 2/26/2017 2:57:52 PM
 *  Author: mckenna
 */ 

#include "rtos_task_frequencies.h"

void assertConstantDefinitions() {
	// The MOST reads per log corresponding to the LOWEST log frequency, 
	// and the lowest log frequency should have a list of size 1 
	// (or possibly some other value if we want to store more data in each struct), 
	// and the other, more frequent tasks should have larger lists and thus fewer reads per log
	// In fact, the ratio of the max reads per log to each sensor's reads per log 
	// should equal the ratio of their data array lengths.
	// EX. If we're creating lists where the lowest frequency data list
	// should be of size one, their lengths should be:
	// [state]_max_reads_per_log / [task]_[sensor]_READS_PER_LOG
	// will result in the lowest frequency having a size of 1
	// (multiply this by something to not a change in the smallest size;
	// i.e. x2 means the lowest frequency sensor has an array of size 2)
	// So, we should check that this property holds for all array lengths and reads_per_log of the sensors:
	uint16_t idle_max_reads_per_log = 0; // TODO: use arrMax();
	assert(idle_IR_DATA_ARR_LEN == idle_IR_READS_PER_LOG / idle_max_reads_per_log);
	
	// We also want to make sure that any sensors' reads per log is greater than zero,
	// i.e. that the log frequency is greater than the read frequency
	// (because we can't log faster than we're even reading)
	assert(idle_IR_READS_PER_LOG > 0);
	
}

int arrMax(int* arr, uint8_t len) {
	int maxVal = arr[0];
	for (uint8_t i = 1; i < len; i++) {
		if (arr[i] > maxVal) {
			maxVal = arr[i];
		}
	}
	return maxVal;
}