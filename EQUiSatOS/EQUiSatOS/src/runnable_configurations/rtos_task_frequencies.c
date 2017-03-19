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
	int all_loops_per_log[] = {
		idle_LION_VOLTS_LOOPS_PER_LOG,
		idle_LION_CURRENT_LOOPS_PER_LOG,
		idle_LED_TEMPS_LOOPS_PER_LOG,
		idle_LIFEPO_CURRENT_LOOPS_PER_LOG,
		idle_IR_LOOPS_PER_LOG,
		idle_DIODE_LOOPS_PER_LOG,
		idle_BAT_TEMP_LOOPS_PER_LOG,
		idle_IR_TEMPS_LOOPS_PER_LOG,
		idle_RADIO_TEMP_LOOPS_PER_LOG,
		idle_IMU_LOOPS_PER_LOG,
		idle_MAGNETOMETER_LOOPS_PER_LOG,
		idle_LED_CURRENT_LOOPS_PER_LOG,
		idle_RADIO_VOLTS_LOOPS_PER_LOG,
		idle_BAT_CHARGE_VOLTS_LOOPS_PER_LOG,
		idle_BAT_CHARGE_DIG_SIGS_LOOPS_PER_LOG,
		idle_DIGITAL_OUT_LOOPS_PER_LOG};
	uint16_t idle_max_loops_per_log = arrMax(all_loops_per_log, 10); // max of the above (MAKE SURE TO UPDATE IF YOU ADD)
	// helpful regex: #define idle_(.*)_DATA_ARR_LEN\s+1--> assert(idle_$1_DATA_ARR_LEN == idle_$1_LOOPS_PER_LOG / <SOMETHING>_max_loops_per_log);
	
	assert(idle_LION_VOLTS_DATA_ARR_LEN == idle_LION_VOLTS_LOOPS_PER_LOG/idle_max_loops_per_log); 
	assert(idle_LION_CURRENT_DATA_ARR_LEN == idle_LION_CURRENT_LOOPS_PER_LOG/idle_max_loops_per_log); 
	assert(idle_LED_TEMPS_DATA_ARR_LEN == idle_LED_TEMPS_LOOPS_PER_LOG/idle_max_loops_per_log); 
	assert(idle_LIFEPO_CURRENT_DATA_ARR_LEN == idle_LIFEPO_CURRENT_LOOPS_PER_LOG/idle_max_loops_per_log); 
	assert(idle_IR_DATA_ARR_LEN == idle_IR_LOOPS_PER_LOG/idle_max_loops_per_log); 
	assert(idle_DIODE_DATA_ARR_LEN == idle_DIODE_LOOPS_PER_LOG/idle_max_loops_per_log); 
	assert(idle_BAT_TEMP_DATA_ARR_LEN == idle_BAT_TEMP_LOOPS_PER_LOG/idle_max_loops_per_log); 
	assert(idle_IR_TEMPS_DATA_ARR_LEN == idle_IR_TEMPS_LOOPS_PER_LOG/idle_max_loops_per_log); 
	assert(idle_RADIO_TEMP_DATA_ARR_LEN == idle_RADIO_TEMP_LOOPS_PER_LOG/idle_max_loops_per_log); 
	assert(idle_IMU_DATA_ARR_LEN == idle_IMU_LOOPS_PER_LOG/idle_max_loops_per_log); 
	assert(idle_MAGNETOMETER_DATA_ARR_LEN == idle_MAGNETOMETER_LOOPS_PER_LOG/idle_max_loops_per_log); 
	assert(idle_LED_CURRENT_DATA_ARR_LEN == idle_LED_CURRENT_LOOPS_PER_LOG/idle_max_loops_per_log); 
	assert(idle_RADIO_VOLTS_DATA_ARR_LEN == idle_RADIO_VOLTS_LOOPS_PER_LOG/idle_max_loops_per_log); 
	assert(idle_BAT_CHARGE_VOLTS_DATA_ARR_LEN == idle_BAT_CHARGE_VOLTS_LOOPS_PER_LOG/idle_max_loops_per_log); 
	assert(idle_BAT_CHARGE_DIG_SIGS_DATA_ARR_LEN == idle_BAT_CHARGE_DIG_SIGS_LOOPS_PER_LOG/idle_max_loops_per_log); 
	assert(idle_DIGITAL_OUT_DATA_ARR_LEN == idle_DIGITAL_OUT_LOOPS_PER_LOG/idle_max_loops_per_log); 

	
	// We also want to make sure that any sensors' reads per log is no less than one,
	// i.e. that the log frequency is greater than the read frequency
	// (because we can't log faster than we're even reading)
	assert(idle_IR_LOOPS_PER_LOG >= 1);
	
	
	
}

/* Max of array values, of which there must be at least one */
int arrMax(int* arr, uint8_t len) {
	int maxVal = arr[0];
	for (uint8_t i = 1; i < len; i++) {
		if (arr[i] > maxVal) {
			maxVal = arr[i];
		}
	}
	return maxVal;
}