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
#define FLASH_TASK_FREQ							30000
#define TRANSMIT_TASK_FREQ						1000

/* 
 * NOTE: The idle data collection doesn't really need these constants;
 * all sensors are being read at the same frequency, but we're doing it here for consistency
 */

#define DATA_TASK_FREQ							1000
#define DATA_LOW_POWER_TASK_FREQ				10000
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
#define idle_IR_LOOPS_PER_LOG					(10000 / 132) // = idle read freq / DATA_TASK_FREQ
#define idle_TEMP_LOOPS_PER_LOG					(10000 / 132) // = idle read freq / DATA_TASK_FREQ
#define idle_DIODE_LOOPS_PER_LOG				(10000 / 132) // = idle read freq / DATA_TASK_FREQ
#define idle_LED_CURRENT_LOOPS_PER_LOG			(10000 / 132) // = idle read freq / DATA_TASK_FREQ
#define idle_IMU_LOOPS_PER_LOG					(10000 / 132) // = idle read freq / DATA_TASK_FREQ
#define idle_MAGNETOMETER_LOOPS_PER_LOG			(10000 / 132) // = idle read freq / DATA_TASK_FREQ
#define idle_CHARGING_DATA_LOOPS_PER_LOG		(10000 / 132) // = idle read freq / DATA_TASK_FREQ
#define idle_RADIO_TEMP_LOOPS_PER_LOG			(10000 / 132) // = idle read freq / DATA_TASK_FREQ
#define idle_BAT_VOLTAGE_LOOPS_PER_LOG			(10000 / 132) // = idle read freq / DATA_TASK_FREQ
#define idle_REG_VOLTAGE_LOOPS_PER_LOG			(10000 / 132) // = idle read freq / DATA_TASK_FREQ

/*
 * These may have data array lists longer than 1
 */

/* Data array lengths for flash data struct */
#define flash_TEMP_DATA_ARR_LEN					1
#define flash_LED_CURRENT_DATA_ARR_LEN			1
#define flash_BAT_VOLTAGE_DATA_ARR_LEN			1

/* Flash action and data reader task reads per log */
/* LOOPS_PER_LOG for each sensor at each state - 
	How many times the whole sensor task loop must interate before the given sensor is logged 
	in a equistack for transmission. Note that this has a relationship with the array length;
	more frequent sensors (with fewer loops per log) must have longer arrays.
	(see tests in rtos_task_frequencies.c)
	
	NOTE: because the actual HZ frequency entered here is only computed relative 
	to the execution frequency of the data reading RTOS task, it must be less than that frequency.
	*/
#define flash_TEMP_LOOPS_PER_LOG				(2000 / 132) // = ms / DATA_TASK_FREQ
#define flash_LED_CURRENT_LOOPS_PER_LOG			(1000 / 132) // = ms / DATA_TASK_FREQ
#define flash_BAT_VOLTAGE_LOOPS_PER_LOG			(1000 / 132) // = ms / DATA_TASK_FREQ



/* Data array lengths for radio transmit action and data reader task */
#define transmit_RADIO_TEMP_DATA_ARR_LEN		1
#define transmit_LED_CURRENT_DATA_ARR_LEN		1
#define transmit_BAT_VOLTAGE_DATA_ARR_LEN		1

/* Radio transmit and data read task reads per log */
/* ibid */
#define transmit_RADIO_TEMP_LOOPS_PER_LOG		(1000 / 132) // = ms / DATA_TASK_FREQ
#define transmit_LED_CURRENT_LOOPS_PER_LOG		(1000 / 132) // = ms / DATA_TASK_FREQ
#define transmit_BAT_VOLTAGE_LOOPS_PER_LOG		(1000 / 132) // = ms / DATA_TASK_FREQ


/* Data array lengths for attitude data reader task */
#define attitute_data_IR_DATA_ARR_LEN					1
#define attitute_data_DIODE_DATA_ARR_LEN				1
#define attitute_data_IMU_DATA_ARR_LEN					1
#define attitute_data_MAGNETOMETER_DATA_ARR_LEN			1

/* Attitude data read task reads per log */
/* ibid */
#define attitute_data_IR_LOOPS_PER_LOG					(2000 / 132) // = ms / DATA_TASK_FREQ
#define attitute_data_DIODE_LOOPS_PER_LOG				(1000 / 132) // = ms / DATA_TASK_FREQ
#define attitute_data_IMU_LOOPS_PER_LOG					(1000 / 132) // = ms / DATA_TASK_FREQ
#define attitute_data_MAGNETOMETER_LOOPS_PER_LOG		(1000 / 132) // = ms / DATA_TASK_FREQ

// TODO: Do this for:
// Flash comparison?

/*
 * A function to make sure that the constants defined here are internally consistent.
 */
void assertConstantDefinitions(void);
int arrMax(int* arr, uint8_t len);

#endif
