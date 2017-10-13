/*
 * sensor_read_commands.c
 *
 * Created: 11/1/2016 8:18:16 PM
 *  Author: mckenna
 */ 

#include "sensor_read_commands.h"

/* NOTE: the "batch" value passed into these functions are generally arrays, so are passed by reference */

read_ir_batch(ir_batch batch) {
	for (int i = 0; i < 12; i++)
	{
		batch[i] = i;
	}
}
read_lion_volts_batch(lion_volts_batch batch) {
	
};

read_lion_current_batch(lion_current_batch batch) {
	
};

read_led_temps_batch(led_temps_batch batch) {
		
};

read_lifepo_current_batch(lifepo_current_batch batch) {
		
};

read_lifepo_volts_batch(lifepo_volts_batch batch) {
		
};

read_diode_batch(diode_batch batch) {
	
};

read_bat_temp_batch(bat_temp_batch batch) {
		
};

read_ir_temps_batch(ir_temps_batch batch) {
		
};

read_radio_temp_batch(radio_temp_batch batch) {
		
};

read_imu_batch(imu_batch *batch) { // only one that's a struct not an array ref, so make sure it passes by reference too
	imu_batch imu;
	batch->accelerometer[0] = 5;
	batch->gyro[0] = 5;
	batch->magnetometer[0] = 5;
};	

read_magnetometer_batch(magnetometer_batch batch) {
	
};	

read_led_current_batch(led_current_batch batch) {
	
};
	
read_radio_volts_batch(radio_volts_batch batch) {
		
};

read_bat_charge_volts_batch(bat_charge_volts_batch batch) {
	
};

read_bat_charge_dig_sigs_batch(bat_charge_dig_sigs_batch batch) {
	
};
	
read_digital_out_batch(digital_out_batch batch) {
		
};