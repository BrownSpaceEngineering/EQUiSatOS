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
typedef struct error_t 
{
	uint32_t timestamp;
	uint8_t id;
} error_t;

typedef struct ir_batch
{
	uint32_t timestamp;
	uint16_t values[12];
} ir_batch;

typedef struct temp_batch
{
	uint32_t timestamp;
	uint16_t values[12];
} temp_batch;

typedef struct diode_batch
{
	uint32_t timestamp;
	uint16_t values[6]
} diode_batch;

typedef struct led_current_batch
{
	uint32_t timestamp;
	uint16_t values[4]
} led_current_batch;

typedef struct gyro_batch
{
	
} gyro_batch;

typedef struct magnetometer_batch
{
	uint32_t timestamp;
	uint64_t value;
} magnetometer_batch;

typedef struct charging_batch
{
	uint32_t timestamp;
	uint32_t values[3];
} charging_batch;

typedef struct radio_temp_batch
{
	uint32_t timestamp;
	uint8_t value;
} radio_temp_batch;

typedef struct battery_voltages_batch
{
	uint32_t timestamp;
	uint16_t values[5];
} battery_voltages_batch;

typedef struct regulator_voltages_batch
{
	uint32_t timestamp;
	uint16_t values[3];
} regulator_voltages_batch;

#endif
