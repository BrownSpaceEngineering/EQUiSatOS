/*
 * State_Structs.h
 *
 * Created: 10/30/2016 2:02:48 PM
 *  Author: mcken
 */ 

#ifndef STATE_STRUCTS_H
#define STATE_STRUCTS_H

#include "Sensor_Structs.h"
#include "../runnable_configurations/rtos_task_frequencies.h"
#include <asf.h>

typedef struct idle_data_t
{
	uint32_t timestamp;
	/* lists sizes are proportional to the max READS_PER_LOG
	(because the higher the MAX number of READS_PER_LOG, the lower the 
	frequency of the least frequent task, and the larger our higher-frequency 
	arrays need to be to get that one element of lower frequency data).
	They are also inversely proportional to the READS_PER_LOG of each 
	corresponding sensor because if MORE reads are required for each log,
	the data is of LOWER frequency and thus needs a SMALLER array. */
	ir_batch ir_data								[idle_IR_DATA_ARR_LEN];
	temp_batch temp_data							[idle_TEMP_DATA_ARR_LEN];
	diode_batch diode_data							[idle_DIODE_DATA_ARR_LEN];
	led_current_batch led_current_data				[idle_LED_CURRENT_DATA_ARR_LEN];
	imu_batch imu_data								[idle_IMU_DATA_ARR_LEN];
	magnetometer_batch magnetometer_data			[idle_MAGNETOMETER_DATA_ARR_LEN];
	charging_batch charging_data					[idle_CHARGING_DATA_DATA_ARR_LEN];
	radio_temp_batch radio_temp_data				[idle_RADIO_TEMP_DATA_ARR_LEN];
	battery_voltages_batch battery_voltages_data	[idle_BAT_VOLTAGE_DATA_ARR_LEN];
	regulator_voltages_batch regulator_voltages_data[idle_REG_VOLTAGE_DATA_ARR_LEN];
	
} idle_data_t;

typedef struct flash_data_t
{
	uint32_t timestamp;
	/* ibid */
	temp_batch temp_data							[flash_TEMP_DATA_ARR_LEN];
	led_current_batch led_current_data				[flash_LED_CURRENT_DATA_ARR_LEN];
	battery_voltages_batch battery_voltages_data	[flash_BAT_VOLTAGE_DATA_ARR_LEN];
	
} flash_data_t;

void free_idle(idle_data_t* id);
void free_flash(flash_data_t* fd);

// TODO: MORE

#endif