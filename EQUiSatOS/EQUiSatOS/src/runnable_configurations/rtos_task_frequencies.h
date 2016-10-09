/*
 * rtos_task_frequencies.h
 *
 * Created: 10/2/2016 3:38:22 PM
 *  Author: mckenna
 */ 

#ifndef RTOS_TASK_FREQ_H
#define RTOS_TASK_FREQ_H

/* "idle" state frequencies - normal operation */
#define idle_TASK_LED_FREQ						200 // ms
#define idle_TASK_RADIO_TRANSMIT_FREQ			200
#define idle_TASK_STATE_RD_FREQ					200
#define idle_TASK_IR_RD_FREQ					200
#define idle_TASK_TEMP_RD_FREQ					200
#define idle_TASK_DIODE_RD_FREQ					200
#define idle_TASK_LED_CURRENT_RD_FREQ			200
#define idle_TASK_GYRO_RD_FREQ					200
#define idle_TASK_MAGNETOMETER_RD_FREQ			200
#define idle_TASK_CHARGING_DATA_RD_FREQ			200
#define idle_TASK_RADIO_TEMP_RD_FREQ			200
#define idle_TASK_BAT_VOLTAGE_RD_FREQ			200
#define idle_TASK_REG_VOLTAGE_RD_FREQ			200

/* "flash" state frequencies - operation during flash */
#define flash_TASK_LED_FREQ						200 // ms
#define flash_TASK_RADIO_TRANSMIT_FREQ			200
#define flash_TASK_STATE_RD_FREQ				200
#define flash_TASK_IR_RD_FREQ					200
#define flash_TASK_TEMP_RD_FREQ					200
#define flash_TASK_DIODE_RD_FREQ				200
#define flash_TASK_LED_CURRENT_RD_FREQ			200
#define flash_TASK_GYRO_RD_FREQ					200
#define flash_TASK_MAGNETOMETER_RD_FREQ			200
#define flash_TASK_CHARGING_DATA_RD_FREQ		200
#define flash_TASK_RADIO_TEMP_RD_FREQ			200
#define flash_TASK_BAT_VOLTAGE_RD_FREQ			200
#define flash_TASK_REG_VOLTAGE_RD_FREQ			200

/* "low_power" state frequencies - operation when low on power */
#define low_power_TASK_LED_FREQ					200 // ms
#define low_power_TASK_RADIO_TRANSMIT_FREQ		200 
#define low_power_TASK_STATE_RD_FREQ			200
#define low_power_TASK_IR_RD_FREQ				200
#define low_power_TASK_TEMP_RD_FREQ				200
#define low_power_TASK_DIODE_RD_FREQ			200
#define low_power_TASK_LED_CURRENT_RD_FREQ		200
#define low_power_TASK_GYRO_RD_FREQ				200
#define low_power_TASK_MAGNETOMETER_RD_FREQ		200
#define low_power_TASK_CHARGING_DATA_RD_FREQ	200
#define low_power_TASK_RADIO_TEMP_RD_FREQ		200
#define low_power_TASK_BAT_VOLTAGE_RD_FREQ		200
#define low_power_TASK_REG_VOLTAGE_RD_FREQ		200

#endif