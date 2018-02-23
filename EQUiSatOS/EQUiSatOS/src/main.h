/*
 * main.h
 *
 * Created: 5/8/2015 3:34:09 PM
 *  Author: Daniel
 */


#ifndef MAIN_H
#define MAIN_H

#include <global.h>
#include "runnable_configurations/satellite_state_control.h"
#include "runnable_configurations/scratch_testing.h"
#include "testing_functions/test_stacks.h"
#include "testing_functions/system_test.h"

void run_tests(void);
void run_rtos_tests(void);

#endif /* MAIN_H */
