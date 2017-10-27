/*
 * bat_testing.h
 *
 * Created: 10/26/2017 3:26:50 PM
 *  Author: Ryan Izant
 */ 


#ifndef BAT_TESTING_H_
#define BAT_TESTING_H_

#include "../global.h"

void bat_testing_init();
void readRemoteADC_0(float* batReadings);
void bat_testing_run();



#endif /* BAT_TESTING_H_ */