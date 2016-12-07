/*
 * Sensor_Structs.c
 *
 * Created: 11/15/2016 9:02:03 PM
 *  Author: jleiken
 */ 

#include "Sensor_Structs.h"

void free_error(error_t* et){
	vPortFree(&et);
}

void free_ir(ir_batch* ib){
	vPortFree(&(ib->values));
	vPortFree(&ib);
}

void free_temp(temp_batch* tb){
	vPortFree(&(tb->values));
	vPortFree(&tb);
}

void free_diode(diode_batch* db){
	vPortFree(&(db->values));
	vPortFree(&db);
}

void free_led(led_current_batch* lcb){
	vPortFree(&(lcb->values));
	vPortFree(&lcb);
}

void free_gyro(gyro_batch* gb){
	vPortFree(&(gb->values));
	vPortFree(&gb);
}

void free_magnetometer(magnetometer_batch* mb){
	vPortFree(&mb);
}

void free_charging(charging_batch* cb){
	vPortFree(&(cb->values));
	vPortFree(&cb);
}

void free_radio_temp(radio_temp_batch* rtb){
	vPortFree(&rtb);
}

void free_battery_volt(battery_voltages_batch* bvb){
	vPortFree(&(bvb->values));
	vPortFree(&bvb);
}

void free_regulator_volt(regulator_voltages_batch* rvb){
	vPortFree(&(rvb->values));
	vPortFree(&rvb);
}