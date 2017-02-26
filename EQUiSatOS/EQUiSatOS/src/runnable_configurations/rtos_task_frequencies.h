/*
 * rtos_task_frequencies.h
 *
 * Created: 10/2/2016 3:38:22 PM
 *  Author: mckenna
 */ 

#ifndef RTOS_TASK_FREQ_H
#define RTOS_TASK_FREQ_H

/* action frequency periods in MS */
#define WATCHDOG_TASK_FREQ						1000
#define ANTENNA_DEPLOY_TASK_FREQ				1000
#define BATTERY_CHARGING_TASK_FREQ				1000
#define FLASH_ACTIVATE_TASK_FREQ				1000
#define TRANSMIT_TASK_FREQ						1000
#define ATTITUDE_DATA_TASK_FREQ					1000

#define CURRENT_DATA_TASK_FREQ					1000
/* Data array lengths for current data reader task */
#define idle_IR_DATA_ARR_LEN					1
#define idle_TEMP_DATA_ARR_LEN					1
#define idle_DIODE_DATA_ARR_LEN					1
#define idle_LED_CURRENT_DATA_ARR_LEN			1
#define idle_GYRO_DATA_ARR_LEN					1
#define idle_MAGNETOMETER_DATA_ARR_LEN			1
#define idle_CHARGING_DATA_DATA_ARR_LEN			1
#define idle_RADIO_TEMP_DATA_ARR_LEN			1
#define idle_BAT_VOLTAGE_DATA_ARR_LEN			1
#define idle_REG_VOLTAGE_DATA_ARR_LEN			1

/* Current data reader reads per log */
/* READS_PER_LOG for each sensor at each state - 
	how often the data is logged in a equistack for transmission 
	NOTE: because the actual HZ frequency entered here is only computed relative 
	to the execution frequency of the reading RTOS task, it must be less than that frequency.
	*/
#define idle_IR_READS_PER_LOG					(2000 / 1000) // = ms / CURRENT_DATA_TASK_FREQ
#define idle_TEMP_READS_PER_LOG					(1000 / 1000) // = ms / CURRENT_DATA_TASK_FREQ
#define idle_DIODE_READS_PER_LOG				(1000 / 1000) // = ms / CURRENT_DATA_TASK_FREQ
#define idle_LED_CURRENT_READS_PER_LOG			(1000 / 1000) // = ms / CURRENT_DATA_TASK_FREQ
#define idle_GYRO_READS_PER_LOG					(1000 / 1000) // = ms / CURRENT_DATA_TASK_FREQ
#define idle_MAGNETOMETER_READS_PER_LOG			(1000 / 1000) // = ms / CURRENT_DATA_TASK_FREQ
#define idle_CHARGING_DATA_READS_PER_LOG		(1000 / 1000) // = ms / CURRENT_DATA_TASK_FREQ
#define idle_RADIO_TEMP_READS_PER_LOG			(1000 / 1000) // = ms / CURRENT_DATA_TASK_FREQ
#define idle_BAT_VOLTAGE_READS_PER_LOG			(1000 / 1000) // = ms / CURRENT_DATA_TASK_FREQ
#define idle_REG_VOLTAGE_READS_PER_LOG			(1000 / 1000) // = ms / CURRENT_DATA_TASK_FREQ

#define CURRENT_DATA_LOW_POWER_TASK_FREQ		1000;
/* Data array lengths for current data low power reader task */
#define low_power_IR_DATA_ARR_LEN					1
#define low_power_TEMP_DATA_ARR_LEN					1
#define low_power_DIODE_DATA_ARR_LEN				1
#define low_power_LED_CURRENT_DATA_ARR_LEN			1
#define low_power_GYRO_DATA_ARR_LEN					1
#define low_power_MAGNETOMETER_DATA_ARR_LEN			1
#define low_power_CHARGING_DATA_DATA_ARR_LEN		1
#define low_power_RADIO_TEMP_DATA_ARR_LEN			1
#define low_power_BAT_VOLTAGE_DATA_ARR_LEN			1
#define low_power_REG_VOLTAGE_DATA_ARR_LEN			1

/* Current data low power reader reads per log */
/* ibid */
#define low_power_IR_READS_PER_LOG					(2000 / 1000) // = ms / CURRENT_DATA_LOW_POWER_TASK_FREQ
#define low_power_TEMP_READS_PER_LOG				(1000 / 1000) // = ms / CURRENT_DATA_LOW_POWER_TASK_FREQ
#define low_power_DIODE_READS_PER_LOG				(1000 / 1000) // = ms / CURRENT_DATA_LOW_POWER_TASK_FREQ
#define low_power_LED_CURRENT_READS_PER_LOG			(1000 / 1000) // = ms / CURRENT_DATA_LOW_POWER_TASK_FREQ
#define low_power_GYRO_READS_PER_LOG				(1000 / 1000) // = ms / CURRENT_DATA_LOW_POWER_TASK_FREQ
#define low_power_MAGNETOMETER_READS_PER_LOG		(1000 / 1000) // = ms / CURRENT_DATA_LOW_POWER_TASK_FREQ
#define low_power_CHARGING_DATA_READS_PER_LOG		(1000 / 1000) // = ms / CURRENT_DATA_LOW_POWER_TASK_FREQ
#define low_power_RADIO_TEMP_READS_PER_LOG			(1000 / 1000) // = ms / CURRENT_DATA_LOW_POWER_TASK_FREQ
#define low_power_BAT_VOLTAGE_READS_PER_LOG			(1000 / 1000) // = ms / CURRENT_DATA_LOW_POWER_TASK_FREQ
#define low_power_REG_VOLTAGE_READS_PER_LOG			(1000 / 1000) // = ms / CURRENT_DATA_LOW_POWER_TASK_FREQ

