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
		batch[i] = i;
	}
	return batch;
}
lion_volts_batch read_lion_volts_batch(void) {
	return 0;
};

lion_current_batch read_lion_current_batch(void) {
	return 0;
};

led_temps_batch read_led_temps_batch(void) {
	return 0;	
};

lifepo_current_batch read_lifepo_current_batch(void) {
	return 0;	
};

lifepo_volts_batch read_lifepo_volts_batch(void) {
	return 0;	
};

diode_batch read_diode_batch(void) {
	return 0;
};

bat_temp_batch read_bat_temp_batch(void) {
	return 0;	
};

ir_temps_batch read_ir_temps_batch(void) {
	return 0;	
};

radio_temp_batch read_radio_temp_batch(void) {
	return 0;	
};

imu_batch read_imu_batch(void) {
	imu_batch imu;
	imu.accelerometer[0] = 5;
	imu.gyro[0] = 5;
	imu.magnetometer[0] = 5;
	return imu;
};	

magnetometer_batch read_magnetometer_batch(void) {
	return 0;
};	

led_current_batch read_led_current_batch(void) {
	return 0;
};
	
radio_volts_batch read_radio_volts_batch(void) {
	return 0;	
};

bat_charge_volts_batch read_bat_charge_volts_batch(void) {
	return 0;
};

bat_charge_dig_sigs_batch read_bat_charge_dig_sigs_batch(void) {
	return 0;
};
	
digital_out_batch read_digital_out_batch(void) {
	return 0;	
};