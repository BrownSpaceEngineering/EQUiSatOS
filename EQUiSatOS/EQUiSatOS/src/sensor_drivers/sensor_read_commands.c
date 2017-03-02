/*
 * sensor_read_commands.c
 *
 * Created: 11/1/2016 8:18:16 PM
 *  Author: mckenna
 */ 

#include "sensor_read_commands.h"

ir_batch read_ir_batch()
{
	ir_batch batch;
	for (int i = 0; i < 12; i++)
	{
		batch.values[i] = i;
	}
	return batch;
}
temp_batch read_temp_batch() {};
diode_batch read_diode_batch() {};
led_current_batch read_led_current_batch() {};
imu_batch read_imu_batch() {};
magnetometer_batch read_magnetometer_batch() {};
charging_batch read_charging_batch() {};
radio_temp_batch read_radio_temp_batch() {};
battery_voltages_batch read_battery_voltages_batch() {};
regulator_voltages_batch read_regulator_voltages_batch() {};