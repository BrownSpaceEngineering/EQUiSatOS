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
	if (charging_data.bat_charging == LFB1)
		set_output(true, P_LF_B1_RUNCHG);
	else if (charging_data.bat_charging == LFB2)
		set_output(true, P_LF_B2_RUNCHG);
}

// returns true if now active (false if couldn't acquire battery mutex)
void flash_arm(void)
{
	#ifdef FLASH_ACTIVE
		// make sure lifepo's aren't charging (we should have the critical action mutex so this doesn't violate charging)
		set_lifepo_charge_disable(); 
		
		set_lifepo_output_enable(true);
		set_output(true, P_LED_CMD);
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
		
		// set the lifepo's to be charging again (we have the critical action mutex)
		set_lifepo_charge_enable(); 
	#endif
}
