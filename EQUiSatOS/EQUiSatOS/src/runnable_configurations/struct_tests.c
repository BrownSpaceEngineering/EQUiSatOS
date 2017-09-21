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

	ir_batch ib; // ir_batch *ib = pvPortMalloc(sizeof(ir_batch));
	ib[0] = 5; // ib->values[0] = 5;
	IDT->ir_data[0] = ib; // BDT->ir_data[0] = *ib;

	ir_temps_batch tb;
	tb[0] = 5;
	IDT->ir_temps_data[0] = tb;

	diode_batch db;
	db[0] = 5;
	IDT->diode_data[0] = db;

	led_current_batch lcb;
	lcb[0] = 5;
	IDT->led_current_data[0] = lcb;

	imu_batch gb;
	/*gb.timestamp = 65;*/
	gb.accelerometer[0] = 5;
	gb.gyro[0] = 5;
	gb.magnetometer[0] = 5;
	IDT->imu_data[0] = gb;

	bat_charge_volts_batch cb;
	cb[0] = 5;
	IDT->bat_charge_volts_data[0] = cb;

	radio_temp_batch rtb;
	rtb = 5;
	IDT->radio_temp_data[0] = rtb;

	lion_volts_batch bvb;
	bvb[0] = 5;
	IDT->lion_volts_data[0] = bvb;

	bat_charge_dig_sigs_batch rvb;
	rvb = 0b00101001;
	IDT->bat_charge_dig_sigs_data[0] = rvb;
}
