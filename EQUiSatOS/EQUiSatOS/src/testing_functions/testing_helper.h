/*
 * testing_helper.h
 *
 * Created: 12/13/2016 9:11:50 PM
 *  Author: rj16
 */ 

#ifndef TESTING_HELPER_H_
#define TESTING_HELPER_H_

#include "../runnable_configurations/init_rtos_tasks.h"
#include "../runnable_configurations/rtos_tasks.h"
#include "../stacks/equistacks.h"
#include "../stacks/State_Structs.h"

idle_data_t* create_dummy_batch();
void* create_dummy_arr(int* arr, int val);

#endif /* TESTING_HELPER_H_ */