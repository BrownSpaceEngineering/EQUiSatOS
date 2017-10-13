/*
 * struct_tests.c
 *
 * Created: 11/13/2016 12:03:53 PM
 *  Author: jleiken
 */

#include "struct_tests.h"

idle_data_t init_test_struct(void){
	idle_data_t IDT_ORIG;
	idle_data_t *IDT = &IDT_ORIG;

	IDT->ir_data[0][11] = 5;

	IDT->ir_temps_data[0][5] = 5;

	IDT->diode_data[0][5] = 5;

	IDT->led_current_data[0][3] = 5;

	imu_batch gb;
	/*gb.timestamp = 65;*/
	gb.accelerometer[0] = 5;
	gb.gyro[0] = 5;
	gb.magnetometer[0] = 5;
	IDT->imu_data[0] = gb;

	IDT->bat_charge_volts_data[0][13] = 5;

	IDT->radio_temp_data[0] = 5;

	IDT->lion_volts_data[0][0] = 5;

	bat_charge_dig_sigs_batch rvb;
	rvb = 0b00101001;
	IDT->bat_charge_dig_sigs_data[0] = rvb;
}
