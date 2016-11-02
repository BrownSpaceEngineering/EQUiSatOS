/*
 * rtos_task_frequencies.h
 *
 * Created: 10/2/2016 3:38:22 PM
 *  Author: mckenna
 */ 

#ifndef RTOS_TASK_FREQ_H
#define RTOS_TASK_FREQ_H

/* action frequencies */
// idle state
#define idle_TASK_LED_FREQ						200
#define idle_TASK_RADIO_TRANSMIT_FREQ			200

// flash state
#define flash_TASK_LED_FREQ						200
#define flash_TASK_RADIO_TRANSMIT_FREQ			200

// low power state
#define low_power_TASK_LED_FREQ					200
#define low_power_TASK_RADIO_TRANSMIT_FREQ		200

/* sensor read frequencies (correspond directly to states) */
#define IDLE_RD_TASK_FREQ						200
#define FLASH_RD_TASK_FREQ						200
#define LOW_POWER_RD_TASK_FREQ					200
#define BOOT_RD_TASK_FREQ					    200

/* Data structure size determinations 
	Each should ideally be equal to DATA_DURATION_TO_STORE * (max _RD_FREQ / _READS_PER_LOG)
								AKA DATA_DURATION_TO_STORE * [logs per second]
	*/

// #define DATA_DURATION_TO_STORE =				10 // sec; ideal duration of data to store in queues
// 				
// #define IR_RD_QUEUE_SIZE						DATA_DURATION_TO_STORE * (flash_STATE_RD_FREQ / ) // state queue should be the LAST to overflow  
// #define TEMP_RD_QUEUE_SIZE						200
// #define DIODE_RD_QUEUE_SIZE						200
// #define LED_CURRENT_RD_QUEUE_SIZE				200
// #define GYRO_RD_QUEUE_SIZE						200
// #define MAGNETOMETER_RD_QUEUE_SIZE				200
// #define CHARGING_DATA_RD_QUEUE_SIZE				200
// #define RADIO_TEMP_RD_QUEUE_SIZE					200
// #define BAT_VOLTAGE_RD_QUEUE_SIZE				200
// #define REG_VOLTAGE_RD_QUEUE_SIZE				200

/* READS_PER_LOG for each sensor at each state - 
	how often the data is logged in a equistack for transmission 
	NOTE: because the actual HZ frequency entered here is only computed relative 
	to the execution frequency of the reading RTOS task, it must be less than that frequency.
	*/



/* "idle" state LOG frequencies - normal operation 
	IF YOU CHANGE THESE: update _MAX_READS_PER_LOG below */
#define idle_IR_READS_PER_LOG					IDLE_RD_TASK_FREQ / 200
#define idle_TEMP_READS_PER_LOG					IDLE_RD_TASK_FREQ / 200
#define idle_DIODE_READS_PER_LOG				IDLE_RD_TASK_FREQ / 200
#define idle_LED_CURRENT_READS_PER_LOG			IDLE_RD_TASK_FREQ / 200
#define idle_GYRO_READS_PER_LOG					IDLE_RD_TASK_FREQ / 200
#define idle_MAGNETOMETER_READS_PER_LOG			IDLE_RD_TASK_FREQ / 200
#define idle_CHARGING_DATA_READS_PER_LOG		IDLE_RD_TASK_FREQ / 200
#define idle_RADIO_TEMP_READS_PER_LOG			IDLE_RD_TASK_FREQ / 200
#define idle_BAT_VOLTAGE_READS_PER_LOG			IDLE_RD_TASK_FREQ / 200
#define idle_REG_VOLTAGE_READS_PER_LOG			IDLE_RD_TASK_FREQ / 200

// from above values, define a shortcut for the MOST reads per log
// corresponding to the LOWEST log frequency, which can be used
// to easily create a series of lists with the lowest log frequency
// having the smallest list (see State_Structs.h)
// EX. If we're creating lists where the lowest frequency data list
// should be of size one, defining their lengths as:
// [state]_MAX_READS_PER_LOG / [state]_[sensor]_READS_PER_LOG
// will result in the lowest frequency having a size of 1
#define idle_MAX_READS_PER_LOG					idle_IR_READS_PER_LOG
// define a reference to the lowest frequency sensor using the 
// enum value from the data_types enum so we can easily
// check if it's been read (which would indicate that a state struct is full) 
// (this is the same sensor that was used in _MAX_READS_PER_LOG)
#define idle_LOWEST_FREQ_SENSOR  				0	

