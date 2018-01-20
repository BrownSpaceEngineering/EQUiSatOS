/*
 * os_tests.c
 *
 * Created: 11/19/2017 22:43:11
 *  Author: mcken
 */ 

#include "os_system_tests.h"

#define TASK_EXECUTION_WINDOW_BUFFER_TIME		1000 // how long to add to a perfectly sized window for a task to run in (buffer)
#define TICKS_IN_EACH_VALID_STATE				11000
#define IN_STATE_TIME 500 // ms

void test_all_state_transitions(void) 
{
	// Go through every possible state change, and check that 
	// set_sat_state returns the correct validity of the state 
	// change. 
	// Because this goes through every satellite state, 
	// it's also a good way to check the whole satellite operation!
	for (int old_state = 0; old_state < NUM_SAT_STATES; old_state++) 
	{
		for (int new_state = 0; new_state < NUM_SAT_STATES; new_state++) 
		{
			// make sure we start at the old state
			force_set_state(old_state);
			
			// try to transition, checking if responded correctly
			bool valid_state = check_set_sat_state(old_state, new_state);
			
			if (valid_state)
			{
				// run for a bit after a valid state transition
				delay_ms(TICKS_IN_EACH_VALID_STATE);
			}
		}
	}
}

void test_normal_satellite_state_sequence(void)
{
	static state_num = 0; // set to some number to skip to a state
	
	TickType_t ms = xTaskGetTickCount();
	
	if (state_num < 1) {
		delay_ms(IN_STATE_TIME);
// 		delay_ms(ATTITUDE_DATA_TASK_FREQ + // lowest frequency task in state // TODO: BATTERY_CHARGING_TASK
// 			TASK_EXECUTION_WINDOW_BUFFER_TIME);
		check_set_sat_state(INITIAL, ANTENNA_DEPLOY);	
		state_num++;		
	}
	
	// the below may actually transition sooner to hello world depending on what the antenna deploy state is
	else if (state_num < 2) {
		delay_ms(IN_STATE_TIME);
// 		delay_ms(ATTITUDE_DATA_TASK_FREQ + // lowest frequency task in state // TODO: BATTERY_CHARGING_TASK
// 				TASK_EXECUTION_WINDOW_BUFFER_TIME);
		check_set_sat_state(ANTENNA_DEPLOY, HELLO_WORLD);
		state_num++;
	}
	
	else if (state_num < 3) {
		delay_ms(IN_STATE_TIME);
// 		delay_ms(ATTITUDE_DATA_TASK_FREQ + // lowest frequency task in state // TODO: BATTERY_CHARGING_TASK
// 				TASK_EXECUTION_WINDOW_BUFFER_TIME);
		check_set_sat_state(HELLO_WORLD, IDLE_NO_FLASH);
		state_num++;
	}
				
	else if (state_num < 4) {
		delay_ms(IN_STATE_TIME);
// 		delay_ms(ATTITUDE_DATA_TASK_FREQ + // lowest frequency task in state // TODO: BATTERY_CHARGING_TASK
// 				TASK_EXECUTION_WINDOW_BUFFER_TIME);
		check_set_sat_state(IDLE_NO_FLASH, IDLE_FLASH);	
		state_num++;			
	}
	
	else if (state_num < 5) {
		delay_ms(IN_STATE_TIME);
// 		delay_ms(ATTITUDE_DATA_TASK_FREQ + // lowest frequency task in state // TODO: BATTERY_CHARGING_TASK
// 				TASK_EXECUTION_WINDOW_BUFFER_TIME);
		check_set_sat_state(IDLE_FLASH, LOW_POWER);		
		state_num++;		
	}
	
	else if (state_num < 6) {
		delay_ms(IN_STATE_TIME);
// 		delay_ms(2 * LOW_POWER_DATA_TASK_FREQ + // lowest frequency task in state // TODO: BATTERY_CHARGING_TASK
// 				TASK_EXECUTION_WINDOW_BUFFER_TIME);
		// throw RIP in at the very end
		check_set_sat_state(LOW_POWER, RIP);		
		state_num++;		
	}
}

/************************************************************************/
/* WATCHDOG TESTS                                                       */
/************************************************************************/

// use normal execution tests to test whether watchdog gives false alarms (kicks)

// these test generally will enter a state where the given task to test
// runs, wait for one or two executions of that task, and then suspend
// it to get a watchdog reset (hopefully)

void test_watchdog_reset_bat_charging(void) {
	TickType_t ms = xTaskGetTickCount();
	
	check_set_sat_state(INITIAL, INITIAL);
	if (ms > 2 * ATTITUDE_DATA_TASK_FREQ + TASK_EXECUTION_WINDOW_BUFFER_TIME) {
		task_suspend(BATTERY_CHARGING_TASK);
	}
	
	// watchdog should reset eventually
}

void test_watchdog_reset_attitude_data(void) {
	TickType_t ms = xTaskGetTickCount();
	
	check_set_sat_state(INITIAL, INITIAL);
	check_set_sat_state(INITIAL, ANTENNA_DEPLOY);
	if (ms > 2 * ATTITUDE_DATA_TASK_FREQ + TASK_EXECUTION_WINDOW_BUFFER_TIME) {
		task_suspend(ATTITUDE_DATA_TASK);
	}
	
	// watchdog should reset eventually
}

void test_watchdog_reset_antenna_deploy(void) {
	TickType_t ms = xTaskGetTickCount();
	
	check_set_sat_state(INITIAL, INITIAL);
	check_set_sat_state(INITIAL, ANTENNA_DEPLOY);
	// only once because otherwise it may check itself out
	if (ms > ANTENNA_DEPLOY_TASK_FREQ + TASK_EXECUTION_WINDOW_BUFFER_TIME) {
		task_suspend(ANTENNA_DEPLOY_TASK);
	}
	
	// watchdog should reset eventually
}

