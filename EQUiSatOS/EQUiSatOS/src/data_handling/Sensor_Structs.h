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
typedef uint8_t lion_temps_batch			[2];
typedef uint8_t led_temps_batch				[4];
typedef uint8_t lifepo_current_batch		[4];
typedef uint8_t lifepo_volts_batch			[4];
typedef uint8_t lifepo_bank_temps_batch		[2];
typedef uint16_t ir_ambient_temps_batch		[6];
typedef uint8_t radio_temp_batch;			// single value
typedef uint8_t radio_current_batch;		// single value
typedef uint8_t radio_volts_batch;			// single value
typedef uint8_t proc_temp_batch;			// single value
typedef uint8_t bat_charge_volts_batch		[2];
typedef uint8_t satellite_history_batch;	// single value
typedef uint16_t bat_charge_dig_sigs_batch;	// single value
typedef uint16_t digital_out_batch;			// single value // TODO: still exists?
typedef uint16_t ir_object_temps_batch		[6];
typedef uint8_t pdiode_batch				[6];
typedef uint8_t led_current_batch			[4];
typedef uint8_t accelerometer_batch			[3];
typedef uint8_t gyro_batch					[3];
typedef uint8_t magnetometer_batch			[3];
typedef uint8_t imu_temp_batch;				// single value
typedef uint8_t rail_5v_batch;				// single value

#endif
