/*
 * struct_tests.c
 *
 * Created: 11/13/2016 12:03:53 PM
 *  Author: jleiken
 */

#include "struct_tests.h"

void init_test_struct(void){
	idle_data_t IDT_ORIG;
	idle_data_t *IDT = &IDT_ORIG;

	IDT->lion_temps_data[3] = 5;
	
	IDT->radio_temp_data = 5;
	
	IDT->imu_temp_data = 5;
	
	IDT->ir_amb_temps_data[5] = 5;
}
