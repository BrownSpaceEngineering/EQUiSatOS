#include "rtos_tasks.h"
#include "battery_charging_task.h"
#include "../runnable_configurations/antenna_pwm.h"
#include "../telemetry/Radio_Commands.h"

#define TIME_TO_WAIT_FOR_CRIT_MS	2000

void emergency_resume_antenna_deploy(void);
void decide_next_state(sat_state_t current_state);
void check_for_error_issues(void);

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
		// check if there are any patterns of errors that warrent action
		check_for_error_issues();
		
		// check for state change
		decide_next_state(get_sat_state());
	}

	// delete this task if it ever breaks out
	vTaskDelete(NULL);
}

// Decides next state based on the criteria that dictate
// state changes. Delegates the actions required on a state
// change to the logic in satellite_state_control.c
void decide_next_state(sat_state_t current_state) {
	
	///
	// the state decision will be predicated on the current battery levels and
	// the timestamp -- we'll grab them here
	///

	uint16_t li1_mv;
	uint16_t li2_mv;
	read_lion_volts_precise(&li1_mv, &li2_mv, true);

	// individual batteries within the life po banks
	uint16_t lf1_mv;
	uint16_t lf2_mv;
	uint16_t lf3_mv;
	uint16_t lf4_mv;
	read_lifepo_volts_precise(&lf1_mv, &lf2_mv, &lf3_mv, &lf4_mv, true);

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

	bat_charge_dig_sigs_batch batch;
	bool got_batch = !read_bat_charge_dig_sigs_batch_with_retry(&batch);

	bool one_lf_full_one_above_flash = (get_lf_full(lfb1_sum, max(lf1_mv, lf2_mv), LFB1, batch, got_batch) && (lfb2_sum >= LF_FLASH_MIN_MV || charging_data.decommissioned[LFB2])) 
									|| (get_lf_full(lfb2_sum, max(lf3_mv, lf4_mv), LFB2, batch, got_batch) && (lfb1_sum >= LF_FLASH_MIN_MV || charging_data.decommissioned[LFB1]));
	bool one_lf_below_flash = (lfb1_sum < LF_FLASH_MIN_MV) || (lfb2_sum < LF_FLASH_MIN_MV);

	bool one_li_below_low_power = li1_mv <= LI_LOW_POWER_MV || li2_mv <= LI_LOW_POWER_MV;
	bool low_power_entry_criteria = charging_data.curr_meta_charge_state == TWO_LI_DOWN
									|| (charging_data.curr_meta_charge_state == ALL_GOOD &&
										one_li_below_low_power)
									|| (charging_data.curr_meta_charge_state == ONE_LI_DOWN &&
										(charging_data.decommissioned[LI1] ? true : (li1_mv > LI_LOW_POWER_MV))
										&& (charging_data.decommissioned[LI2] ? true : (li2_mv > LI_LOW_POWER_MV)));
	bool low_power_exit_criteria = !low_power_entry_criteria;

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
			} else if (should_exit_antenna_deploy()) {
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

		case IDLE_NO_FLASH:
			// it's higher priority to go to low power
			if (low_power_entry_criteria)
				set_sat_state(LOW_POWER);
			else if (one_lf_full_one_above_flash && !flash_killed)
				set_sat_state(IDLE_FLASH);
			break;

		case IDLE_FLASH:
			// it's higher priority to go to low power
			if (low_power_entry_criteria)
				set_sat_state(LOW_POWER);
			else if (one_lf_below_flash || flash_killed)
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


/* responds to certain errors that may require action */
void check_for_error_issues(void) {
	uint num_i2c_errors = 0;
	uint32_t cur_timestamp = get_current_timestamp();
	uint32_t oldest_worrisome_timestamp = cached_state._secs_since_launch_at_boot;
	if (cur_timestamp > Max(I2C_ERROR_CONSIDERATION_PERIOD_S, cached_state._secs_since_launch_at_boot)) {
		oldest_worrisome_timestamp = cur_timestamp - I2C_ERROR_CONSIDERATION_PERIOD_S;
	}
	
	// take error equistack mutex to have a consistent state; if we can't get it it's not crucial though
	bool got_mutex = xSemaphoreTake(error_equistack.mutex, (TickType_t) EQUISTACK_MUTEX_WAIT_TIME_TICKS);
	if (!got_mutex) {
		log_error(ELOC_STATE_HANDLING, ECODE_EQUISTACK_MUTEX_TIMEOUT, false);
	}
	{
		for (uint8_t i = 0; i < error_equistack.cur_size; i++) {
			sat_error_t* err = (sat_error_t*) equistack_Get_Unsafe(&error_equistack, i);
			if (err != NULL) {
				sat_ecode code = get_ecode(err);
				// observe properties of errors
				if (eloc_category_i2c(err->eloc)
					&& (code == ECODE_BAD_ADDRESS || code == ECODE_TIMEOUT || code == ECODE_OVERFLOW)
					&& err->timestamp >= oldest_worrisome_timestamp) {
					num_i2c_errors++;
				}
			}
		}
	}
	if (got_mutex) xSemaphoreGive(error_equistack.mutex);
	
	/*
		if i2c-related devices are all failing, there may be an issue with the i2c bus, so 
		if we get enough of them, reset the satellite in the hopes of fixing it.
	*/
	if (num_i2c_errors > I2C_ERROR_MAX_NUM_IN_CONSID_PERIOD) {
		log_error(ELOC_STATE_HANDLING, ECODE_I2C_BUS_ERROR, true);
		configASSERT(false); // this is bad
		write_state_to_storage_emergency(false);
		system_reset();
	}
}