/*
 * main.h
 *
 * Created: 5/8/2015 3:34:09 PM
 *  Author: Daniel
 */


#ifndef MAIN_H
#define MAIN_H

// BOARD TYPE - ALSO LOOK AT SERCOM_PINOUT.H, it has to have it's own version changed too!!!
#define CNTRL_BRD_V3 // to use print, etc. on real control board
//#define XPLAINED_PRO  // to use print, etc. on devboard

#include <global.h>
#include "runnable_configurations/init_rtos_tasks.h"
#include "runnable_configurations/scratch_testing.h"
#include "testing_functions/test_stacks.h"

static void The_Fall_Of_Ryan(void);
static void run_tests(void);
static void global_init(void);

#endif /* MAIN_H */
