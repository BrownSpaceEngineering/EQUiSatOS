/*
 * rtos_task_frequencies.h
 *
 * Created: 10/2/2016 3:38:22 PM
 *  Author: mckenna
 */ 

#ifndef RTOS_TASK_FREQ_H
#define RTOS_TASK_FREQ_H

#include <asf.h>
#include <assert.h>

/* action frequency periods in MS (some that actually have data collection are below) */
#define WATCHDOG_TASK_FREQ						1000
#define ANTENNA_DEPLOY_TASK_FREQ				1000
#define BATTERY_CHARGING_TASK_FREQ				100

/* 
 * NOTE: The idle data collection doesn't really need these constants;
 * all sensors are being read at the same frequency, but we're doing it here for consistency
 */

#define CURRENT_DATA_TASK_FREQ					1000
#define CURRENT_DATA_LOW_POWER_TASK_FREQ		10000
/* Data array lengths for current data reader task */
#define idle_IR_DATA_ARR_LEN					1
#define idle_TEMP_DATA_ARR_LEN					1
#define idle_DIODE_DATA_ARR_LEN					1
#define idle_LED_CURRENT_DATA_ARR_LEN			1
#define idle_IMU_DATA_ARR_LEN					1
#define idle_MAGNETOMETER_DATA_ARR_LEN			1
#define idle_CHARGING_DATA_DATA_ARR_LEN			1
#define idle_RADIO_TEMP_DATA_ARR_LEN			1
#define idle_BAT_VOLTAGE_DATA_ARR_LEN			1
#define idle_REG_VOLTAGE_DATA_ARR_LEN			1
// TODO: ADD more

/* Current data reader reads per log */
/* READS_PER_LOG for each sensor at each state - 
	how often the data is logged in a equistack for transmission 
	NOTE: because the actual HZ frequency entered here is only computed relative 
	to the execution frequency of the reading RTOS task, it must be less than that frequency.
	*/
#define idle_IR_READS_PER_LOG					1 // ms freq == CURRENT_DATA_TASK_FREQ
#define idle_TEMP_READS_PER_LOG					1 // ms freq == CURRENT_DATA_TASK_FREQ
#define idle_DIODE_READS_PER_LOG				1 // ms freq == CURRENT_DATA_TASK_FREQ
#define idle_LED_CURRENT_READS_PER_LOG			1 // ms freq == CURRENT_DATA_TASK_FREQ
#define idle_IMU_READS_PER_LOG					1 // ms freq == CURRENT_DATA_TASK_FREQ
#define idle_MAGNETOMETER_READS_PER_LOG			1 // ms freq == CURRENT_DATA_TASK_FREQ
#define idle_CHARGING_DATA_READS_PER_LOG		1 // ms freq == CURRENT_DATA_TASK_FREQ
#define idle_RADIO_TEMP_READS_PER_LOG			1 // ms freq == CURRENT_DATA_TASK_FREQ
#define idle_BAT_VOLTAGE_READS_PER_LOG			1 // ms freq == CURRENT_DATA_TASK_FREQ
#define idle_REG_VOLTAGE_READS_PER_LOG			1 // ms freq == CURRENT_DATA_TASK_FREQ

/*
 * These may have data array lists longer than 1
 */

#define FLASH_ACTIVATE_TASK_FREQ				1000
/* Data array lengths for flash action and data reader task */
#define flash_TEMP_DATA_ARR_LEN					1
#define flash_LED_CURRENT_DATA_ARR_LEN			1
#define flash_BAT_VOLTAGE_DATA_ARR_LEN			1

/* Flash action and data reader task reads per log */
/* READS_PER_LOG for each sensor at each state - 
	how often the data is logged in a equistack for transmission 
	NOTE: because the actual HZ frequency entered here is only computed relative 
	to the execution frequency of the reading RTOS task, it must be less than that frequency.
	*/
#define flash_TEMP_READS_PER_LOG				(1000 / 1000) // = ms / FLASH_ACTIVATE_TASK_FREQ
#define flash_LED_CURRENT_READS_PER_LOG			(1000 / 1000) // = ms / FLASH_ACTIVATE_TASK_FREQ
#define flash_BAT_VOLTAGE_READS_PER_LOG			(1000 / 1000) // = ms / FLASH_ACTIVATE_TASK_FREQ


#define TRANSMIT_TASK_FREQ						1000
/* Data array lengths for radio transmit action and data reader task */
#define transmit_RADIO_TEMP_DATA_ARR_LEN		1
#define transmit_LED_CURRENT_DATA_ARR_LEN		1
#define transmit_BAT_VOLTAGE_DATA_ARR_LEN		1

/* Radio transmit and data read task reads per log */
/* ibid */
#define transmit_RADIO_TEMP_READS_PER_LOG		(1000 / 1000) // = ms / TRANSMIT_TASK_FREQ
#define transmit_LED_CURRENT_READS_PER_LOG		(1000 / 1000) // = ms / TRANSMIT_TASK_FREQ
#define transmit_BAT_VOLTAGE_READS_PER_LOG		(1000 / 1000) // = ms / TRANSMIT_TASK_FREQ


#define ATTITUDE_DATA_TASK_FREQ							1000
/* Data array lengths for attitude data reader task */
#define attitute_data_IR_DATA_ARR_LEN					1
#define attitute_data_DIODE_DATA_ARR_LEN				1
#define attitute_data_IMU_DATA_ARR_LEN					1
#define attitute_data_MAGNETOMETER_DATA_ARR_LEN			1

/* Attitude data read task reads per log */
/* ibid */
#define attitute_data_IR_READS_PER_LOG					(2000 / 1000) // = ms / ATTITUDE_DATA__ACTIVATE_TASK_FREQ
#define attitute_data_DIODE_READS_PER_LOG				(1000 / 1000) // = ms / ATTITUDE_DATA__ACTIVATE_TASK_FREQ
#define attitute_data_IMU_READS_PER_LOG					(1000 / 1000) // = ms / ATTITUDE_DATA__ACTIVATE_TASK_FREQ
#define attitute_data_MAGNETOMETER_READS_PER_LOG		(1000 / 1000) // = ms / ATTITUDE_DATA__ACTIVATE_TASK_FREQ

// TODO: Do this for:
// Flash comparison?

/*
 * A function to make sure that the constants defined here are internally consistent.
 */
void assertConstantDefinitions(void);
int arrMax(int* arr, uint8_t len);

#endif
