/*
 * stack_tests.h
 *
 * Created: 11/15/2016 9:17:40 PM
 *  Author: rj16
 */ 


#ifndef STACK_TESTS_H_
#define STACK_TESTS_H_

#include "radio/Stacks/EquiStacks.h"
#include "runnable_configurations/init_rtos_tasks.h"
#include "runnable_configurations/rtos_tasks.h"

void Num_Stack_Test();
void Num_Stack_Test_Mutex();
void Num_Stack_Write1(void *pvParameters);
void Num_Stack_Write2(void *pvParameters);
void Num_Stack_Read1(void *pvParameters);
void Num_Stack_Read2(void *pvParameters);

#endif /* STACK_TESTS_H_ */