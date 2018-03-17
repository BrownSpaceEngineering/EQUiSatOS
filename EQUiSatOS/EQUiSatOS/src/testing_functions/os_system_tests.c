/*
 * os_tests.c
 *
 * Created: 11/19/2017 22:43:11
 *  Author: mcken
 */

#include "os_system_tests.h"

#define TASK_EXECUTION_WINDOW_BUFFER_TIME		1000 // how long to add to a perfectly sized window for a task to run in (buffer)
#define TICKS_IN_EACH_VALID_STATE				11000

// determines how long we spend in each state
#define LOWEST_TASK_FREQ			240000 //BATTERY_CHARGING_TASK_FREQ
#define EQUISIM_IN_STATE_TIME_MS			(LOWEST_TASK_FREQ + TASK_EXECUTION_WINDOW_BUFFER_TIME)

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

void test_normal_satellite_state_sequence(void) {
	int state_num = 0; // set to some number to skip to a state

	if (state_num == 0) {
		// start out in initial
		set_sat_state_helper(INITIAL);
		check_out_task_unsafe(STATE_HANDLING_TASK); // to avoid watchdog reset when warranted
		vTaskDelay(EQUISIM_IN_STATE_TIME_MS / portTICK_PERIOD_MS);
	}

	if (state_num < 1) {
		check_set_sat_state(INITIAL, ANTENNA_DEPLOY);
		check_out_task_unsafe(STATE_HANDLING_TASK); // to avoid watchdog reset when warranted
		vTaskDelay(EQUISIM_IN_STATE_TIME_MS / portTICK_PERIOD_MS);
		state_num++;
	}

	// the below may actually transition sooner to hello world depending on what the antenna deploy state is
	if (state_num < 2) {
		check_set_sat_state(ANTENNA_DEPLOY, HELLO_WORLD);
		check_out_task_unsafe(STATE_HANDLING_TASK); // to avoid watchdog reset when warranted
		vTaskDelay(EQUISIM_IN_STATE_TIME_MS / portTICK_PERIOD_MS);
		state_num++;
	}

	if (state_num < 3) {
		check_set_sat_state(HELLO_WORLD, IDLE_NO_FLASH);
		check_out_task_unsafe(STATE_HANDLING_TASK); // to avoid watchdog reset when warranted
		vTaskDelay(EQUISIM_IN_STATE_TIME_MS / portTICK_PERIOD_MS);
		state_num++;
	}

	if (state_num < 4) {
		check_set_sat_state(IDLE_NO_FLASH, IDLE_FLASH);
		check_out_task_unsafe(STATE_HANDLING_TASK); // to avoid watchdog reset when warranted
		vTaskDelay(EQUISIM_IN_STATE_TIME_MS / portTICK_PERIOD_MS);
		state_num++;
	}

	if (state_num < 5) {
		check_set_sat_state(IDLE_FLASH, LOW_POWER);
		check_out_task_unsafe(STATE_HANDLING_TASK); // to avoid watchdog reset when warranted
		vTaskDelay(EQUISIM_IN_STATE_TIME_MS / portTICK_PERIOD_MS);
		state_num++;
	}

	if (state_num < 6) {
		check_set_sat_state(LOW_POWER, IDLE_NO_FLASH);
		check_out_task_unsafe(STATE_HANDLING_TASK); // to avoid watchdog reset when warranted
		vTaskDelay(EQUISIM_IN_STATE_TIME_MS / portTICK_PERIOD_MS);
		state_num++;
	}
	vTaskDelay(LOWEST_TASK_FREQ * 10);
}

