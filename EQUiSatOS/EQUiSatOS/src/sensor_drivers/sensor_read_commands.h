/*
 * sensor_read_commands.h
 *
 * Created: 11/1/2016 8:17:44 PM
 *  Author: mckenna
 */ 

#ifndef SENSOR_READ_COMMANDS_H
#define	SENSOR_READ_COMMANDS_H

#include "../stacks/Sensor_Structs.h"

ir_batch read_ir_batch(void);
temp_batch read_temp_batch(void);
diode_batch read_diode_batch(void);
led_current_batch read_led_current_batch(void);
gyro_batch read_gyro_batch(void);
magnetometer_batch read_magnetometer_batch(void);
charging_batch read_charging_batch(void);
radio_temp_batch read_radio_temp_batch(void);
battery_voltages_batch read_battery_voltages_batch(void);
regulator_voltages_batch read_regulator_voltages_batch(void);

#endif