/*
 * antenna_pwm.h
 *
 * Created: 12/2/17 8:58:35 PM
 *  Author: jacobleiken
 */ 


#ifndef ANTENNA_PWM_H_
#define ANTENNA_PWM_H_

#include <global.h>

#define PWM_LENGTH_MS		1000
#define PWM_LION_MIN_V		3800 // TODO: set back to 4100
#define PWM_LIFEPO_MIN_V	6600
#define PWM_MAX_CUR			2000

void pwm_test(void);
void try_pwm_deploy(long pin, long pin_mux, int ms, uint8_t p_ant);
int get_current_pwm_pin(void);

#endif /* ANTENNA_PWM_H_ */