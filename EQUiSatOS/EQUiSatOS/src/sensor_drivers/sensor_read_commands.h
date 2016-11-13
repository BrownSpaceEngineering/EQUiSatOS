/*
 * sensor_read_commands.h
 *
 * Created: 11/1/2016 8:17:44 PM
 *  Author: mckenna
 */ 

#ifndef SENSOR_READ_COMMANDS_H
#define	SENSOR_READ_COMMANDS_H

#include "../radio/Stacks/Sensor_Structs.h"

ir_batch read_ir_batch();
temp_batch read_temp_batch();
diode_batch read_diode_batch();
led_current_batch read_led_current_batch();
gyro_batch read_gyro_batch();
magnetometer_batch read_magnetometer_batch();
charging_batch read_charging_batch();
radio_temp_batch read_radio_temp_batch();
battery_voltages_batch read_battery_voltages_batch();
regulator_voltages_batch read_regulator_voltages_batch();

#endif