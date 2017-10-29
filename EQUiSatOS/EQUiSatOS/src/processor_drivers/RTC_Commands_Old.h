/*
 * IncFile1.h
 *
 * Created: 10/29/2017 3:54:05 PM
 *  Author: rj16
 */ 


#ifndef INCFILE1_H_
#define INCFILE1_H_

///
// Arun's old code for bit flip testing
///

struct rtc_module rtc_instance; // global rtc_module struct

void configure_rtc_calendar(struct rtc_calendar_time *alarm_time, enum rtc_calendar_alarm_mask mask_option);
void configure_rtc_callbacks(void (*callback_action)(void));


#endif /* INCFILE1_H_ */