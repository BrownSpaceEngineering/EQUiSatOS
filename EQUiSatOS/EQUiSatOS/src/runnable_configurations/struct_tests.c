/*
 * struct_tests.c
 *
 * Created: 11/13/2016 12:03:53 PM
 *  Author: jleiken
 */ 

#include "struct_tests.h"

idle_data_t init_test_struct(void){
	idle_data_t *IDT = pvPortMalloc(sizeof(idle_data_t));

	ir_batch ib; // ir_batch *ib = pvPortMalloc(sizeof(ir_batch));
	/*ib.timestamp = 65;*/ // ib->timestamp = 65;
	ib.values[0] = 5; // ib->values[0] = 5;
	IDT->ir_data[0] = ib; // BDT->ir_data[0] = *ib;
	
	ir_temps_batch tb;
	/*tb.timestamp = 65;*/
	tb.values[0] = 5;
	IDT->ir_temps_data[0] = tb;
	
	diode_batch db;
	/*db.timestamp = 65;*/
	db.values[0] = 5;
	IDT->diode_data[0] = db;
	
	led_current_batch lcb;
	/*lcb.timestamp = 65;*/
	lcb.values[0] = 5;
	IDT->led_current_data[0] = lcb;
	
	imu_batch gb;
	/*gb.timestamp = 65;*/
	gb.accelerometer[0] = 5;
	gb.gyro[0] = 5;
	gb.magnetometer[0] = 5;
	IDT->imu_data[0] = gb;
	
	bat_charge_volts_batch cb;
	/*cb.timestamp = 65;*/
	cb.values[0] = 5;
	IDT->bat_charge_volts_data[0] = cb;
	
	radio_temp_batch rtb;
	/*rtb.timestamp = 65;*/
	rtb.value = 5;
	IDT->radio_temp_data[0] = rtb;
	
	lion_volts_batch bvb;
	/*bvb.timestamp = 65;*/
	bvb.values[0] = 5;
	IDT->lion_volts_data[0] = bvb;
	
	bat_charge_dig_sigs_batch rvb;
	/*rvb.timestamp = 65;*/
	rvb.value = 0b00101001;
	IDT->bat_charge_dig_sigs_data[0] = rvb;
}