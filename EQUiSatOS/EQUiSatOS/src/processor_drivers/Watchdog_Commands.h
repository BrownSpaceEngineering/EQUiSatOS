/*
 * Watchdog_Commands.h
 *
 * Created: 2/8/2017 8:23:47 PM
 *  Author: BSE
 */ 


#ifndef WATCHDOG_COMMANDS_H_
#define WATCHDOG_COMMANDS_H_

#include <global.h>

void configure_watchdog(void);
void pet_watchdog(void);

#define WATCHDOG_CLOCK GCLK_GENERATOR_2

#define WATCHDOG_PERIOD WDT_PERIOD_16384CLK
#define WATCHDOG_WARNING_PERIOD WDT_PERIOD_8192CLK

#endif /* WATCHDOG_COMMANDS_H_ */