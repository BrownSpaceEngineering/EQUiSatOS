/*
 * Sensor_Structs.c
 *
 * Created: 10/18/2016 8:54:51 PM
 *  Author: rj16
 */ 

#include "Sensor_Structs.h"

// TODO: Do we want a state struct?
// TODO: Parity bits?
// TODO: Make sure these are the batches we want
struct error 
{
	uint32_t timestamp;
	uint8_t id;
};

struct ir_batch
{
	uint32_t timestamp;
	uint16_t values[6];
};

struct temp_batch
{
	uint32_t timestamp;
	uint16_t values[12];
};

struct photo_diode_batch
{
	uint32_t timestamp;
	uint16_t values[6]
};

struct led_current_batch
{
	uint32_t timestamp;
	uint16_t values[4]
};

/*
struct gyro_batch
{
	
}; */

struct magnetometer_batch
{
	uint32_t timestamp;
	uint64_t value;
};

struct charging_batch
{
	uint32_t timestamp;
	uint32_t values[3];
};

struct radio_temp_batch
{
	uint32_t timestamp;
	uint8_t value;
};

struct battery_voltages_batch
{
	uint32_t timestamp;
	uint16_t values[5];
};

struct regulator_voltages_batch
{
	uint32_t timestamp;
	uint16_t values[3];
};