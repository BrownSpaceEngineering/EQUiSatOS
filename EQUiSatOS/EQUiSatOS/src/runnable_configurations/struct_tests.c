/*
 * struct_tests.c
 *
 * Created: 11/13/2016 12:03:53 PM
 *  Author: jleiken
 */

#include "struct_tests.h"

void init_test_struct(void){
	current_data_t CDT_ORIG;
	current_data_t* CDT = &CDT_ORIG;
	idle_data_t IDT_ORIG;
	idle_data_t *IDT = &IDT_ORIG;

	CDT->lion_volts_data[1] = 5;

	CDT->lion_current_data[1] = 5;

	CDT->bat_charge_volts_data[1] = 5;

	bat_charge_dig_sigs_batch rvb;
	rvb = 0b0010100010010101;
	CDT->bat_charge_dig_sigs_data = rvb;
	
	digital_out_batch dob;
	dob = 0b1010100101011011;
	CDT->digital_out_data = dob;

	IDT->lion_temps_data[3] = 5;
	
	IDT->radio_volts_data = 5;
	
	IDT->radio_temp_data = 5;
	
	IDT->proc_temp_data = 5;
	
	IDT->ir_amb_temps_data[5] = 5;

	IDT->rail_5v_data = 5;
}
