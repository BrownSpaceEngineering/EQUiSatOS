/*
 * RTC_Commands.h
 *
 * Author: Arun Drelich
 */

#ifndef _RTC_COMMANDS_H
#define _RTC_COMMANDS_H

#include <global.h>

/* NOTE: The conf_clocks.h file must have set CONF_CLOCK_OSC32K_ENABLE and CONF_CLOCK_GCLK_2_ENABLE to true */

struct rtc_module rtc_instance;

void init_rtc(void);
int get_rtc_count(void);

#endif