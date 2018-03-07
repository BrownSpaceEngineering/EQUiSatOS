#include "rtos_tasks.h"
#include "battery_charging_task.h"
#include "../runnable_configurations/antenna_pwm.h"

#define MIN_TIME_IN_INITIAL_S		(30*60)
#define MIN_TIME_IN_BOOT_S			(26*ORBITAL_PERIOD_S)
#define TIME_TO_WAIT_FOR_CRIT_MS	2000

/* controls antenna deploy task state based on whether the antenna was deployed */
void handle_antenna_deploy_task(void) {
	if (get_antenna_deployed()) { 
		// if the antenna has been deployed, suspend the task and note it occurred
		update_sat_event_history(1, 0, 0, 0, 0, 0, 0);
		set_task_state_safe(ANTENNA_DEPLOY_TASK, false); // we're the only task that can suspend a task explicitly
		
	} else if (get_sat_state() != INITIAL
				&& get_sat_state() != ANTENNA_DEPLOY
				&& get_sat_state() != LOW_POWER) {
		// alternatively, if it is not deployed, start the task again to deploy it
		// (ignoring any sat states where it MUST be running or suspended)
		set_task_state_safe(ANTENNA_DEPLOY_TASK, true);
	}
}

// returns the current state if no change required
sat_state_t check_for_end_of_life(int li1_mv, int li2_mv, sat_state_t current_state)
{
	// enter the rip state if both are lion_critical_mv
	if (li1_mv <= LI_CRITICAL_MV && li2_mv <= LI_CRITICAL_MV)
	{
		int end_of_life = 1;

		// we want to be very sure that both are actually lion_critical_mv
		for (int i = 0; i < 5; i++)
		{
			vTaskDelay(TIME_TO_WAIT_FOR_CRIT_MS);

			uint16_t li1_recalc_mv;
			uint16_t li2_recalc_mv;
			read_lion_volts_precise(&li1_recalc_mv, &li2_recalc_mv);

			if (!(li1_recalc_mv <= LI_CRITICAL_MV && li2_recalc_mv <= LI_CRITICAL_MV))
			{
				end_of_life = 0;
				break;
			}
		}

		if (end_of_life)
		{
			log_error(ELOC_STATE_HANDLING, ECODE_ENTER_RIP, true);
			return RIP;
		}
		// we got conflicted data
		else
		{
			log_error(ELOC_STATE_HANDLING, ECODE_UNCERTAIN_RIP, true);
			switch (current_state)
			{
				case INITIAL:
				case ANTENNA_DEPLOY:
					return current_state;

				case HELLO_WORLD:
				case IDLE_FLASH:
				case IDLE_NO_FLASH:
				case LOW_POWER:
					return LOW_POWER;

				// this should really never happen
				default:
					log_error(ELOC_STATE_HANDLING, ECODE_UNEXPECTED_CASE, true);
					return INITIAL;
			}
		}
	}

	return current_state;
}

void decide_next_state(sat_state_t current_state);

void state_handling_task(void *pvParameters)
{
	// delay to offset task relative to others, then start
	vTaskDelay(STATE_HANDLING_TASK_FREQ_OFFSET);
	TickType_t prev_wake_time = xTaskGetTickCount();

	init_task_state(STATE_HANDLING_TASK);

	for ( ;; )
	{
		vTaskDelayUntil(&prev_wake_time, STATE_HANDLING_TASK_FREQ / portTICK_PERIOD_MS);

		// report to watchdog
		report_task_running(STATE_HANDLING_TASK);

		#if OVERRIDE_STATE_HOLD_INIT != 1
			// handle antenna deploy task seperately
			handle_antenna_deploy_task();
		
			decide_next_state(get_sat_state());
		#endif
	}

	// delete this task if it ever breaks out
	vTaskDelete(NULL);
}

void decide_next_state(sat_state_t current_state) {

	// if the current state is RIP we don't want to do anything
	if (current_state != RIP)
	{
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
		read_lf_volts_precise(&lf1_mv, &lf2_mv, &lf3_mv, &lf4_mv);

		// average voltage for the batteries within each LF bank
		int lfb1_avg_mv = (lf1_mv + lf2_mv) / 2;
		int lfb2_avg_mv = (lf3_mv + lf4_mv) / 2;

		///
		// we always will need a check whether we want to go into RIP (or a low
		// power state is the data is conflicted)
		///

		sat_state_t checked_state = check_for_end_of_life(lf1_mv, lf2_mv, current_state);
		if (checked_state != current_state)
		{
			// checked state will be either RIP or LOW_POWER
			set_sat_state(checked_state);
			return;
		}

		///
		// now it's time to check for all of the standard state changes
		///

		int both_li_above_down = li1_mv > LI_DOWN_MV && li2_mv > LI_DOWN_MV;

		int one_lf_above_flash = lfb1_avg_mv > LF_FLASH_AVG_MV || lfb2_avg_mv > LF_FLASH_AVG_MV;

		int one_li_below_low_power = li1_mv <= LI_LOW_POWER_MV || li2_mv <= LI_LOW_POWER_MV;
		int one_li_below_down = li1_mv <= LI_DOWN_MV || li2_mv <= LI_DOWN_MV;
		int low_power_entry_criteria = charging_data.curr_meta_charge_state == TWO_LI_DOWN
										|| charging_data.curr_meta_charge_state == TWO_LF_DOWN
										|| (charging_data.curr_meta_charge_state == ALL_GOOD &&
											one_li_below_low_power)
										|| (charging_data.curr_meta_charge_state == ONE_LI_DOWN &&
											(charging_data.decommissioned[LI1] ? true : (li1_mv > LI_LOW_POWER_MV))
											&& charging_data.decommissioned[LI2] ? true : (li2_mv > LI_LOW_POWER_MV));
		int low_power_exit_criteria = !low_power_entry_criteria;

		// TODO: do we want some notion of time?
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

				// if the antenna is open kill the task because the antenna has been deployed
				// or kill it if it's run more than 5 times because it's a lost cause
				} else if (sat_history.antenna_deployed || should_exit_antenna_deploy()) { // TODO: really use persistent state as short circuit???
					// switch state to hello world, then determine whether we should keep trying
					// (we WON'T be suspended on state change)
					set_sat_state(HELLO_WORLD);

					// suspend antenna deploy task if necessary
					handle_antenna_deploy_task();
				}
				break;

			case HELLO_WORLD:
				// it's higher priority to go to low power
				if (low_power_entry_criteria)
					set_sat_state(LOW_POWER);
				else if (get_current_timestamp() > MIN_TIME_IN_BOOT_S)
					set_sat_state(IDLE_NO_FLASH);
				break;

			case IDLE_NO_FLASH:
				// it's higher priority to go to low power
				if (low_power_entry_criteria)
					set_sat_state(LOW_POWER);
				else if (one_lf_above_flash)
					set_sat_state(IDLE_FLASH);
				break;

			case IDLE_FLASH:
				// it's higher priority to go to low power
				if (low_power_entry_criteria)
					set_sat_state(LOW_POWER);
				else if (!one_lf_above_flash)
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
				log_error(ELOC_STATE_HANDLING, ECODE_UNEXPECTED_CASE, true);
				set_sat_state(IDLE_NO_FLASH); // default state
				break;
		}
	}
}
