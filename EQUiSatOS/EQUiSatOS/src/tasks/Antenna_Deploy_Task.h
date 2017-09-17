/*
 * PWM_Task.h
 *
 * Created: 9/14/2017 7:54:41 PM
 *  Author: jleiken
 */ 


#ifndef ANTENNA_DEPLOY_TASK_H_
#define ANTENNA_DEPLOY_TASK_H_

#include <global.h>
#include "../processor_drivers/PWM_Commands.h"
#include "../processor_drivers/Direct_Pin_Commands.h"
#include "../config/proc_pins.h"

void antenna_deploy_task(void *pvParameters);

#define ANTENNA_DEPLOY_TASK_FREQ 1000

#endif /* ANTENNA_DEPLOY_TASK_H_ */