#define FLASH_DATA_TASK_FREQ		1000;
/* Data array lengths for current data low power reader task */
#define flash_IR_DATA_ARR_LEN					1
#define flash_TEMP_DATA_ARR_LEN					1
#define flash_DIODE_DATA_ARR_LEN				1
#define flash_LED_CURRENT_DATA_ARR_LEN			1
#define flash_GYRO_DATA_ARR_LEN					1
#define flash_MAGNETOMETER_DATA_ARR_LEN			1
#define flash_CHARGING_DATA_DATA_ARR_LEN		1
#define flash_RADIO_TEMP_DATA_ARR_LEN			1
#define flash_BAT_VOLTAGE_DATA_ARR_LEN			1
#define flash_REG_VOLTAGE_DATA_ARR_LEN			1

/* Current data low power reader reads per log */
/* ibid */
#define flash_IR_READS_PER_LOG					(2000 / 1000) // = ms / FLASH_DATA_TASK_FREQ
#define flash_TEMP_READS_PER_LOG				(1000 / 1000) // = ms / FLASH_DATA_TASK_FREQ
#define flash_DIODE_READS_PER_LOG				(1000 / 1000) // = ms / FLASH_DATA_TASK_FREQ
#define flash_LED_CURRENT_READS_PER_LOG			(1000 / 1000) // = ms / FLASH_DATA_TASK_FREQ
#define flash_GYRO_READS_PER_LOG				(1000 / 1000) // = ms / FLASH_DATA_TASK_FREQ
#define flash_MAGNETOMETER_READS_PER_LOG		(1000 / 1000) // = ms / FLASH_DATA_TASK_FREQ
#define flash_CHARGING_DATA_READS_PER_LOG		(1000 / 1000) // = ms / FLASH_DATA_TASK_FREQ
#define flash_RADIO_TEMP_READS_PER_LOG			(1000 / 1000) // = ms / FLASH_DATA_TASK_FREQ
#define flash_BAT_VOLTAGE_READS_PER_LOG			(1000 / 1000) // = ms / FLASH_DATA_TASK_FREQ
#define flash_REG_VOLTAGE_READS_PER_LOG			(1000 / 1000) // = ms / FLASH_DATA_TASK_FREQ

// TODO: Do this for:
// Attitude data
// Radio data
// Flash burst
// Flash comparison?

/*
*
* OLD
*
*/

// /* READS_PER_LOG for each sensor at each state - 
// 	how often the data is logged in a equistack for transmission 
// 	NOTE: because the actual HZ frequency entered here is only computed relative 
// 	to the execution frequency of the reading RTOS task, it must be less than that frequency.
// 	*/
// 
// /* "idle" state LOG frequencies - normal operation 
// 	IF YOU CHANGE THESE: update _MAX_READS_PER_LOG below */
// #define idle_IR_READS_PER_LOG					(2000 / 1000) // = ms / IDLE_RD_TASK_FREQ
// #define idle_TEMP_READS_PER_LOG					(1000 / 1000) // = ms / IDLE_RD_TASK_FREQ
// #define idle_DIODE_READS_PER_LOG				(1000 / 1000) // = ms / IDLE_RD_TASK_FREQ
// #define idle_LED_CURRENT_READS_PER_LOG			(1000 / 1000) // = ms / IDLE_RD_TASK_FREQ
// #define idle_GYRO_READS_PER_LOG					(1000 / 1000) // = ms / IDLE_RD_TASK_FREQ
// #define idle_MAGNETOMETER_READS_PER_LOG			(1000 / 1000) // = ms / IDLE_RD_TASK_FREQ
// #define idle_CHARGING_DATA_READS_PER_LOG		(1000 / 1000) // = ms / IDLE_RD_TASK_FREQ
// #define idle_RADIO_TEMP_READS_PER_LOG			(1000 / 1000) // = ms / IDLE_RD_TASK_FREQ
// #define idle_BAT_VOLTAGE_READS_PER_LOG			(1000 / 1000) // = ms / IDLE_RD_TASK_FREQ
// #define idle_REG_VOLTAGE_READS_PER_LOG			(1000 / 1000) // = ms / IDLE_RD_TASK_FREQ
// 
// // from above values, define a shortcut for the MOST reads per log
// // corresponding to the LOWEST log frequency, which can be used
// // to easily create a series of lists with the lowest log frequency
// // having the smallest list (see State_Structs.h)
// // EX. If we're creating lists where the lowest frequency data list
// // should be of size one, defining their lengths as:
// // [state]_MAX_READS_PER_LOG / [state]_[sensor]_READS_PER_LOG
// // will result in the lowest frequency having a size of 1
// // (multiply this by something to change the smallest size; 
// // i.e. x2 means _LOWEST_FREQ_SENSOR has an array of size 2)
// #define idle_MAX_READS_PER_LOG					idle_IR_READS_PER_LOG
// // define a reference to the lowest frequency sensor using the 
// // enum value from the data_types enum so we can easily
// // check if it's been read (which would indicate that a state struct is full) 
// // (this is the same sensor that was used in _MAX_READS_PER_LOG)
// #define idle_LOWEST_FREQ_SENSOR  				0 // IR_DATA

#endif
