/*
 * Sensor_Structs.h
 *
 * Created: 9/27/2016 9:42:32 PM
 *  Author: rj16
 */ 

#ifndef SENSOR_STRUCTS_H
#define SENSOR_STRUCTS_H

#include <asf.h>

// TODO: Do we want a state struct?
// TODO: Parity bits?
// TODO: Make sure these are the batches we want
// TODO: Does it make sense wrap these arrays in structs?????

typedef struct lion_volts_batch
{
	uint16_t values[2];
} lion_volts_batch;

typedef struct lion_current_batch
{
	uint16_t values[2];
} lion_current_batch;

typedef struct led_temps_batch
{
	uint16_t values[4];
} led_temps_batch;

typedef struct lifepo_current_batch
{
	uint16_t values[4];
} lifepo_current_batch;

typedef struct lifepo_volts_batch
{
	uint16_t values[4];
} lifepo_volts_batch;

typedef struct bat_temp_batch
{
	uint16_t values[4];
} bat_temp_batch;

typedef struct ir_temps_batch
{
	uint16_t values[6];
} ir_temps_batch;

typedef struct radio_volts_batch
{
	uint16_t values[2];
} radio_volts_batch;

typedef struct bat_charge_volts_batch
{
	uint16_t values[14];
} bat_charge_volts_batch;

typedef struct bat_charge_dig_sigs_batch
{
	uint16_t value;
} bat_charge_dig_sigs_batch;

typedef struct digital_out_batch
{
	uint16_t value;
} digital_out_batch;

typedef struct ir_batch
{
	uint16_t values[12];
} ir_batch;

typedef struct diode_batch
{
	uint8_t values[6];
} diode_batch;

typedef struct led_current_batch
{
	uint16_t values[4];
} led_current_batch;

typedef struct magnetometer_batch
{
	// one for each axis
	uint16_t values[3];
} magnetometer_batch;

typedef struct imu_batch
{
	// one for each axis, for each sensor
	uint16_t accelerometer[3];
	uint16_t gyro[3];
	uint16_t magnetometer[3];
} imu_batch;

typedef struct radio_temp_batch
{
	uint8_t value;
} radio_temp_batch;

#endif
