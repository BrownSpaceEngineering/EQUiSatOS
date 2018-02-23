/*
 * antenna_pwm.h
 *
 * Created: 12/2/17 8:58:35 PM
 *  Author: jacobleiken
 */ 


#ifndef ANTENNA_PWM_H_
#define ANTENNA_PWM_H_

#include <global.h>

#define PWM_LENGTH_MS		3000
#define PWM_LION_MIN_V		4100
#define PWM_LIFEPO_MIN_V	3300
#define PWM_MAX_CUR			2500

void pwm_test(void);
void try_pwm_deploy(int pin, int pin_mux, int ms, int p_ant);
int get_current_pwm_pin(void);

#endif /* ANTENNA_PWM_H_ */