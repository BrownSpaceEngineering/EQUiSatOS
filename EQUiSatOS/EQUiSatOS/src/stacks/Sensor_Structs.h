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
// TODO: Does it make sense to wrap these arrays in structs?????

typedef struct ir_batch
{
	uint16_t values[12];
} ir_batch;
void free_ir(ir_batch*);

typedef struct temp_batch
{
	uint16_t values[8];
} temp_batch;
void free_temp(temp_batch*);

typedef struct diode_batch
{
	uint8_t values[6];
} diode_batch;
void free_diode(diode_batch*);

typedef struct led_current_batch
{
	uint16_t values[4];
} led_current_batch;
void free_led(led_current_batch*);

typedef struct gyro_batch
{
	// one for each axis
	uint16_t values[3];
} gyro_batch;
void free_gyro(gyro_batch*);

typedef struct magnetometer_batch
{
	// one for each axis
	uint16_t values[3];
} magnetometer_batch;
void free_magnetometer(magnetometer_batch*);

typedef struct charging_batch
{
	uint32_t values[8];
} charging_batch;
void free_charging(charging_batch*);

typedef struct radio_temp_batch
{
	uint8_t value;
} radio_temp_batch;
void free_radio_temp(radio_temp_batch*);

typedef struct battery_voltages_batch
{
	uint16_t values[6];
} battery_voltages_batch;
void free_battery_volt(battery_voltages_batch*);

typedef struct regulator_voltages_batch
{
	uint16_t values[3];
} regulator_voltages_batch;

void free_regulator_volt(regulator_voltages_batch*);

#endif
