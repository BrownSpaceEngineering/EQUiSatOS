///
// Arun's old calendar code for bit flip testing
///

/* Configure the RTC calendar driver to enable an alarm with the given conditions.
 * Parameters:
 *  alarm_time: see (http://asf.atmel.com/docs/3.17.0/samd21/html/structrtc__calendar__time.html)
 *  mask_option: see (http://asf.atmel.com/docs/3.17.0/samd21/html/group__asfdoc__sam0__rtc__calendar__group.html#gac9610022ffc50d5da111b79f0fcc0f74)
 */
// void configure_rtc_calendar(struct rtc_calendar_time *alarm_time, enum rtc_calendar_alarm_mask mask_option) {
  /* Initialize RTC in calendar mode. */
  struct rtc_calendar_config config_rtc_calendar;
  rtc_calendar_get_config_defaults(&config_rtc_calendar);

  /* I think alarm is a global struct defined somewhere?
  alarm.time.year      = 2013;
  alarm.time.month     = 1;
  alarm.time.day       = 1;
  alarm.time.hour      = 0;
  alarm.time.minute    = 0;
  alarm.time.second    = 4; */

  config_rtc_calendar.clock_24h = true;
  config_rtc_calendar.alarm[0].time = *alarm_time;
  config_rtc_calendar.alarm[0].mask = mask_option;

  rtc_calendar_init(&rtc_instance, RTC, &config_rtc_calendar);
  rtc_calendar_enable(&rtc_instance);
}

/* Bind the given callback function. Whenever the RTC alarm matches the conditions setup in
 * configure_rtc_calendar(), perform the callback action.
 */
void configure_rtc_callbacks(void (*callback_action)(void)) {
  rtc_calendar_register_callback(&rtc_instance, callback_action, RTC_CALENDAR_CALLBACK_ALARM_0);
  rtc_calendar_enable_callback(&rtc_instance, RTC_CALENDAR_CALLBACK_ALARM_0);
}