/*
 * os_tests.c
 *
 * Created: 11/19/2017 22:43:11
 *  Author: mcken
 */ 

#include "os_system_tests.h"

#define TASK_EXECUTION_WINDOW_BUFFER_TIME		1000 // how long to add to a perfectly sized window for a task to run in (buffer)
#define TICKS_IN_EACH_VALID_STATE				11000

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
				vTaskDelay(TICKS_IN_EACH_VALID_STATE);
			}
		}
	}
}

void test_normal_satellite_state_sequence(void)
{
	check_set_sat_state(INITIAL, INITIAL);
	vTaskDelay(2 * ATTITUDE_DATA_TASK_FREQ + // lowest frequency task in state // TODO: BATTERY_CHARGING_TASK
				TASK_EXECUTION_WINDOW_BUFFER_TIME);
				
	check_set_sat_state(INITIAL, ANTENNA_DEPLOY);
	vTaskDelay(2 * ATTITUDE_DATA_TASK_FREQ + // lowest frequency task in state // TODO: BATTERY_CHARGING_TASK
				TASK_EXECUTION_WINDOW_BUFFER_TIME);
	// the above may actually transition sooner to hello world depending on what the antenna deploy state is
	
	check_set_sat_state(ANTENNA_DEPLOY, HELLO_WORLD);
	vTaskDelay(2 * ATTITUDE_DATA_TASK_FREQ + // lowest frequency task in state // TODO: BATTERY_CHARGING_TASK
				TASK_EXECUTION_WINDOW_BUFFER_TIME);
				
	check_set_sat_state(HELLO_WORLD, IDLE_NO_FLASH);
	vTaskDelay(2 * ATTITUDE_DATA_TASK_FREQ + // lowest frequency task in state // TODO: BATTERY_CHARGING_TASK
				TASK_EXECUTION_WINDOW_BUFFER_TIME);
 
	check_set_sat_state(IDLE_NO_FLASH, IDLE_FLASH);
	vTaskDelay(2 * ATTITUDE_DATA_TASK_FREQ + // lowest frequency task in state // TODO: BATTERY_CHARGING_TASK
				TASK_EXECUTION_WINDOW_BUFFER_TIME);
		
	check_set_sat_state(IDLE_FLASH, LOW_POWER);
	vTaskDelay(2 * LOW_POWER_DATA_TASK_FREQ + // lowest frequency task in state // TODO: BATTERY_CHARGING_TASK
				TASK_EXECUTION_WINDOW_BUFFER_TIME);
	
	check_set_sat_state(LOW_POWER, RIP); // throw it in at the very end
	vTaskDelay(2 * ATTITUDE_DATA_TASK_FREQ + // lowest frequency task in state // TODO: BATTERY_CHARGING_TASK
				TASK_EXECUTION_WINDOW_BUFFER_TIME);
}

/************************************************************************/
/* WATCHDOG TESTS                                                       */
/************************************************************************/

// use normal execution tests to test whether watchdog gives false alarms (kicks)

// these test generally will enter a state where the given task to test
// runs, wait for one or two executions of that task, and then suspend
// it to get a watchdog reset (hopefully)

void test_watchdog_reset_bat_charging(void) {
	check_set_sat_state(INITIAL, INITIAL);
	vTaskDelay(2 * ATTITUDE_DATA_TASK_FREQ + TASK_EXECUTION_WINDOW_BUFFER_TIME);
		
	task_suspend(BATTERY_CHARGING_TASK);
	
	// watchdog should reset eventually
}

void test_watchdog_reset_attitude_data(void) {
	check_set_sat_state(INITIAL, INITIAL);
	check_set_sat_state(INITIAL, ATTITUDE_DATA);
	vTaskDelay(2 * ATTITUDE_DATA_TASK_FREQ + TASK_EXECUTION_WINDOW_BUFFER_TIME);
	
	task_suspend(ATTITUDE_DATA_TASK);
	
	// watchdog should reset eventually
}

