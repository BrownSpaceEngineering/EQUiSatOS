/*
 * rtos_tasks_config.c
 *
 * Responsible for making sure that the constants defined in rtos_tasks_config.h are internally consistent.
 *
 * Created: 2/26/2017 2:57:52 PM
 *  Author: mckenna
 */ 

#include "rtos_tasks_config.h"

/* Max of array values, of which there must be at least one */
static int arrMax(int* arr, uint8_t len) {
	int maxVal = arr[0];
	for (uint8_t i = 1; i < len; i++) {
		if (arr[i] > maxVal) {
			maxVal = arr[i];
		}
	}
	return maxVal;
}

void assert_rtos_constants() {
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
	int attitude_all_loops_per_log[] = {
		attitude_IR_DATA_ARR_LEN,
		attitude_DIODE_DATA_ARR_LEN,
		attitude_ACCELEROMETER_DATA_ARR_LEN,
		attitude_GYRO_DATA_ARR_LEN,
		attitude_MAGNETOMETER_DATA_ARR_LEN};
	uint16_t attitude_max_loops_per_log = arrMax(attitude_all_loops_per_log, 6); // max of the above (MAKE SURE TO UPDATE IF YOU ADD)
		
	assert(attitude_IR_DATA_ARR_LEN == attitude_max_loops_per_log / attitude_IR_LOOPS_PER_LOG);
	assert(attitude_DIODE_DATA_ARR_LEN == attitude_max_loops_per_log / attitude_DIODE_LOOPS_PER_LOG);
	assert(attitude_ACCELEROMETER_DATA_ARR_LEN == attitude_max_loops_per_log / attitude_ACCELEROMETER_LOOPS_PER_LOG);
	assert(attitude_GYRO_DATA_ARR_LEN == attitude_max_loops_per_log / attitude_GYRO_LOOPS_PER_LOG);
	assert(attitude_MAGNETOMETER_DATA_ARR_LEN == attitude_max_loops_per_log / attitude_MAGNETOMETER_LOOPS_PER_LOG);
		
	// We also want to make sure that any sensors' reads per log is no less than one,
	// i.e. that the log frequency is greater than the read frequency
	// (because we can't log faster than we're even reading)
	// helpful Regex: #define (.*?)_(.*)_DATA_ARR_LEN.* --> assert($1_$2_LOOPS_PER_LOG >= 1);
	assert(attitude_IR_LOOPS_PER_LOG >= 1);
	assert(attitude_DIODE_LOOPS_PER_LOG >= 1);
	assert(attitude_ACCELEROMETER_LOOPS_PER_LOG >= 1);
	assert(attitude_GYRO_LOOPS_PER_LOG >= 1);
	assert(attitude_MAGNETOMETER_LOOPS_PER_LOG >= 1);	
		
	int flash_all_loops_per_log[] = {
		flash_LED_CURRENT_LOOPS_PER_LOG,
		flash_LED_TEMPS_LOOPS_PER_LOG,
		flash_LIFEPO_CURRENT_LOOPS_PER_LOG,
		flash_LIFEPO_VOLTS_LOOPS_PER_LOG};
	uint16_t flash_max_loops_per_log = arrMax(flash_all_loops_per_log, 4); // max of the above (MAKE SURE TO UPDATE IF YOU ADD)
	
	assert(flash_LED_TEMPS_DATA_ARR_LEN == flash_max_loops_per_log / flash_LED_TEMPS_LOOPS_PER_LOG);
	assert(flash_LIFEPO_CURRENT_DATA_ARR_LEN == flash_max_loops_per_log / flash_LIFEPO_CURRENT_LOOPS_PER_LOG);
	assert(flash_LIFEPO_VOLTS_DATA_ARR_LEN == flash_max_loops_per_log / flash_LIFEPO_VOLTS_LOOPS_PER_LOG);
	assert(flash_LED_CURRENT_DATA_ARR_LEN == flash_max_loops_per_log / flash_LED_CURRENT_LOOPS_PER_LOG);

	assert(flash_LED_TEMPS_LOOPS_PER_LOG >= 1);
	assert(flash_LIFEPO_CURRENT_LOOPS_PER_LOG >= 1);
	assert(flash_LIFEPO_VOLTS_LOOPS_PER_LOG >= 1);
	assert(flash_LED_CURRENT_LOOPS_PER_LOG >= 1);
	
	int flash_cmp_all_loops_per_log[] = {
		flash_LED_CURRENT_AVG_LOOPS_PER_LOG,
		flash_LED_TEMPS_AVG_LOOPS_PER_LOG,
		flash_LIFEPO_CURRENT_AVG_LOOPS_PER_LOG,
		flash_LIFEPO_VOLTS_AVG_LOOPS_PER_LOG};
	uint16_t flash_cmp_max_loops_per_log = arrMax(flash_cmp_all_loops_per_log, 4); // max of the above (MAKE SURE TO UPDATE IF YOU ADD)
		
	assert(flash_LED_TEMPS_AVG_DATA_ARR_LEN == flash_cmp_max_loops_per_log / flash_LED_TEMPS_AVG_LOOPS_PER_LOG);
	assert(flash_LIFEPO_CURRENT_AVG_DATA_ARR_LEN == flash_cmp_max_loops_per_log / flash_LIFEPO_CURRENT_AVG_LOOPS_PER_LOG);
	assert(flash_LIFEPO_VOLTS_AVG_DATA_ARR_LEN == flash_cmp_max_loops_per_log / flash_LIFEPO_VOLTS_AVG_LOOPS_PER_LOG);
	assert(flash_LED_CURRENT_AVG_DATA_ARR_LEN == flash_cmp_max_loops_per_log / flash_LED_CURRENT_AVG_LOOPS_PER_LOG);

	assert(flash_LED_TEMPS_AVG_LOOPS_PER_LOG >= 1);
	assert(flash_LIFEPO_CURRENT_AVG_LOOPS_PER_LOG >= 1);
	assert(flash_LIFEPO_VOLTS_AVG_LOOPS_PER_LOG >= 1);
	assert(flash_LED_CURRENT_AVG_LOOPS_PER_LOG >= 1);
}