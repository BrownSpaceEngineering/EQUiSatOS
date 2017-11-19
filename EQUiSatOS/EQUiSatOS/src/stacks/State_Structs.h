/*
 * State_Structs.h
 *
 * Created: 10/30/2016 2:02:48 PM
 *  Author: mcken
 */

#ifndef STATE_STRUCTS_H
#define STATE_STRUCTS_H

#include "Sensor_Structs.h"
#include "../rtos_tasks/rtos_tasks_config.h"
#include <global.h>

// for packet header
typedef struct current_data_t
{
	lion_volts_batch lion_volts_data;
	lion_current_batch lion_current_data;
	bat_charge_volts_batch bat_charge_volts_data;
	bat_charge_dig_sigs_batch bat_charge_dig_sigs_data;
	digital_out_batch digital_out_data;

} current_data_t;

// for idle data package
typedef struct idle_data_t
{
	lion_temps_batch lion_temps_data;
	radio_volts_batch radio_volts_data;
	radio_temp_batch radio_temp_data;
	imu_temp_batch imu_temp_data;
	ir_ambient_temps_batch ir_temps_data;
	rail_5v_batch rail_5v_data;

	uint32_t timestamp;
	
} idle_data_t;

typedef struct attitude_data_t
{
	/* lists sizes are proportional to the max LOOPS_PER_LOG
	(because the higher the MAX number of LOOPS_PER_LOG, the lower the
	frequency of the least frequent task, and the larger our higher-frequency
	arrays need to be to get that one element of lower frequency data).
	They are also inversely proportional to the LOOPS_PER_LOG of each
	corresponding sensor because if MORE reads are required for each log,
	the data is of LOWER frequency and thus needs a SMALLER array. */
	ir_object_temps_batch ir_data					[attitude_IR_DATA_ARR_LEN];
	pdiode_batch diode_data							[attitude_DIODE_DATA_ARR_LEN];
	accelerometer_batch accelerometer_data			[attitude_ACCELEROMETER_DATA_ARR_LEN];
	gyro_batch gyro_data							[attitude_GYRO_DATA_ARR_LEN];
	magnetometer_batch magnetometer_data			[attitude_MAGNETOMETER_DATA_ARR_LEN];

	uint32_t timestamp;

} attitude_data_t;

typedef struct flash_data_t
{
	/* ibid */
	led_temps_batch led_temps_data						[FLASH_DATA_ARR_LEN];
	lifepo_current_batch lifepo_current_data			[FLASH_DATA_ARR_LEN];
	lifepo_volts_batch lifepo_volts_data				[FLASH_DATA_ARR_LEN];
	led_current_batch led_current_data					[FLASH_DATA_ARR_LEN];

	uint32_t timestamp;

} flash_data_t;

typedef struct flash_cmp_data_t
{
	/* ibid */
	/* NOTE: Though these are the same types as in flash_data_t, they are those values AVERAGED */
	led_temps_batch led_temps_avg_data						[FLASH_CMP_DATA_ARR_LEN];
	led_current_batch led_current_avg_data					[FLASH_CMP_DATA_ARR_LEN];
	lifepo_current_batch lifepo_current_avg_data			[FLASH_CMP_DATA_ARR_LEN];
	lifepo_volts_batch lifepo_volts_avg_data				[FLASH_CMP_DATA_ARR_LEN];

	uint32_t timestamp;

} flash_cmp_data_t;

typedef struct low_power_data_t
{
	lion_volts_batch lion_volts_data;
	lion_current_batch lion_current_data;
	lion_temps_batch lion_temps_data;
	bat_charge_volts_batch bat_charge_volts_data;
	bat_charge_dig_sigs_batch bat_charge_dig_sigs_data;
	ir_object_temps_batch ir_object_temps_data;
	gyro_batch gyro_data;

	uint32_t timestamp;

} low_power_data_t;

#endif
