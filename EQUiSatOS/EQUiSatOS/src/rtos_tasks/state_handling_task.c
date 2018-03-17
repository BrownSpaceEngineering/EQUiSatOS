#include "rtos_tasks.h"
#include "battery_charging_task.h"
#include "../runnable_configurations/antenna_pwm.h"

#ifndef TESTING_SPEEDUP
	#define MIN_TIME_IN_INITIAL_S		(30*60)
	#define MIN_TIME_IN_BOOT_S			ORBITAL_PERIOD_S//(26*ORBITAL_PERIOD_S)
#else
	#define MIN_TIME_IN_INITIAL_S		(15*60)
	#define MIN_TIME_IN_BOOT_S			(ORBITAL_PERIOD_S / 2)
#endif
#define TIME_TO_WAIT_FOR_CRIT_MS	2000

void decide_next_state(sat_state_t current_state);

void state_handling_task(void *pvParameters)
{
	// delay to offset task relative to others, then start
	vTaskDelay(STATE_HANDLING_TASK_FREQ_OFFSET);
	TickType_t prev_wake_time = xTaskGetTickCount();

	init_task_state(STATE_HANDLING_TASK);

	for ( ;; )
	{
		#if OVERRIDE_STATE_HOLD_INIT != 1
			#ifdef EQUISIM_WATCHDOG_RESET_TEST
				test_watchdog_reset_bat_charging();
				test_watchdog_reset_attitude_data();
				test_watchdog_reset_antenna_deploy();
				test_watchdog_reset_transmit_task();
				test_watchdog_reset_idle_data_task();
				test_watchdog_reset_flash_activate_task();
				test_watchdog_reset_low_power_data_task();
			#endif

			#ifdef EQUISIM_SIMULATE_DIRECT_STATE_CHANGES
				test_normal_satellite_state_sequence();
				test_error_case_satellite_state_sequence();
			#endif
		#endif
		
		vTaskDelayUntil(&prev_wake_time, STATE_HANDLING_TASK_FREQ / portTICK_PERIOD_MS);
		
		// report to watchdog
		report_task_running(STATE_HANDLING_TASK);

		/* normal operation */
		decide_next_state(get_sat_state());
	}

	// delete this task if it ever breaks out
	vTaskDelete(NULL);
}

/* function called periodically to check if we need to resume trying to deploy the antenna */
static void emergency_resume_antenna_deploy(void) {
	// if the antenna still hasn't technically deployed, we should keep trying
	if (!antenna_did_deploy()
	&& get_sat_state() != INITIAL
	&& get_sat_state() != ANTENNA_DEPLOY
	&& get_sat_state() != LOW_POWER) {
		// if the antenna has not been deployed, start the task again to deploy it
		// (ignoring any sat states where it MUST be running or suspended)
		task_resume_safe(ANTENNA_DEPLOY_TASK);
	}
}

// Decides next state based on the criteria that dictate
// state changes. Delegates the actions required on a state
// change to the logic in satellite_state_control.c
void decide_next_state(sat_state_t current_state) {
	// handle antenna deploy task separately
	emergency_resume_antenna_deploy();
	
	// shutdown IR power if it's on when it shouldn't be
	ensure_ir_power_disabled(false);
	
	///
	// the state decision will be predicated on the current battery levels and
	// the timestamp -- we'll grab them here
	///

	uint16_t li1_mv;
	uint16_t li2_mv;
	read_lion_volts_precise(&li1_mv, &li2_mv);

	// individual batteries within the life po banks
	uint16_t lf1_mv;
	uint16_t lf2_mv;
	uint16_t lf3_mv;
	uint16_t lf4_mv;
	read_lifepo_volts_precise(&lf1_mv, &lf2_mv, &lf3_mv, &lf4_mv);

	// average voltage for the batteries within each LF bank
	int lfb1_sum = lf1_mv + lf2_mv;
	int lfb2_sum = lf3_mv + lf4_mv;

	///
	// we always will need a check whether we want to go into a low
	// power state is the data is conflicted
	///

	///
	// now it's time to check for all of the standard state changes
	///

	bool one_lf_full_one_above_flash = (get_lf_full(LFB1, max(lf1_mv, lf2_mv)) && lfb2_sum >= LF_FLASH_MIN_MV) 
									|| (get_lf_full(LFB2, max(lf3_mv, lf4_mv)) && lfb1_sum >= LF_FLASH_MIN_MV);
	bool one_lf_below_flash = (lfb1_sum < LF_FLASH_MIN_MV) || (lfb2_sum < LF_FLASH_MIN_MV);

	bool one_li_below_low_power = li1_mv <= LI_LOW_POWER_MV || li2_mv <= LI_LOW_POWER_MV;
	bool low_power_entry_criteria = charging_data.curr_meta_charge_state == TWO_LI_DOWN
									|| (charging_data.curr_meta_charge_state == ALL_GOOD &&
										one_li_below_low_power)
									|| (charging_data.curr_meta_charge_state == ONE_LI_DOWN &&
										(charging_data.decommissioned[LI1] ? true : (li1_mv > LI_LOW_POWER_MV))
										&& (charging_data.decommissioned[LI2] ? true : (li2_mv > LI_LOW_POWER_MV)));
	bool low_power_exit_criteria = !low_power_entry_criteria;

	satellite_history_batch sat_history = cache_get_sat_event_history();

	switch (current_state)
	{
		case INITIAL:
			if (get_current_timestamp() > MIN_TIME_IN_INITIAL_S
				&& charging_data.should_move_to_antenna_deploy) {
				set_sat_state(ANTENNA_DEPLOY);
			}
			break;

		case ANTENNA_DEPLOY:
			// if we should go to low power, do so
			if (low_power_entry_criteria) {
				set_sat_state(LOW_POWER);

			// otherwise if the antenna deploy task says we should move on
			// from deployment, do so
			} else if (should_exit_antenna_deploy() || get_num_tries_antenna_deploy() >= 10) {
				set_sat_state(HELLO_WORLD);
			}
			break;

		case HELLO_WORLD:
			// it's higher priority to go to low power
			if (low_power_entry_criteria)
				set_sat_state(LOW_POWER);
			else if (get_current_timestamp() > MIN_TIME_IN_BOOT_S)
				set_sat_state(IDLE_NO_FLASH);
			else if (should_exit_antenna_deploy())
				task_suspend(ANTENNA_DEPLOY_TASK);
			break;

		// TODO: new criteria here
		case IDLE_NO_FLASH:
			// it's higher priority to go to low power
			if (low_power_entry_criteria)
				set_sat_state(LOW_POWER);
			else if (one_lf_full_one_above_flash)
				set_sat_state(IDLE_FLASH);
			break;

		case IDLE_FLASH:
			// it's higher priority to go to low power
			if (low_power_entry_criteria)
				set_sat_state(LOW_POWER);
			else if (one_lf_below_flash)
				set_sat_state(IDLE_NO_FLASH);
			break;

		case LOW_POWER:
			if (low_power_exit_criteria) {
				if (get_current_timestamp() <= MIN_TIME_IN_BOOT_S)
					set_sat_state(ANTENNA_DEPLOY);
				else
					set_sat_state(IDLE_NO_FLASH);
			}
			break;

		default:
			configASSERT(false);
			log_error(ELOC_STATE_HANDLING, ECODE_UNEXPECTED_CASE, false);
			set_sat_state(IDLE_NO_FLASH); // default state
			break;
	}
}
