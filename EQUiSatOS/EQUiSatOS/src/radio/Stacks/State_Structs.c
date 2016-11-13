/*
 * State_Structs.c
 *
 * Created: 11/6/2016 2:22:53 PM
 *  Author: jleiken
 */ 

#include "State_Structs.h"

void free_boot(boot_data_t* bd){
	for (int i = 0; i < sizeof(bd->ir_data); i++)
	{
		vPortFree(&(bd->ir_data[i]));
	}
	vPortFree(&(bd->ir_data));
	for (int i = 0; i < sizeof(bd->temp_data); i++)
	{
		vPortFree(&(bd->temp_data[i]));
	}
	vPortFree(&(bd->temp_data));
	for (int i = 0; i < sizeof(bd->diode_data); i++)
	{
		vPortFree(&(bd->diode_data[i]));
	}
	vPortFree(&(bd->diode_data));
	for (int i = 0; i < sizeof(bd->led_current_data); i++)
	{
		vPortFree(&(bd->led_current_data[i]));
	}
	vPortFree(&(bd->led_current_data));
	for (int i = 0; i < sizeof(bd->gyro_data); i++)
	{
		vPortFree(&(bd->gyro_data[i]));
	}
	vPortFree(&(bd->gyro_data));
	for (int i = 0; i < sizeof(bd->magnetometer_data); i++)
	{
		vPortFree(&(bd->magnetometer_data[i]));
	}
	vPortFree(&(bd->magnetometer_data));
	for (int i = 0; i < sizeof(bd->charging_data); i++)
	{
		vPortFree(&(bd->charging_data[i]));
	}
	vPortFree(&(bd->charging_data));
	for (int i = 0; i < sizeof(bd->radio_temp_data); i++)
	{
		vPortFree(&(bd->radio_temp_data[i]));
	}
	vPortFree(&(bd->radio_temp_data));
	for (int i = 0; i < sizeof(bd->battery_voltages_data); i++)
	{
		vPortFree(&(bd->battery_voltages_data[i]));
	}
	vPortFree(&(bd->battery_voltages_data));
	for (int i = 0; i < sizeof(bd->regulator_voltages_data); i++)
	{
		vPortFree(&(bd->regulator_voltages_data[i]));
	}
	vPortFree(&(bd->regulator_voltages_data));
	vPortFree(&bd);
}

void free_low_power(low_power_data_t* lpd){
	for (int i = 0; i < sizeof(lpd->ir_data); i++)
	{
		vPortFree(&(lpd->ir_data[i]));
	}
	vPortFree(&(lpd->ir_data));
	for (int i = 0; i < sizeof(lpd->temp_data); i++)
	{
		vPortFree(&(lpd->temp_data[i]));
	}
	vPortFree(&(lpd->temp_data));
	for (int i = 0; i < sizeof(lpd->diode_data); i++)
	{
		vPortFree(&(lpd->diode_data[i]));
	}
	vPortFree(&(lpd->diode_data));
	for (int i = 0; i < sizeof(lpd->led_current_data); i++)
	{
		vPortFree(&(lpd->led_current_data[i]));
	}
	vPortFree(&(lpd->led_current_data));
	for (int i = 0; i < sizeof(lpd->gyro_data); i++)
	{
		vPortFree(&(lpd->gyro_data[i]));
	}
	vPortFree(&(lpd->gyro_data));
	for (int i = 0; i < sizeof(lpd->magnetometer_data); i++)
	{
		vPortFree(&(lpd->magnetometer_data[i]));
	}
	vPortFree(&(lpd->magnetometer_data));
	for (int i = 0; i < sizeof(lpd->charging_data); i++)
	{
		vPortFree(&(lpd->charging_data[i]));
	}
	vPortFree(&(lpd->charging_data));
	for (int i = 0; i < sizeof(lpd->radio_temp_data); i++)
	{
		vPortFree(&(lpd->radio_temp_data[i]));
	}
	vPortFree(&(lpd->radio_temp_data));
	for (int i = 0; i < sizeof(lpd->battery_voltages_data); i++)
	{
		vPortFree(&(lpd->battery_voltages_data[i]));
	}
	vPortFree(&(lpd->battery_voltages_data));
	for (int i = 0; i < sizeof(lpd->regulator_voltages_data); i++)
	{
		vPortFree(&(lpd->regulator_voltages_data[i]));
	}
	vPortFree(&(lpd->regulator_voltages_data));
	vPortFree(&lpd);
}

