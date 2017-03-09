/*
 * State_Structs.c
 *
 * Created: 11/6/2016 2:22:53 PM
 *  Author: jleiken
 */ 

#include "State_Structs.h"
// DEPRECATED
 void free_idle(idle_data_t* id){
	 for (int i = 0; i < sizeof(id->ir_data); i++)
	 {
		 free_ir(&(id->ir_data[i]));
	 }
	 vPortFree(&(id->ir_data));
	 for (int i = 0; i < sizeof(id->temp_data); i++)
	 {
		 free_temp(&(id->temp_data[i]));
	 }
	 vPortFree(&(id->temp_data));
	 for (int i = 0; i < sizeof(id->diode_data); i++)
	 {
		 free_diode(&(id->diode_data[i]));
	 }
	 vPortFree(&(id->diode_data));
	 for (int i = 0; i < sizeof(id->led_current_data); i++)
	 {
		 free_led(&(id->led_current_data[i]));
	 }
	 vPortFree(&(id->led_current_data));
	 for (int i = 0; i < sizeof(id->imu_data); i++)
	 {
		 free_imu(&(id->imu_data[i]));
	 }
	 vPortFree(&(id->imu_data));
	 for (int i = 0; i < sizeof(id->charging_data); i++)
	 {
		 free_charging(&(id->charging_data[i]));
	 }
	 vPortFree(&(id->charging_data));
	 for (int i = 0; i < sizeof(id->radio_temp_data); i++)
	 {
		 free_radio_temp(&(id->radio_temp_data[i]));
	 }
	 vPortFree(&(id->radio_temp_data));
	 for (int i = 0; i < sizeof(id->battery_voltages_data); i++)
	 {
		 free_battery_volt(&(id->battery_voltages_data[i]));
	 }
	 vPortFree(&(id->battery_voltages_data));
	 for (int i = 0; i < sizeof(id->regulator_voltages_data); i++)
	 {
		 free_regulator_volt(&(id->regulator_voltages_data[i]));
	 }
	 vPortFree(&(id->regulator_voltages_data));
	 vPortFree(&id);
 }
 
 void free_flash(flash_data_t* fd){
	 for (int i = 0; i < sizeof(fd->temp_data); i++)
	 {
		 free_temp(&(fd->temp_data[i]));
	 }
	 vPortFree(&(fd->temp_data));
	 for (int i = 0; i < sizeof(fd->led_current_data); i++)
	 {
		 free_led(&(fd->led_current_data[i]));
	 }
	 vPortFree(&(fd->led_current_data));
	 for (int i = 0; i < sizeof(fd->battery_voltages_data); i++)
	 {
		 free_battery_volt(&(fd->battery_voltages_data[i]));
	 }
	 vPortFree(&(fd->battery_voltages_data));
	 
	 vPortFree(&fd);
 }
