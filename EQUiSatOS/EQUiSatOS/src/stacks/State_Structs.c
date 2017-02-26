/*
 * State_Structs.c
 *
 * Created: 11/6/2016 2:22:53 PM
 *  Author: jleiken
 */ 

#include "State_Structs.h"

void free_low_power(low_power_data_t* lpd){
	for (int i = 0; i < sizeof(lpd->ir_data); i++)
	{
		free_ir(&(lpd->ir_data[i]));
	}
	vPortFree(&(lpd->ir_data));
	for (int i = 0; i < sizeof(lpd->temp_data); i++)
	{
		free_temp(&(lpd->temp_data[i]));
	}
	vPortFree(&(lpd->temp_data));
	for (int i = 0; i < sizeof(lpd->diode_data); i++)
	{
		free_diode(&(lpd->diode_data[i]));
	}
	vPortFree(&(lpd->diode_data));
	for (int i = 0; i < sizeof(lpd->led_current_data); i++)
	{
		free_led(&(lpd->led_current_data[i]));
	}
	vPortFree(&(lpd->led_current_data));
	for (int i = 0; i < sizeof(lpd->imu_data); i++)
	{
		free_imu(&(lpd->imu_data[i]));
	}
	vPortFree(&(lpd->imu_data));
	for (int i = 0; i < sizeof(lpd->charging_data); i++)
	{
		free_charging(&(lpd->charging_data[i]));
	}
	vPortFree(&(lpd->charging_data));
	for (int i = 0; i < sizeof(lpd->radio_temp_data); i++)
	{
		free_radio_temp(&(lpd->radio_temp_data[i]));
	}
	vPortFree(&(lpd->radio_temp_data));
	for (int i = 0; i < sizeof(lpd->battery_voltages_data); i++)
	{
		free_battery_volt(&(lpd->battery_voltages_data[i]));
	}
	vPortFree(&(lpd->battery_voltages_data));
	for (int i = 0; i < sizeof(lpd->regulator_voltages_data); i++)
	{
		free_regulator_volt(&(lpd->regulator_voltages_data[i]));
	}
	vPortFree(&(lpd->regulator_voltages_data));
	vPortFree(&lpd);
}

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
	for (int i = 0; i < sizeof(fd->ir_data); i++)
	{
		free_ir(&(fd->ir_data[i]));
	}
	vPortFree(&(fd->ir_data));
	for (int i = 0; i < sizeof(fd->temp_data); i++)
	{
		free_temp(&(fd->temp_data[i]));
	}
	vPortFree(&(fd->temp_data));
	for (int i = 0; i < sizeof(fd->diode_data); i++)
	{
		free_diode(&(fd->diode_data[i]));
	}
	vPortFree(&(fd->diode_data));
	for (int i = 0; i < sizeof(fd->led_current_data); i++)
	{
		free_led(&(fd->led_current_data[i]));
	}
	vPortFree(&(fd->led_current_data));
	for (int i = 0; i < sizeof(fd->imu_data); i++)
	{
		free_imu(&(fd->imu_data[i]));
	}
	vPortFree(&(fd->imu_data));
	for (int i = 0; i < sizeof(fd->charging_data); i++)
	{
		free_charging(&(fd->charging_data[i]));
	}
	vPortFree(&(fd->charging_data));
	for (int i = 0; i < sizeof(fd->radio_temp_data); i++)
	{
		free_radio_temp(&(fd->radio_temp_data[i]));
	}
	vPortFree(&(fd->radio_temp_data));
	for (int i = 0; i < sizeof(fd->battery_voltages_data); i++)
	{
		free_battery_volt(&(fd->battery_voltages_data[i]));
	}
	vPortFree(&(fd->battery_voltages_data));
	for (int i = 0; i < sizeof(fd->regulator_voltages_data); i++)
	{
		free_regulator_volt(&(fd->regulator_voltages_data[i]));
	}

	vPortFree(&(fd->regulator_voltages_data));
	vPortFree(&fd);
}
