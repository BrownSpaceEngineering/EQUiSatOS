/*
 * State_Structs.h
 *
 * Created: 10/30/2016 2:02:48 PM
 *  Author: mcken
 */

#ifndef STATE_STRUCTS_H
#define STATE_STRUCTS_H

#include <global.h>
#include "Sensor_Structs.h"
#include "data_handling/persistent_storage.h"


// for packet header - TODO: Needed?
// typedef struct current_data_t
// {
// 	
// 	lion_volts_batch lion_volts_data;
// 	lion_current_batch lion_current_data;
// 	bat_charge_volts_batch bat_charge_volts_data;
// 	bat_charge_dig_sigs_batch bat_charge_dig_sigs_data;
// 
// } current_data_t;

// for idle data package
typedef struct idle_data_t
{
	satellite_history_batch satellite_history;
	uint8_t reboot_count;
	lion_volts_batch lion_volts_data;
	lion_current_batch lion_current_data;
	lion_temps_batch lion_temps_data;
	bat_charge_volts_batch bat_charge_volts_data;
	bat_charge_dig_sigs_batch bat_charge_dig_sigs_data;
	radio_temp_batch radio_temp_data;
	proc_temp_batch proc_temp_data;
	ir_ambient_temps_batch ir_amb_temps_data;

	uint32_t timestamp;
	bool transmitted; // for determining which data to transmit
	
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
	ir_object_temps_batch ir_obj_temps_data					[attitude_IR_DATA_ARR_LEN];
	pdiode_batch pdiode_data								[attitude_DIODE_DATA_ARR_LEN];
	accelerometer_batch accelerometer_data					[attitude_ACCELEROMETER_DATA_ARR_LEN];
	gyro_batch gyro_data									[attitude_GYRO_DATA_ARR_LEN];
	magnetometer_batch magnetometer_data					[attitude_MAGNETOMETER_DATA_ARR_LEN];

	uint32_t timestamp;
	bool transmitted;

} attitude_data_t;

typedef struct flash_data_t
{
	/* ibid */
	led_temps_batch led_temps_data						[FLASH_DATA_ARR_LEN];
	lifepo_bank_temps_batch lifepo_bank_temps_data		[FLASH_DATA_ARR_LEN];
	lifepo_current_batch lifepo_current_data			[FLASH_DATA_ARR_LEN];
	lifepo_volts_batch lifepo_volts_data				[FLASH_DATA_ARR_LEN];
	led_current_batch led_current_data					[FLASH_DATA_ARR_LEN];
	gyro_batch gyro_data								[FLASH_DATA_ARR_LEN];

	uint32_t timestamp;
	bool transmitted;

} flash_data_t;

typedef struct flash_cmp_data_t
{
	/* ibid */
	/* NOTE: Though these are the same types as in flash_data_t, they are those values AVERAGED */
	led_temps_batch led_temps_avg_data;
	lifepo_bank_temps_batch lifepo_bank_temps_avg_data;
	led_current_batch led_current_avg_data;
	lifepo_current_batch lifepo_current_avg_data;
	lifepo_volts_batch lifepo_volts_avg_data;
	magnetometer_batch mag_before_data;

	uint32_t timestamp;
	bool transmitted;

} flash_cmp_data_t;

// for low power data package
typedef struct low_power_data_t
{
	satellite_history_batch satellite_history;
	lion_volts_batch lion_volts_data;
	lion_current_batch lion_current_data;
	lion_temps_batch lion_temps_data;
	bat_charge_volts_batch bat_charge_volts_data;
	bat_charge_dig_sigs_batch bat_charge_dig_sigs_data;
	ir_object_temps_batch ir_obj_temps_data;
	gyro_batch gyro_data;

	uint32_t timestamp;
	bool transmitted;
	
} low_power_data_t;

#endif