void test_error_case_satellite_state_sequence(void)
{
	int state_num = 0; // set to some number to skip to a state

	if (state_num == 0) {
		// start out in initial
		set_sat_state_helper(INITIAL);
		vTaskDelay(EQUISIM_IN_STATE_TIME_MS / portTICK_PERIOD_MS);
	}
	report_task_running(STATE_HANDLING_TASK);

	if (state_num < 1) {
		check_set_sat_state(INITIAL, ANTENNA_DEPLOY);
		vTaskDelay(EQUISIM_IN_STATE_TIME_MS / portTICK_PERIOD_MS);
		state_num++;
	}
	report_task_running(STATE_HANDLING_TASK);

	// go to emergency low power (the ANTENNA_DEPLOY state is very different from IDLE_NO_FLASH)
	if (state_num < 2) {
		check_set_sat_state(ANTENNA_DEPLOY, LOW_POWER);
		vTaskDelay(EQUISIM_IN_STATE_TIME_MS / portTICK_PERIOD_MS);
		check_set_sat_state(LOW_POWER, ANTENNA_DEPLOY);
		vTaskDelay(EQUISIM_IN_STATE_TIME_MS / portTICK_PERIOD_MS);
		state_num++;
	}
	report_task_running(STATE_HANDLING_TASK);

	// the below may actually transition sooner to hello world depending on what the antenna deploy state is
	if (state_num < 3) {
		check_set_sat_state(ANTENNA_DEPLOY, HELLO_WORLD);
		vTaskDelay(EQUISIM_IN_STATE_TIME_MS / portTICK_PERIOD_MS);
		state_num++;
	}
	report_task_running(STATE_HANDLING_TASK);

	// go to emergency low power
	if (state_num < 4) {
		check_set_sat_state(HELLO_WORLD, LOW_POWER);
		vTaskDelay(EQUISIM_IN_STATE_TIME_MS / portTICK_PERIOD_MS);
		check_set_sat_state(LOW_POWER, HELLO_WORLD);
		vTaskDelay(EQUISIM_IN_STATE_TIME_MS / portTICK_PERIOD_MS);
		state_num++;
	}
	report_task_running(STATE_HANDLING_TASK);

	if (state_num < 5) {
		check_set_sat_state(HELLO_WORLD, IDLE_NO_FLASH);
		vTaskDelay(EQUISIM_IN_STATE_TIME_MS / portTICK_PERIOD_MS);
		state_num++;
	}
	report_task_running(STATE_HANDLING_TASK);

	// go to emergency low power, then back again
	if (state_num < 6) {
		check_set_sat_state(IDLE_NO_FLASH, LOW_POWER);
		vTaskDelay(EQUISIM_IN_STATE_TIME_MS / portTICK_PERIOD_MS);
		check_set_sat_state(LOW_POWER, IDLE_NO_FLASH);
		vTaskDelay(EQUISIM_IN_STATE_TIME_MS / portTICK_PERIOD_MS);
		state_num++;
	}
	report_task_running(STATE_HANDLING_TASK);

	if (state_num < 7) {
		check_set_sat_state(IDLE_NO_FLASH, IDLE_FLASH);
		vTaskDelay(EQUISIM_IN_STATE_TIME_MS / portTICK_PERIOD_MS);
		state_num++;
	}
	report_task_running(STATE_HANDLING_TASK);

	if (state_num < 8) {
		check_set_sat_state(IDLE_FLASH, LOW_POWER);
		vTaskDelay(EQUISIM_IN_STATE_TIME_MS / portTICK_PERIOD_MS);
		state_num++;
	}
	report_task_running(STATE_HANDLING_TASK);

	if (state_num < 9) {
		check_set_sat_state(LOW_POWER, IDLE_FLASH);
		vTaskDelay(EQUISIM_IN_STATE_TIME_MS / portTICK_PERIOD_MS);
		state_num++;
	}
	report_task_running(STATE_HANDLING_TASK);

	if (state_num < 10) {
		check_set_sat_state(HELLO_WORLD, IDLE_NO_FLASH);
		vTaskDelay(EQUISIM_IN_STATE_TIME_MS / portTICK_PERIOD_MS);
		state_num++;
	}
	report_task_running(STATE_HANDLING_TASK);
	vTaskDelay(LOWEST_TASK_FREQ * 10);
	report_task_running(STATE_HANDLING_TASK);
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
			configASSERT(!valid || valid && (new_state == ANTENNA_DEPLOY ));
			return valid;
		case ANTENNA_DEPLOY:
			configASSERT(!valid || valid && (new_state == HELLO_WORLD || new_state == LOW_POWER));
			return valid;
		case HELLO_WORLD:
			configASSERT(!valid || valid && (new_state == IDLE_NO_FLASH || new_state == LOW_POWER));
			return valid;
		case IDLE_NO_FLASH:
			configASSERT(!valid || valid && (new_state == IDLE_FLASH || new_state == LOW_POWER));
			return valid;
		case IDLE_FLASH:
			configASSERT(!valid || valid && (new_state == IDLE_NO_FLASH || new_state == LOW_POWER));
			return valid;
		case LOW_POWER:
			configASSERT(!valid || valid && (new_state == IDLE_NO_FLASH || new_state == ANTENNA_DEPLOY));
			return valid;
		default:
			configASSERT(!valid);
			return valid;
	}
}

void force_set_state(sat_state_t new_state)
{
	// note: probably a transition from new_state -> new_state in here
	switch (new_state) {
		case INITIAL:
			set_all_task_states(INITIAL_TASK_STATES, INITIAL, new_state);
			break;
		case ANTENNA_DEPLOY:
			set_all_task_states(ANTENNA_DEPLOY_TASK_STATES, ANTENNA_DEPLOY, new_state);
			break;
		case HELLO_WORLD:
			set_all_task_states(HELLO_WORLD_TASK_STATES, HELLO_WORLD, new_state);
			break;
		case IDLE_NO_FLASH:
			set_all_task_states(IDLE_NO_FLASH_TASK_STATES, IDLE_NO_FLASH, new_state);
			break;
		case IDLE_FLASH:
			set_all_task_states(IDLE_FLASH_TASK_STATES, IDLE_FLASH, new_state);
			break;
		case LOW_POWER:
			set_all_task_states(LOW_POWER_TASK_STATES, LOW_POWER, new_state);
			break;
		default:
			configASSERT(false);
	}
}
