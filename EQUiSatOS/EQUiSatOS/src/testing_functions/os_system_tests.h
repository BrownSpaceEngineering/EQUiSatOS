/*
 * os_tests.h
 *
 * Created: 11/19/2017 22:42:01
 *  Author: mcken
 */ 

#include "global.h"
#include "../rtos_tasks/rtos_tasks_config.h"
#include "../runnable_configurations/satellite_state_control.h"

/**
 * These generally must be run and checked from the idle task hook
 */
void test_normal_satellite_state_sequence(void);
void test_error_case_satellite_state_sequence(void);
void test_all_state_transitions(void);
// watchdog test
void test_watchdog_reset_bat_charging(void);
void test_watchdog_reset_attitude_data(void);
void test_watchdog_reset_antenna_deploy(void);
void test_watchdog_reset_transmit_task(void);
void test_watchdog_reset_idle_data_task(void);
void test_watchdog_reset_flash_activate_task(void);
void test_watchdog_reset_low_power_data_task(void);

// helpers
bool check_set_sat_state(sat_state_t old_state, sat_state_t new_state);
void force_set_state(sat_state_t new_state);