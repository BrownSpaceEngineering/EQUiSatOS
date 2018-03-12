/*
 * Sensor_Structs.h
 *
 * Created: 9/27/2016 9:42:32 PM
 *  Author: rj16
 */

#ifndef SENSOR_STRUCTS_H
#define SENSOR_STRUCTS_H

#include <global.h>

typedef uint8_t lion_volts_batch			[2];
typedef uint8_t lion_current_batch			[2];
typedef uint8_t panelref_lref_batch			[2];
typedef uint8_t lion_temps_batch			[2];
typedef uint8_t led_temps_batch				[4];
typedef uint8_t lifepo_current_batch		[4];
typedef uint8_t lifepo_volts_batch			[4];
typedef uint8_t led_current_batch			[4];
typedef uint8_t lifepo_bank_temps_batch		[2];
typedef uint8_t ir_ambient_temps_batch		[6];
typedef uint8_t radio_temp_batch;			// single value
typedef uint8_t imu_temp_batch;				// single value
typedef uint16_t ad7991_ctrlbrd_batch		[4];
typedef uint16_t bat_charge_dig_sigs_batch;	// single value
typedef uint16_t ir_object_temps_batch		[6];
typedef uint16_t pdiode_batch;				// single value; 2 bits per reading
typedef uint8_t accelerometer_batch			[3];
typedef uint8_t gyro_batch					[3];
typedef uint8_t magnetometer_batch			[3];
typedef struct satellite_history_batch
{
	bool antenna_deployed : 1;
	bool lion_1_charged : 1;
	bool lion_2_charged : 1;
	bool lifepo_b1_charged : 1;
	bool lifepo_b2_charged : 1;
	bool first_flash : 1;
	bool prog_mem_rewritten : 1;

} satellite_history_batch;


#endif
