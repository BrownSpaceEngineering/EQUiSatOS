/*
 * antenna_pwm.h
 *
 * Created: 12/2/17 8:58:35 PM
 *  Author: jacobleiken
 */ 


#ifndef ANTENNA_PWM_H_
#define ANTENNA_PWM_H_

#include <global.h>

#define PWM_LENGTH_MS 500

void pwm_test(void);
void try_pwm_deploy(int pin, int pin_mux, int ms, int p_ant);

#endif /* ANTENNA_PWM_H_ */