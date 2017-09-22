/*
 * State_Structs.h
 *
 * Created: 10/30/2016 2:02:48 PM
 *  Author: mcken
 */ 

#ifndef STATE_STRUCTS_H
#define STATE_STRUCTS_H

#include "Sensor_Structs.h"
#include "../rtos_tasks/rtos_task_frequencies.h"
#include <global.h>

typedef struct idle_data_t
{
	uint32_t timestamp;
	/* lists sizes are proportional to the max LOOPS_PER_LOG
	(because the higher the MAX number of LOOPS_PER_LOG, the lower the 
	frequency of the least frequent task, and the larger our higher-frequency 
	arrays need to be to get that one element of lower frequency data).
	They are also inversely proportional to the LOOPS_PER_LOG of each 
	corresponding sensor because if MORE reads are required for each log,
	the data is of LOWER frequency and thus needs a SMALLER array. */
	
	/* Helpful regex:
	 *
	 */
	lion_volts_batch lion_volts_data 							[idle_LION_VOLTS_DATA_ARR_LEN];
	lion_current_batch lion_current_data						[idle_LION_CURRENT_DATA_ARR_LEN];
	led_temps_batch led_temps_data 								[idle_LED_TEMPS_DATA_ARR_LEN];
	lifepo_current_batch lifepo_current_data 					[idle_LIFEPO_CURRENT_DATA_ARR_LEN];
	ir_batch ir_data 											[idle_IR_DATA_ARR_LEN];
	diode_batch diode_data 										[idle_DIODE_DATA_ARR_LEN];
	bat_temp_batch bat_temp_data 								[idle_BAT_TEMP_DATA_ARR_LEN];
	ir_temps_batch ir_temps_data 								[idle_IR_TEMPS_DATA_ARR_LEN];
	radio_temp_batch radio_temp_data 							[idle_RADIO_TEMP_DATA_ARR_LEN];
	imu_batch imu_data 											[idle_IMU_DATA_ARR_LEN];
	magnetometer_batch magnetometer_data 						[idle_MAGNETOMETER_DATA_ARR_LEN];
	led_current_batch led_current_data 							[idle_LED_CURRENT_DATA_ARR_LEN];
	radio_volts_batch radio_volts_data 							[idle_RADIO_VOLTS_DATA_ARR_LEN];
	bat_charge_volts_batch bat_charge_volts_data 				[idle_BAT_CHARGE_VOLTS_DATA_ARR_LEN];
	bat_charge_dig_sigs_batch bat_charge_dig_sigs_data 			[idle_BAT_CHARGE_DIG_SIGS_DATA_ARR_LEN];
	digital_out_batch digital_out_data 							[idle_DIGITAL_OUT_DATA_ARR_LEN];
	
	
} idle_data_t;

typedef struct flash_data_t
{
	uint32_t timestamp;
	/* ibid */
	led_temps_batch led_temps_data						[flash_LED_TEMPS_DATA_ARR_LEN];
	led_current_batch led_current_data					[flash_LED_CURRENT_DATA_ARR_LEN];
	lifepo_current_batch lifepo_current_data			[flash_LIFEPO_CURRENT_DATA_ARR_LEN];
	lifepo_volts_batch lifepo_volts_data				[flash_LIFEPO_VOLTS_DATA_ARR_LEN];
	
} flash_data_t;

typedef struct transmit_data_t
{
	uint32_t timestamp;
	/* ibid */
	radio_temp_batch radio_temp_data				[transmit_RADIO_TEMP_DATA_ARR_LEN];
	lion_current_batch lion_current_data			[transmit_LION_CURRENT_DATA_ARR_LEN];
	lion_volts_batch lion_volts_data				[transmit_LION_VOLTS_DATA_ARR_LEN];
	
} transmit_data_t;

typedef struct attitude_data_t
{
	uint32_t timestamp;
	/* ibid */
	ir_batch ir_data								[attitude_IR_DATA_ARR_LEN];
	diode_batch diode_data							[attitude_DIODE_DATA_ARR_LEN];
	imu_batch imu_data								[attitude_IMU_DATA_ARR_LEN];
	magnetometer_batch magnetometer_data			[attitude_MAGNETOMETER_DATA_ARR_LEN];
	
} attitude_data_t;

#endif