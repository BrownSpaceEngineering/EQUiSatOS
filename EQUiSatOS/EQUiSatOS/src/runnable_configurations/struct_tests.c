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

	IDT->lion_volts_data[0][1] = 5;

	IDT->lion_current_data[0][1] = 5;

	IDT->bat_charge_volts_data[0][13] = 5;

	bat_charge_dig_sigs_batch rvb;
	rvb = 0b0010100010010101;
	IDT->bat_charge_dig_sigs_data[0] = rvb;
	
	digital_out_batch dob;
	dob = 0b1010100101011011;
	IDT->bat_charge_dig_sigs_data[0] = dob;

	IDT->bat_temp_data[0][3] = 5;
	
	IDT->radio_volts_data[0][1] = 5;
	
	IDT->radio_temp_data[0] = 5;
	
	IDT->imu_temp_data[0] = 5;
	
	IDT->ir_temps_data[0][5] = 5;

	IDT->rail_3v_data[0] = 5;
	
	IDT->rail_3v_data[0] = 5;
}
