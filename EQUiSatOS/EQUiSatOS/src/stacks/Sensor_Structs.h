/*
 * Sensor_Structs.h
 *
 * Created: 9/27/2016 9:42:32 PM
 *  Author: rj16
 */ 

#ifndef SENSOR_STRUCTS_H
#define SENSOR_STRUCTS_H

#include <global.h>

typedef uint16_t* lion_volts_batch;				// array of size 2
typedef uint16_t* lion_current_batch;			// array of size 2
typedef uint16_t* led_temps_batch;				// array of size 4
typedef uint16_t* lifepo_current_batch;			// array of size 4
typedef uint16_t* lifepo_volts_batch;			// array of size 4
typedef uint16_t* bat_temp_batch;				// array of size 4
typedef uint16_t* ir_temps_batch;				// array of size 6
typedef uint16_t* radio_volts_batch;			// array of size 2
typedef uint16_t* bat_charge_volts_batch;		// array of size 14
typedef uint16_t bat_charge_dig_sigs_batch;		// single value
typedef uint16_t digital_out_batch;				// single value
typedef uint16_t* ir_batch;						// array of size 12
typedef uint8_t* diode_batch;					// array of size 6
typedef uint16_t* led_current_batch;			// array of size 4
typedef uint16_t* magnetometer_batch;			// array of size 3 (one for each axis)
typedef struct imu_batch
{
	// one for each axis, for each sensor
	uint16_t accelerometer[3];
	uint16_t gyro[3];
	uint16_t magnetometer[3];
} imu_batch;
typedef uint8_t radio_temp_batch;				// single value

#endif
