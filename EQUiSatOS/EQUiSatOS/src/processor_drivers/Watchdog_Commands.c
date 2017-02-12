/*
 * Watchdog_Commands.c
 *
 * Created: 2/8/2017 8:24:11 PM
 *  Author: BSE
 */ 

#include "Watchdog_Commands.h"

void configure_watchdog(void){
	struct wdt_conf config_wdt;
	wdt_get_config_defaults(&config_wdt);
	
	config_wdt.always_on = false;
	
	config_wdt.clock_source = WATCHDOG_CLOCK;
	
	config_wdt.timeout_period = WATCHDOG_PERIOD;
	
	config_wdt.early_warning_period = WATCHDOG_WARNING_PERIOD;
	
	wdt_set_config(&config_wdt);
	
	//only call when xplained pro is used
	//configure_watchdog_callbacks();
}

//Only used for xplained pro
void watchdog_early_warning_callback(void)
{
	port_pin_set_output_level(LED_0_PIN, LED_0_ACTIVE);
}

//only call when xplained pro is used
void configure_watchdog_callbacks(void)
{
	//! [setup_5]
	wdt_register_callback(watchdog_early_warning_callback, WDT_CALLBACK_EARLY_WARNING);
	//! [setup_5]

	//! [setup_6]
	wdt_enable_callback(WDT_CALLBACK_EARLY_WARNING);
	//! [setup_6]
}

void reset_watchdog(void){
	wdt_reset_count();
}

bool did_watchdog_reset(void){
	return (system_get_reset_cause() == SYSTEM_RESET_CAUSE_WDT);
}