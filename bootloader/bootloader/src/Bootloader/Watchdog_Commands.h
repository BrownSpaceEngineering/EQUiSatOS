/*
 * Watchdog_Commands.h
 *
 * Created: 2/8/2017 8:23:47 PM
 *  Author: BSE
 */ 

#ifndef WATCHDOG_COMMANDS_H_
#define WATCHDOG_COMMANDS_H_

#include <wdt.h>

#define WATCHDOG_ALWAYS_ON		false
#define WATCHDOG_CLOCK			GCLK_GENERATOR_2
#define WATCHDOG_PERIOD			WDT_PERIOD_16384CLK
#define WATCHDOG_WARNING_PERIOD WDT_PERIOD_8192CLK

void configure_watchdog(const wdt_callback_t early_warning_callback);
void pet_watchdog(void);

#endif /* WATCHDOG_COMMANDS_H_ */