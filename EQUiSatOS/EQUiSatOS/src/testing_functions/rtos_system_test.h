/*
 * rtos_system_test.h
 *
 * Created: 2/11/2018 18:29:52
 *  Author: mcken
 */ 


#ifndef RTOS_SYSTEM_TEST_H_
#define RTOS_SYSTEM_TEST_H_

#include <global.h>
#include "data_handling/State_Structs.h"
#include "data_handling/Sensor_Structs.h"
#include "errors.h"

void print_accel_batch(accelerometer_batch batch);
void print_gyro_batch(gyro_batch batch);
void print_magnetometer_batch(magnetometer_batch batch);
void print_ir_ambient_temps_batch(ir_ambient_temps_batch batch);
void print_ir_object_temps_batch(ir_object_temps_batch batch);
void print_pdiode_batch(pdiode_batch batch);
void print_lion_volts_batch(lion_volts_batch batch);
void print_led_current_batch(led_current_batch batch);
void print_lion_temps_batch(lion_temps_batch batch);
void print_led_temps_batch(led_temps_batch batch);
void print_lifepo_temps_batch(lifepo_bank_temps_batch batch);
void print_lifepo_volts_batch(lifepo_volts_batch batch);
void print_lifepo_current_batch(lifepo_current_batch batch);
void print_satellite_state_history_batch(satellite_history_batch batch);
void print_panelref_lref_batch(panelref_lref_batch batch);
void print_bat_charge_dig_sigs_batch(bat_charge_dig_sigs_batch batch);
void print_radio_temp_batch(radio_temp_batch batch);
void print_imu_temp_batch(imu_temp_batch batch);

void print_idle_data(idle_data_t* data, int i);
void print_attitude_data(attitude_data_t* data, int i);
void print_flash_data(flash_data_t* data, int i);
void print_flash_cmp_data(flash_cmp_data_t* data, int i);
void print_low_power_data(low_power_data_t* data, int i);

const char* get_sat_state_str(sat_state_t state);
const char* get_task_str(task_type_t task);
const char* get_task_state_str(eTaskState state);
const char* get_msg_type_str(msg_data_type_t msg_type);

void print_task_info(void);
void print_errors(int max_num);
void print_equistack(equistack* stack, void (*elm_print)(void*, int), const char* header, int max_num);
void print_cur_data_buf(uint8_t* cur_data_buf);
void print_task_stack_usage(task_type_t task, uint32_t stack_size);
void print_task_stack_usages(void);
void rtos_system_test(void);

#endif /* RTOS_SYSTEM_TEST_H_ */