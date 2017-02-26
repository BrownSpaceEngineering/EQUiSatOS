/*
 * testing_helper.c
 *
 * Created: 12/13/2016 9:11:37 PM
 *  Author: rj16
 */ 

#include "testing_helper.h"

idle_data_t* create_dummy_batch()
{
	idle_data_t* result;
	result = pvPortMalloc(sizeof(idle_data_t));
	
	int* ir_data = result->ir_data;
	int* temp_data = result->temp_data;
	int* diode_data = result->diode_data;
	int* led_current_data = result->led_current_data;
	int* imu_data = result->imu_data;
	int* magnetometer_data = result->magnetometer_data;
	int* charging_data = result->charging_data;
	int* radio_temp_data = result->radio_temp_data;
	int* battery_voltages_data = result->battery_voltages_data;
	int* regulator_voltages_data = result->regulator_voltages_data;
	
	create_dummy_arr(ir_data, 1);
	create_dummy_arr(temp_data, 1);
	create_dummy_arr(diode_data, 1);
	create_dummy_arr(led_current_data, 1);
	create_dummy_arr(imu_data, 1);
	create_dummy_arr(magnetometer_data, 1);
	create_dummy_arr(charging_data, 1);
	create_dummy_arr(radio_temp_data, 1);
	create_dummy_arr(battery_voltages_data, 1);
	create_dummy_arr(regulator_voltages_data, 1);
	
	return result;
}

void* create_dummy_arr(int* arr, int val)
{
	int len = sizeof(arr)/sizeof(arr[0]);
	for (int i = 0; i < len; i++)
	{
		arr[i] = val;
	}
}