/* "flash" state LOG frequencies - operation during flash 
	IF YOU CHANGE THESE: update _MAX_READS_PER_LOG below */
#define flash_IR_READS_PER_LOG					FLASH_RD_TASK_FREQ / 200 
#define flash_TEMP_READS_PER_LOG				FLASH_RD_TASK_FREQ / 200
#define flash_DIODE_READS_PER_LOG				FLASH_RD_TASK_FREQ / 200
#define flash_LED_CURRENT_READS_PER_LOG			FLASH_RD_TASK_FREQ / 200
#define flash_GYRO_READS_PER_LOG				FLASH_RD_TASK_FREQ / 200
#define flash_MAGNETOMETER_READS_PER_LOG		FLASH_RD_TASK_FREQ / 200
#define flash_CHARGING_DATA_READS_PER_LOG		FLASH_RD_TASK_FREQ / 200
#define flash_RADIO_TEMP_READS_PER_LOG			FLASH_RD_TASK_FREQ / 200
#define flash_BAT_VOLTAGE_READS_PER_LOG			FLASH_RD_TASK_FREQ / 200
#define flash_REG_VOLTAGE_READS_PER_LOG			FLASH_RD_TASK_FREQ / 200

// shortcuts from above values
#define flash_MAX_READS_PER_LOG					flash_IR_READS_PER_LOG
#define flash_LOWEST_FREQ_SENSOR  				0	

/* "low_power" state LOG frequencies - operation when low on power 
	IF YOU CHANGE THESE: update _MAX_READS_PER_LOG below */
#define low_power_IR_READS_PER_LOG				LOW_POWER_RD_TASK_FREQ / 200 
#define low_power_TEMP_READS_PER_LOG			LOW_POWER_RD_TASK_FREQ / 200
#define low_power_DIODE_READS_PER_LOG			LOW_POWER_RD_TASK_FREQ / 200
#define low_power_LED_CURRENT_READS_PER_LOG		LOW_POWER_RD_TASK_FREQ / 200
#define low_power_GYRO_READS_PER_LOG			LOW_POWER_RD_TASK_FREQ / 200
#define low_power_MAGNETOMETER_READS_PER_LOG	LOW_POWER_RD_TASK_FREQ / 200
#define low_power_CHARGING_DATA_READS_PER_LOG	LOW_POWER_RD_TASK_FREQ / 200
#define low_power_RADIO_TEMP_READS_PER_LOG		LOW_POWER_RD_TASK_FREQ / 200
#define low_power_BAT_VOLTAGE_READS_PER_LOG		LOW_POWER_RD_TASK_FREQ / 200
#define low_power_REG_VOLTAGE_READS_PER_LOG		LOW_POWER_RD_TASK_FREQ / 200

// shortcuts from above values
#define low_power_MAX_READS_PER_LOG				low_power_IR_READS_PER_LOG
#define low_power_LOWEST_FREQ_SENSOR  			0	

#define boot_IR_READS_PER_LOG				BOOT_RD_TASK_FREQ / 200
#define boot_TEMP_READS_PER_LOG			    BOOT_RD_TASK_FREQ / 200
#define boot_DIODE_READS_PER_LOG			BOOT_RD_TASK_FREQ / 200
#define boot_LED_CURRENT_READS_PER_LOG		BOOT_RD_TASK_FREQ / 200
#define boot_GYRO_READS_PER_LOG			    BOOT_RD_TASK_FREQ / 200
#define boot_MAGNETOMETER_READS_PER_LOG		BOOT_RD_TASK_FREQ / 200
#define boot_CHARGING_DATA_READS_PER_LOG	BOOT_RD_TASK_FREQ / 200
#define boot_RADIO_TEMP_READS_PER_LOG		BOOT_RD_TASK_FREQ / 200
#define boot_BAT_VOLTAGE_READS_PER_LOG		BOOT_RD_TASK_FREQ / 200
#define boot_REG_VOLTAGE_READS_PER_LOG		BOOT_RD_TASK_FREQ / 200
#define boot_REG_VOLTAGE_READS_PER_LOG		BOOT_RD_TASK_FREQ / 200

// shortcuts from above values
#define boot_MAX_READS_PER_LOG				boot_IR_READS_PER_LOG
#define boot_LOWEST_FREQ_SENSOR  			0

#endif