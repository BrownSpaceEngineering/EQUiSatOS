/*
 * intTesting.h
 *
 * Created: 3/24/2017 5:42:20 PM
 *  Author: rizant
 */ 


#ifndef INTTESTING_H_
#define INTTESTING_H_


#include "run.h"

void flatsat_init(void);
float read_IR(void);
MPU9250Reading read_IMU(void);
float read_temp(void);
void led_flash(void);
void integratedTesting_run(void);



#endif /* INTTESTING_H_ */