void free_idle(idle_data_t* id){
	for (int i = 0; i < sizeof(id->ir_data); i++)
	{
		vPortFree(&(id->ir_data[i]));
	}
	vPortFree(&(id->ir_data));
	for (int i = 0; i < sizeof(id->temp_data); i++)
	{
		vPortFree(&(id->temp_data[i]));
	}
	vPortFree(&(id->temp_data));
	for (int i = 0; i < sizeof(id->diode_data); i++)
	{
		vPortFree(&(id->diode_data[i]));
	}
	vPortFree(&(id->diode_data));
	for (int i = 0; i < sizeof(id->led_current_data); i++)
	{
		vPortFree(&(id->led_current_data[i]));
	}
	vPortFree(&(id->led_current_data));
	for (int i = 0; i < sizeof(id->gyro_data); i++)
	{
		vPortFree(&(id->gyro_data[i]));
	}
	vPortFree(&(id->gyro_data));
	for (int i = 0; i < sizeof(id->magnetometer_data); i++)
	{
		vPortFree(&(id->magnetometer_data[i]));
	}
	vPortFree(&(id->magnetometer_data));
	for (int i = 0; i < sizeof(id->charging_data); i++)
	{
		vPortFree(&(id->charging_data[i]));
	}
	vPortFree(&(id->charging_data));
	for (int i = 0; i < sizeof(id->radio_temp_data); i++)
	{
		vPortFree(&(id->radio_temp_data[i]));
	}
	vPortFree(&(id->radio_temp_data));
	for (int i = 0; i < sizeof(id->battery_voltages_data); i++)
	{
		vPortFree(&(id->battery_voltages_data[i]));
	}
	vPortFree(&(id->battery_voltages_data));
	for (int i = 0; i < sizeof(id->regulator_voltages_data); i++)
	{
		vPortFree(&(id->regulator_voltages_data[i]));
	}
	vPortFree(&(id->regulator_voltages_data));
	vPortFree(&id);
}

void free_flash(flash_data_t* fd){
	for (int i = 0; i < sizeof(fd->ir_data); i++)
	{
		vPortFree(&(fd->ir_data[i]));
	}
	vPortFree(&(fd->ir_data));
	for (int i = 0; i < sizeof(fd->temp_data); i++)
	{
		vPortFree(&(fd->temp_data[i]));
	}
	vPortFree(&(fd->temp_data));
	for (int i = 0; i < sizeof(fd->diode_data); i++)
	{
		vPortFree(&(fd->diode_data[i]));
	}
	vPortFree(&(fd->diode_data));
	for (int i = 0; i < sizeof(fd->led_current_data); i++)
	{
		vPortFree(&(fd->led_current_data[i]));
	}
	vPortFree(&(fd->led_current_data));
	for (int i = 0; i < sizeof(fd->gyro_data); i++)
	{
		vPortFree(&(fd->gyro_data[i]));
	}
	vPortFree(&(fd->gyro_data));
	for (int i = 0; i < sizeof(fd->magnetometer_data); i++)
	{
		vPortFree(&(fd->magnetometer_data[i]));
	}
	vPortFree(&(fd->magnetometer_data));
	for (int i = 0; i < sizeof(fd->charging_data); i++)
	{
		vPortFree(&(fd->charging_data[i]));
	}
	vPortFree(&(fd->charging_data));
	for (int i = 0; i < sizeof(fd->radio_temp_data); i++)
	{
		vPortFree(&(fd->radio_temp_data[i]));
	}
	vPortFree(&(fd->radio_temp_data));
	for (int i = 0; i < sizeof(fd->battery_voltages_data); i++)
	{
		vPortFree(&(fd->battery_voltages_data[i]));
	}
	vPortFree(&(fd->battery_voltages_data));
	for (int i = 0; i < sizeof(fd->regulator_voltages_data); i++)
	{
		vPortFree(&(fd->regulator_voltages_data[i]));
	}
	vPortFree(&(fd->regulator_voltages_data));
	vPortFree(&fd);
}