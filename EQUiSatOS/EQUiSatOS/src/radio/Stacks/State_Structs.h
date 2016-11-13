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

typedef struct flash_data_t
{
	/* ibid */
	ir_batch ir_data								[flash_MAX_READS_PER_LOG / flash_IR_READS_PER_LOG];
	temp_batch temp_data							[flash_MAX_READS_PER_LOG / flash_TEMP_READS_PER_LOG];
	diode_batch diode_data							[flash_MAX_READS_PER_LOG / flash_DIODE_READS_PER_LOG];
	led_current_batch led_current_data				[flash_MAX_READS_PER_LOG / flash_LED_CURRENT_READS_PER_LOG];
	gyro_batch gyro_data							[flash_MAX_READS_PER_LOG / flash_GYRO_READS_PER_LOG];
	magnetometer_batch magnetometer_data			[flash_MAX_READS_PER_LOG / flash_MAGNETOMETER_READS_PER_LOG];
	charging_batch charging_data					[flash_MAX_READS_PER_LOG / flash_CHARGING_DATA_READS_PER_LOG];
	radio_temp_batch radio_temp_data				[flash_MAX_READS_PER_LOG / flash_RADIO_TEMP_READS_PER_LOG];
	battery_voltages_batch battery_voltages_data	[flash_MAX_READS_PER_LOG / flash_BAT_VOLTAGE_READS_PER_LOG];
	regulator_voltages_batch regulator_voltages_data[flash_MAX_READS_PER_LOG / flash_REG_VOLTAGE_READS_PER_LOG];
	
} flash_data_t;

typedef struct low_power_data_t
{
	/* ibid */
	ir_batch ir_data								[low_power_MAX_READS_PER_LOG / low_power_IR_READS_PER_LOG];
	temp_batch temp_data							[low_power_MAX_READS_PER_LOG / low_power_TEMP_READS_PER_LOG];
	diode_batch diode_data							[low_power_MAX_READS_PER_LOG / low_power_DIODE_READS_PER_LOG];
	led_current_batch led_current_data				[low_power_MAX_READS_PER_LOG / low_power_LED_CURRENT_READS_PER_LOG];
	gyro_batch gyro_data							[low_power_MAX_READS_PER_LOG / low_power_GYRO_READS_PER_LOG];
	magnetometer_batch magnetometer_data			[low_power_MAX_READS_PER_LOG / low_power_MAGNETOMETER_READS_PER_LOG];
	charging_batch charging_data					[low_power_MAX_READS_PER_LOG / low_power_CHARGING_DATA_READS_PER_LOG];
	radio_temp_batch radio_temp_data				[low_power_MAX_READS_PER_LOG / low_power_RADIO_TEMP_READS_PER_LOG];
	battery_voltages_batch battery_voltages_data	[low_power_MAX_READS_PER_LOG / low_power_BAT_VOLTAGE_READS_PER_LOG];
	regulator_voltages_batch regulator_voltages_data[low_power_MAX_READS_PER_LOG / low_power_REG_VOLTAGE_READS_PER_LOG];
	
} low_power_data_t;

typedef struct boot_data_t
{
	/* ibid */
	//ir_batch ir_data								[boot_MAX_READS_PER_LOG / boot_IR_READS_PER_LOG];
	//temp_batch temp_data							[boot_MAX_READS_PER_LOG / boot_TEMP_READS_PER_LOG];
	//diode_batch diode_data							[boot_MAX_READS_PER_LOG / boot_DIODE_READS_PER_LOG];
	//led_current_batch led_current_data				[boot_MAX_READS_PER_LOG / boot_LED_CURRENT_READS_PER_LOG];
	//gyro_batch gyro_data							[boot_MAX_READS_PER_LOG / boot_GYRO_READS_PER_LOG];
	//magnetometer_batch magnetometer_data			[boot_MAX_READS_PER_LOG / boot_MAGNETOMETER_READS_PER_LOG];
	//charging_batch charging_data					[boot_MAX_READS_PER_LOG / boot_CHARGING_DATA_READS_PER_LOG];
	//radio_temp_batch radio_temp_data				[boot_MAX_READS_PER_LOG / boot_RADIO_TEMP_READS_PER_LOG];
	//battery_voltages_batch battery_voltages_data	[boot_MAX_READS_PER_LOG / boot_BAT_VOLTAGE_READS_PER_LOG];
	//regulator_voltages_batch regulator_voltages_data[boot_MAX_READS_PER_LOG / boot_REG_VOLTAGE_READS_PER_LOG];
	ir_batch ir_data								[1];
	temp_batch temp_data							[1];
	diode_batch diode_data							[1];
	led_current_batch led_current_data				[1];
	gyro_batch gyro_data							[1];
	magnetometer_batch magnetometer_data			[1];
	charging_batch charging_data					[1];
	radio_temp_batch radio_temp_data				[1];
	battery_voltages_batch battery_voltages_data	[1];
	regulator_voltages_batch regulator_voltages_data[1];
	
} boot_data_t;

void free_boot(boot_data_t*);
void free_low_power(low_power_data_t*);
void free_idle(idle_data_t*);
void free_flash(flash_data_t*);

#endif