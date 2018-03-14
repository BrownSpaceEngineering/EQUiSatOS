/*
 * Watchdog_Commands.c
 *
 * Created: 2/8/2017 8:24:11 PM
 *  Author: BSE
 */ 

#include "Watchdog_Commands.h"

void configure_watchdog(void) {
	#ifdef WATCHDOG_RESET_ACTIVE
		struct wdt_conf config_wdt;
		wdt_get_config_defaults(&config_wdt);
	
		config_wdt.always_on = false;
	
		config_wdt.clock_source = WATCHDOG_CLOCK;
	
		config_wdt.timeout_period = WATCHDOG_PERIOD;
	
		config_wdt.early_warning_period = WATCHDOG_WARNING_PERIOD;
	
		wdt_set_config(&config_wdt);
	
		wdt_register_callback(watchdog_early_warning_callback, WDT_CALLBACK_EARLY_WARNING);
		// only use with xplained pro
		//wdt_register_callback(watchdog_early_warning_callback_xplained, WDT_CALLBACK_EARLY_WARNING);
		wdt_enable_callback(WDT_CALLBACK_EARLY_WARNING);
	#endif
}

/*
// use for xplained pro
static void watchdog_early_warning_callback_xplained(void) {
	port_pin_set_output_level(LED_0_PIN, LED_0_ACTIVE);
}
*/

void pet_watchdog(void){
	wdt_reset_count();
}
