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
lion_volts_batch read_lion_volts_batch(void) {};
lion_current_batch read_lion_current_batch(void) {};
led_temps_batch read_led_temps_batch(void) {};
lifepo_current_batch read_lifepo_current_batch(void) {};
lifepo_volts_batch read_lifepo_volts_batch(void) {};
diode_batch read_diode_batch(void) {};
bat_temp_batch read_bat_temp_batch(void) {};
ir_temps_batch read_ir_temps_batch(void) {};
radio_temp_batch read_radio_temp_batch(void) {};
imu_batch read_imu_batch(void) {};
magnetometer_batch read_magnetometer_batch(void) {};
led_current_batch read_led_current_batch(void) {};
radio_volts_batch read_radio_volts_batch(void) {};
bat_charge_volts_batch read_bat_charge_volts_batch(void) {};
bat_charge_dig_sigs_batch read_bat_charge_dig_sigs_batch(void) {};
digital_out_batch read_digital_out_batch(void) {};