void test_watchdog_reset_transmit_task(void) {
	TickType_t ms = xTaskGetTickCount();
	
	check_set_sat_state(INITIAL, INITIAL);
	check_set_sat_state(INITIAL, ANTENNA_DEPLOY);
	check_set_sat_state(ANTENNA_DEPLOY, HELLO_WORLD);
	if (ms > 2 * TRANSMIT_TASK_FREQ + TASK_EXECUTION_WINDOW_BUFFER_TIME) {
		task_suspend(TRANSMIT_TASK);
	}
	
	
	// watchdog should reset eventually
}

void test_watchdog_reset_idle_data_task(void) {
	TickType_t ms = xTaskGetTickCount();
	
	check_set_sat_state(INITIAL, INITIAL);
	check_set_sat_state(INITIAL, ANTENNA_DEPLOY);
	check_set_sat_state(ANTENNA_DEPLOY, HELLO_WORLD);
	check_set_sat_state(HELLO_WORLD, IDLE_NO_FLASH);
	if (ms > 2 * IDLE_DATA_TASK_FREQ + TASK_EXECUTION_WINDOW_BUFFER_TIME){
		task_suspend(IDLE_DATA_TASK);
	}
	
	// watchdog should reset eventually
}

void test_watchdog_reset_flash_activate_task(void) {
	TickType_t ms = xTaskGetTickCount();
	
	check_set_sat_state(INITIAL, INITIAL);
	check_set_sat_state(INITIAL, ANTENNA_DEPLOY);
	check_set_sat_state(ANTENNA_DEPLOY, HELLO_WORLD);
	check_set_sat_state(HELLO_WORLD, IDLE_NO_FLASH);
	check_set_sat_state(IDLE_NO_FLASH, IDLE_FLASH);
	if (ms > 2 * FLASH_ACTIVATE_TASK_FREQ + TASK_EXECUTION_WINDOW_BUFFER_TIME) {
		task_suspend(FLASH_ACTIVATE_TASK);
	}
	
	// watchdog should reset eventually
}

void test_watchdog_reset_low_power_data_task(void) {
	TickType_t ms = xTaskGetTickCount();
	
	check_set_sat_state(INITIAL, INITIAL);
	check_set_sat_state(INITIAL, ANTENNA_DEPLOY);
	check_set_sat_state(ANTENNA_DEPLOY, HELLO_WORLD);
	check_set_sat_state(HELLO_WORLD, IDLE_NO_FLASH);
	check_set_sat_state(IDLE_NO_FLASH, IDLE_FLASH);
	check_set_sat_state(IDLE_FLASH, LOW_POWER);
	if (ms > 2 * LOW_POWER_DATA_TASK_FREQ + TASK_EXECUTION_WINDOW_BUFFER_TIME) {
		task_suspend(LOW_POWER_DATA_TASK);
	}
	
	// watchdog should reset eventually
}

/************************************************************************/
/* HELPERS                                                              */
/************************************************************************/

/* transitions to the new_state, and given the old_state, checks whether
   the return value of set_sat_state was in accordance with what was required.
   Returns whether the change was valid if it was in accordance with expected, else hangs. */
bool check_set_sat_state(sat_state_t old_state, sat_state_t new_state)
{
	bool valid = set_sat_state(new_state);
	
	if (old_state == new_state) {
		configASSERT(valid);
		return true;
	}
	
	switch (old_state) 
	{
		case INITIAL:
			configASSERT(!valid || valid && (new_state == ANTENNA_DEPLOY || new_state == RIP));
			return valid;
		case ANTENNA_DEPLOY:
			configASSERT(!valid || valid && (new_state == HELLO_WORLD || new_state == RIP));
			return valid;
		case HELLO_WORLD:
			configASSERT(!valid || valid && (new_state == IDLE_NO_FLASH || new_state == RIP));
			return valid;
		case IDLE_NO_FLASH:
			configASSERT(!valid || valid && (new_state == IDLE_FLASH || new_state == LOW_POWER || new_state == RIP));
			return valid;
		case IDLE_FLASH:
			configASSERT(!valid || valid && (new_state == IDLE_NO_FLASH || new_state == LOW_POWER || new_state == RIP));
			return valid;
		case LOW_POWER:
			configASSERT(!valid || valid && (new_state == IDLE_NO_FLASH || new_state == RIP));
			return valid;
		case RIP: 
			configASSERT(!valid); // can't transition from here
			return valid;
		default:
			configASSERT(!valid);
			return valid;
	}
}

void force_set_state(sat_state_t new_state) 
{
	switch (new_state) {
		case INITIAL:
			set_all_task_states(INITIAL_TASK_STATES, INITIAL);
			break;
		case ANTENNA_DEPLOY:
			set_all_task_states(ANTENNA_DEPLOY_TASK_STATES, ANTENNA_DEPLOY);
			break;
		case HELLO_WORLD:
			set_all_task_states(HELLO_WORLD_TASK_STATES, HELLO_WORLD);
			break;
		case IDLE_NO_FLASH:
			set_all_task_states(IDLE_NO_FLASH_TASK_STATES, IDLE_NO_FLASH);
			break;
		case IDLE_FLASH:
			set_all_task_states(IDLE_FLASH_TASK_STATES, IDLE_FLASH);
			break;
		case LOW_POWER:
			set_all_task_states(LOW_POWER_TASK_STATES, LOW_POWER);
			break;
		case RIP:
			set_all_task_states(RIP_TASK_STATES, RIP);
			break;
		default:
			configASSERT(false);
	}
}