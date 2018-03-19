/*
 * antenna_pwm.h
 *
 * Created: 12/2/17 8:58:35 PM
 *  Author: jacobleiken
 */ 


#ifndef ANTENNA_PWM_H_
#define ANTENNA_PWM_H_

#include <global.h>

#define PWM_LENGTH_MS			3000
#define PWM_LION_MIN_V			4100
#define PWM_LIFEPO_MIN_V		6000
#define PWM_MAX_CUR_L			5		//-1960 mA
#define PWM_MIN_CUR_L			485		//-1000 mA
#define PWM_VERY_MIN_CUR_L		635		//-700 mA
#define PWM_MAX_CUR_LF			940		//-2000 mA
#define PWM_MIN_CUR_LF			960		//-1000 mA
#define PWM_VERY_MIN_CUR_LF		966		//-700 mA

bool antenna_did_deploy(void);
bool try_pwm_deploy(long pin, long pin_mux, int ms, uint8_t p_ant);
int get_current_pwm_pin(void);

#endif /* ANTENNA_PWM_H_ */