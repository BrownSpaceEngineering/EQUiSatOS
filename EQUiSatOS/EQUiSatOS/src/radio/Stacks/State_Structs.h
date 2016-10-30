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

typedef struct idle_data_t
{
	/* lists sizes are proportional to the max READS_PER_LOG
	(because the higher the MAX number of READS_PER_LOG, the lower the 
	frequency of the least frequent task, and the larger our higher-frequency 
	arrays need to be to get that one element of lower frequency data).
	They are also inversely proportional to the READS_PER_LOG of each 
	corresponding sensor because if MORE reads are required for each log,
	the data is of LOWER frequency and thus needs a SMALLER array. */
	ir_batch ir_data								[idle_MAX_READS_PER_LOG / idle_IR_READS_PER_LOG];
	temp_batch temp_data							[idle_MAX_READS_PER_LOG / idle_TEMP_READS_PER_LOG];
	diode_batch diode_data							[idle_MAX_READS_PER_LOG / idle_DIODE_READS_PER_LOG];
	led_current_batch led_current_data				[idle_MAX_READS_PER_LOG / idle_LED_CURRENT_READS_PER_LOG];
	gyro_batch gyro_data							[idle_MAX_READS_PER_LOG / idle_GYRO_READS_PER_LOG];
	magnetometer_batch magnetometer_data			[idle_MAX_READS_PER_LOG / idle_MAGNETOMETER_READS_PER_LOG];
	charging_batch charging_data					[idle_MAX_READS_PER_LOG / idle_CHARGING_DATA_READS_PER_LOG];
	radio_temp_batch radio_temp_data				[idle_MAX_READS_PER_LOG / idle_RADIO_TEMP_READS_PER_LOG];
	battery_voltages_batch battery_voltages_data	[idle_MAX_READS_PER_LOG / idle_BAT_VOLTAGE_READS_PER_LOG];
	regulator_voltages_batch regulator_voltages_data[idle_MAX_READS_PER_LOG / idle_REG_VOLTAGE_READS_PER_LOG];
	
} idle_data_t;

#endif