/*
 * RTC_Commands.c
 *
 * Based on: http://asf.atmel.com/docs/3.17.0/samd21/html/asfdoc_sam0_rtc_calendar_callback_use_case.html
 *
 *
 * Author: Arun Drelich
 */

#include "RTC_Commands.h"

void init_rtc(void)
{
	struct rtc_count_config config_rtc_count;
	rtc_count_get_config_defaults(&config_rtc_count);

	// These are all defaults but will set them explicitly anyway
	config_rtc_count.prescaler = RTC_COUNT_PRESCALER_DIV_1024;
	config_rtc_count.mode = RTC_COUNT_MODE_32BIT;
	config_rtc_count.clear_on_match = false;
	config_rtc_count.continuously_update = false;

	int status_code = 0;
	int retries = 0;
	
	while ((status_code = rtc_count_init(&rtc_instance, RTC, &config_rtc_count)) != STATUS_OK &&
		   retries <= 2)
	{
		retries++;
	}
	
	if (status_code != STATUS_OK)
	{
		system_reset();
	} else {
		rtc_count_enable(&rtc_instance);
	}
}

int get_rtc_count(void)
{
	return rtc_count_get_count(&rtc_instance);
}