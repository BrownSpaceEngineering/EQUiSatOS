/*
 * antenna_pwm.h
 *
 * Created: 12/2/17 8:58:35 PM
 *  Author: jacobleiken
 */ 


#ifndef ANTENNA_PWM_H_
#define ANTENNA_PWM_H_

#include <global.h>

void pwm_test(void);
void try_pwm_deploy(int pin, int pin_mux);

#endif /* ANTENNA_PWM_H_ */