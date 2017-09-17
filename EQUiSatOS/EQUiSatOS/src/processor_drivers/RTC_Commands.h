/* 
 * RTC_Commands.h
 *
 * Author: Arun Drelich
 */

#ifndef _RTC_COMMANDS_H
#define _RTC_COMMANDS_H

#include <global.h>

/* NOTE: The conf_clocks.h file must have set CONF_CLOCK_OSC32K_ENABLE and CONF_CLOCK_GCLK_2_ENABLE to true */

struct rtc_module rtc_instance; // global rtc_module struct

void configure_rtc_calendar(struct rtc_calendar_time *alarm_time, enum rtc_calendar_alarm_mask mask_option);
void configure_rtc_callbacks(void (*callback_action)(void));
#endif
