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

	temp_batch tb;
	/*tb.timestamp = 65;*/
	tb.values[0] = 5;
	IDT->temp_data[0] = tb;

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

	charging_batch cb;
	/*cb.timestamp = 65;*/
	cb.values[0] = 5;
	IDT->charging_data[0] = cb;

	radio_temp_batch rtb;
	/*rtb.timestamp = 65;*/
	rtb.value = 5;
	IDT->radio_temp_data[0] = rtb;

	battery_voltages_batch bvb;
	/*bvb.timestamp = 65;*/
	bvb.values[0] = 5;
	IDT->battery_voltages_data[0] = bvb;

	regulator_voltages_batch rvb;
	/*rvb.timestamp = 65;*/
	rvb.values[0] = 5;
	IDT->regulator_voltages_data[0] = rvb;
}

void test_free(void *pvParameters){

	// initialize xNextWakeTime onces
	TickType_t xNextWakeTime = xTaskGetTickCount();
	for (;;)
	{
		vTaskDelayUntil( &xNextWakeTime, 1000 / portTICK_PERIOD_MS);
		idle_data_t idt = init_test_struct();
		free_idle(&idt);
	}
	int local_show = 5;
}