void test_watchdog_reset_antenna_deploy(void) {
	check_set_sat_state(INITIAL, INITIAL);
	check_set_sat_state(INITIAL, ATTITUDE_DATA);
	// only once because otherwise it may check itself out
	vTaskDelay(ANTENNA_DEPLOY_TASK_FREQ + TASK_EXECUTION_WINDOW_BUFFER_TIME);
	
	task_suspend(ANTENNA_DEPLOY_TASK);
	
	// watchdog should reset eventually
}

void test_watchdog_reset_transmit_task(void) {
	check_set_sat_state(INITIAL, INITIAL);
	check_set_sat_state(INITIAL, ATTITUDE_DATA);
	check_set_sat_state(ATTITUDE_DATA, HELLO_WORLD);
	vTaskDelay(2 * TRANSMIT_TASK_FREQ + TASK_EXECUTION_WINDOW_BUFFER_TIME);
	
	task_suspend(TRANSMIT_TASK);
	
	// watchdog should reset eventually
}

void test_watchdog_reset_idle_data_task(void) {
	check_set_sat_state(INITIAL, INITIAL);
	check_set_sat_state(INITIAL, ATTITUDE_DATA);
	check_set_sat_state(ATTITUDE_DATA, HELLO_WORLD);
	check_set_sat_state(HELLO_WORLD, IDLE_NO_FLASH);
	vTaskDelay(2 * IDLE_DATA_TASK_FREQ + TASK_EXECUTION_WINDOW_BUFFER_TIME);
	
	task_suspend(IDLE_DATA_TASK);
	
	// watchdog should reset eventually
}

void test_watchdog_reset_flash_activate_task(void) {
	check_set_sat_state(INITIAL, INITIAL);
	check_set_sat_state(INITIAL, ATTITUDE_DATA);
	check_set_sat_state(ATTITUDE_DATA, HELLO_WORLD);
	check_set_sat_state(HELLO_WORLD, IDLE_NO_FLASH);
	check_set_sat_state(IDLE_NO_FLASH, IDLE_FLASH);
	vTaskDelay(2 * FLASH_ACTIVATE_TASK_FREQ + TASK_EXECUTION_WINDOW_BUFFER_TIME);
	
	task_suspend(FLASH_ACTIVATE_TASK);
	
	// watchdog should reset eventually
}

void test_watchdog_reset_low_power_data_task(void) {
	check_set_sat_state(INITIAL, INITIAL);
	check_set_sat_state(INITIAL, ATTITUDE_DATA);
	check_set_sat_state(ATTITUDE_DATA, HELLO_WORLD);
	check_set_sat_state(HELLO_WORLD, IDLE_NO_FLASH);
	check_set_sat_state(IDLE_NO_FLASH, IDLE_FLASH);
	check_set_sat_state(IDLE_FLASH, LOW_POWER);
	vTaskDelay(2 * LOW_POWER_DATA_TASK_FREQ + TASK_EXECUTION_WINDOW_BUFFER_TIME);
	
	task_suspend(LOW_POWER_DATA_TASK);
	
	// watchdog should reset eventually
}

/************************************************************************/
/* HELPERS                                                              */
/************************************************************************/

/* transitions to the new_state, and given the old_state, checks whether
   the return value of set_sat_state was in accordance with what was required.
   Returns whether the change was valid if it was in accordance with expected, else hangs. */
bool check_set_sat_state(global_state_t old_state, global_state_t new_state)
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

void force_set_state(global_state_t new_state) 
{
	switch (new_state) {
		case INITIAL:
			set_state_initial();
			break;
		case ANTENNA_DEPLOY:
			set_state_antenna_deploy();
			break;
		case HELLO_WORLD:
			set_state_hello_world();
			break;
		case IDLE_NO_FLASH:
			set_state_idle_no_flash();
			break;
		case IDLE_FLASH:
			set_state_idle_flash();
			break;
		case LOW_POWER:
			set_state_low_power();
			break;
		case RIP:
			set_state_rip();
			break;
		default:
			configASSERT(false);
	}
}