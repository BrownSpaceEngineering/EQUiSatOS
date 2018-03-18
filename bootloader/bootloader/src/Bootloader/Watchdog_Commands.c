/*
 * Watchdog_Commands.c
 *
 * Created: 2/8/2017 8:24:11 PM
 *  Author: BSE
 */ 

#include "Watchdog_Commands.h"

void configure_watchdog(const wdt_callback_t early_warning_callback) {
	// only attempt to configure if the watchdog's config is not already locked on
	if (!wdt_is_locked()) {
		struct wdt_conf config_wdt;
		wdt_get_config_defaults(&config_wdt);
		
		config_wdt.always_on = WATCHDOG_ALWAYS_ON;
		config_wdt.clock_source = WATCHDOG_CLOCK;
		config_wdt.timeout_period = WATCHDOG_PERIOD;
		config_wdt.early_warning_period = WATCHDOG_WARNING_PERIOD;
		
		wdt_set_config(&config_wdt);
	}
	
	if (early_warning_callback) {
		wdt_register_callback(early_warning_callback, WDT_CALLBACK_EARLY_WARNING);
		wdt_enable_callback(WDT_CALLBACK_EARLY_WARNING);
	}
}

void pet_watchdog(void) {
	wdt_reset_count();
}
