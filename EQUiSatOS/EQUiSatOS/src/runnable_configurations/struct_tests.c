/*
 * struct_tests.c
 *
 * Created: 11/13/2016 12:03:53 PM
 *  Author: jleiken
 */ 

#include "struct_tests.h"

boot_data_t init_test_struct(void){
	boot_data_t *BDT = pvPortMalloc(sizeof(boot_data_t));

	ir_batch ib; // ir_batch *ib = pvPortMalloc(sizeof(ir_batch));
	/*ib.timestamp = 65;*/ // ib->timestamp = 65;
	ib.values[0] = 5; // ib->values[0] = 5;
	BDT.ir_data[0] = ib; // BDT->ir_data[0] = *ib;
	
	temp_batch tb;
	/*tb.timestamp = 65;*/
	tb.values[0] = 5;
	BDT->temp_data[0] = tb;
	
	diode_batch db;
	/*db.timestamp = 65;*/
	db.values[0] = 5;
	BDT->diode_data[0] = db;
	
	led_current_batch lcb;
	/*lcb.timestamp = 65;*/
	lcb.values[0] = 5;
	BDT->led_current_data[0] = lcb;
	
	gyro_batch gb;
	/*gb.timestamp = 65;*/
	gb.values[0] = 5;
	BDT->gyro_data[0] = gb;
	
	magnetometer_batch mb;
	/*mb.timestamp = 65;*/
	mb.value = 5;
	BDT->magnetometer_data[0] = mb;
	
	charging_batch cb;
	/*cb.timestamp = 65;*/
	cb.values[0] = 5;
	BDT->charging_data[0] = cb;
	
	radio_temp_batch rtb;
	/*rtb.timestamp = 65;*/
	rtb.value = 5;
	BDT->radio_temp_data[0] = rtb;
	
	battery_voltages_batch bvb;
	/*bvb.timestamp = 65;*/
	bvb.values[0] = 5;
	BDT->battery_voltages_data[0] = bvb;
	
	regulator_voltages_batch rvb;
	/*rvb.timestamp = 65;*/
	rvb.values[0] = 5;
	BDT->regulator_voltages_data[0] = rvb;
}

void test_free(void *pvParameters){
	
	// initialize xNextWakeTime onces
	TickType_t xNextWakeTime = xTaskGetTickCount();
	for (;;)
	{
		vTaskDelayUntil( &xNextWakeTime, 1000 / portTICK_PERIOD_MS);
		boot_data_t bdt = init_test_struct();
		free_boot(&bdt);
	}
	int local_show = 5;
}