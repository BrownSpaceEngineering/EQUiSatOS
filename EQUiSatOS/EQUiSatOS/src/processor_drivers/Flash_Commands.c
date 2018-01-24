/*
 * Flash_Commands.c
 *
 * Created: 9/28/2017 19:31:03
 *  Author: mckenna
 */

#include "Flash_Commands.h"

void set_lifepo_output_enable(bool enabled)
{
	#ifdef FLASH_ACTIVE
		set_output(enabled, P_LF_B1_OUTEN);
		set_output(enabled, P_LF_B2_OUTEN);
	#endif
}

void set_lifepo_charge_disable(void)
{
	set_output(false, P_LF_B1_RUNCHG);
	set_output(false, P_LF_B1_RUNCHG);
}

void set_lifepo_charge_enable(void)
{
	if (batt_charging == LFB1)
		set_output(true, P_LF_B1_RUNCHG);
	else if (batt_charging == LFB2)
		set_output(true, P_LF_B2_RUNCHG);
}

void flash_arm()
{
	#ifdef FLASH_ACTIVE
		// TODO: go over the logic around the use of a mutex here

		// makes sure the battery charging task won't step in and change anything
		xSemaphoreTake(battery_charging_mutex, (TickType_t) BAT_MUTEX_WAIT_TIME_TICKS);

		set_lifepo_charge_disable(); // make sure lifepo's aren't charging
		set_lifepo_output_enable(true);
		set_output(true, P_LED_CMD);
		// should be followed by a delay of at least 2ms before calling flash_leds
	#endif
}

void flash_activate(void)
{
	#ifdef FLASH_ACTIVE
		// send _falling_ edge on LED_CMD to tell subprocessor to activate LEDs for 100ms
		set_output(false, P_LED_CMD);
		// should be followed by a delay of at least 2ms before calling disarm_flash
	#endif
}

void flash_disarm(void)
{
	#ifdef FLASH_ACTIVE
		set_lifepo_output_enable(false);
		set_lifepo_charge_enable(); // set the lifepo's to be charging again

		// makes sure the battery charging task won't step in and change anything
		xSemaphoreGive(battery_charging_mutex);
	#endif
}
