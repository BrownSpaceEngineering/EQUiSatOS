#include "rtos_tasks.h"

#define MIN_TIME_IN_INITIAL_S      (30*60)
#define MIN_TIME_IN_BOOT_S         (26*ORBITAL_PERIOD_S)
#define TIME_TO_WAIT_FOR_CRIT_MS   2000

// returns -1 if no change required
sat_state_t check_for_end_of_life(int li1_mv, int li2_mv, sat_state_t current_state)
{
  // enter the rip state if both are lion_critical_mv
  if (li1_mv <= li_critical_mv && li2_mv <= li_critical_mv)
  {
    int end_of_life = 1;

    // we want to be very sure that both are actually lion_critical_mv
    for (int i = 0; i < 5; i++)
    {
      // TODO: see if there's a more elegant way to make sure that we're reading
      // the up to date Lion voltages
      vTaskDelay(TIME_TO_WAIT_FOR_CRIT_MS);

      // TODO: this will most likely result in seg fault because of the NULL being
      // passed in
      int li1_recalc_mv;
      int li2_recalc_mv;
      get_current_lion_volts(
        &li1_recalc_mv,
        &li2_recalc_mv,
        NULL);

      if (!(li1_recalc_mv <= li_critical_mv && li2_recalc_mv <= li_critical_mv))
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
        case HELLO_WORLD_LOW_POWER:
          return HELLO_WORLD_LOW_POWER;

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

  return -1;
}

void state_handling_task(void *pvParameters)
{
  // initialize xNextWakeTime onces
	TickType_t prev_wake_time = xTaskGetTickCount();

  while (true)
  {
    vTaskDelayUntil(&prev_wake_time, STATE_HANDLING_TASK_FREQ / portTICK_PERIOD_MS);

    // report to watchdog
    report_task_running(STATE_HANDLING_TASK);

    sat_state_t current_state = get_sat_state();

    // if the current state is RIP we don't want to do anything
    if (current_state != RIP)
    {
      ///
      // the state decision will be predicated on the current battery levels and
      // the timestamp -- we'll grab them here
      ///

      int li1_mv;
      int li2_mv;
      get_current_lion_volts(
        &li1_mv,
        &li2_mv,
        NULL);

      // individual batteries within the life po banks
      int lf1_mv;
      int lf2_mv;
      int lf3_mv;
      int lf4_mv;
      get_current_lifepo_volts(
        &lf1_mv,
        &lf2_mv,
        &lf3_mv,
        &lf4_mv,
        NULL);

      // average voltage for the batteries within each LF bank
      int lfb1_avg_mv = (lf1_mv + lf2_mv) / 2;
      int lfb2_avg_mv = (lf3_mv + lf4_mv) / 2;

      ///
      // we always will need a check whether we want to go into RIP (or a low
      // power state is the data is conflicted)
      ///

      sat_state_t checked_state = check_for_end_of_life(lf1_mv, lf2_mv, current_state);
      if (checked_state != -1)
      {
        // checked state will be either RIP or LOW_POWER
        set_sat_state(checked_state);
        continue;
      }

      ///
      // now it's time to check for all of the standard state changes
      ///

      int both_li_above_down = li1_mv > li_down_mv && li2_mv > li_down_mv;
      int one_li_below_low_power = li1_mv <= li_low_power_mv || li2_mv <= li_low_power_mv;

      int one_lf_above_flash = lfb1_avg_mv > lf_flash_avg_mv || lfb2_avg_mv > lf_flash_avg_mv;

      switch (current_state)
      {
        case INITIAL:
          if (get_current_timestamp() > MIN_TIME_IN_INITIAL_S &&
              both_li_above_down)
            set_sat_state(ANTENNA_DEPLOY);
          break;

        case ANTENNA_DEPLOY:
          // this is the one case in which the another task is responsible for
          // state changes
          break;

        // TODO: thresholds to go in and out of low power are subject to change
        case HELLO_WORLD:
          // it's higher priority to go to low power
          if (one_li_below_low_power)
            set_sat_state(HELLO_WORLD_LOW_POWER);
          else if (get_current_timestamp() > MIN_TIME_IN_BOOT_S)
            set_sat_state(IDLE_NO_FLASH);
          break;

        case HELLO_WORLD_LOW_POWER:
          if (both_li_above_down)
            set_sat_state(HELLO_WORLD);
          break;

        case IDLE_NO_FLASH:
          // it's higher priority to go to low power
          if (one_li_below_low_power)
            set_sat_state(LOW_POWER);
          else if (one_lf_above_flash)
            set_sat_state(IDLE_FLASH);
          break;

        case IDLE_FLASH:
          // it's higher priority to go to low power
          if (one_li_below_low_power)
            set_sat_state(LOW_POWER);
          else if (!one_lf_above_flash)
            set_sat_state(IDLE_NO_FLASH);
          break;

        case LOW_POWER:
          if (both_li_above_down)
            set_sat_state(IDLE_NO_FLASH);
          break;

        default:
          assert(false);
          log_error(ELOC_STATE_HANDLING, ECODE_UNEXPECTED_CASE, true);
          set_sat_state(INITIAL);
          break;
      }
    }
  }

  // delete this task if it ever breaks out
  vTaskDelete(NULL);
}
