/*
 * init_rtos_tasks.c
 *
 * Created: 10/4/2016 8:50:22 PM
 *  Author: mckenna
 */ 

#include "init_rtos_tasks.h"
#include "task_testing.h"

void runit_2()
{
	//configure_i2c_master(SERCOM4);
	
	// check that constants 
	assertConstantDefinitions();
	
	// Initialize EQUiStacks
	idle_readings_equistack = equistack_Init(sizeof(idle_data_t), IDLE_STACK_MAX);
	flash_readings_equistack = equistack_Init(sizeof(flash_data_t), FLASH_STACK_MAX);
 	transmit_readings_equistack = equistack_Init(sizeof(transmit_data_t), TRANSMIT_STACK_MAX);
 	attitude_readings_equistack = equistack_Init(sizeof(attitude_data_t), ATTITUDE_STACK_MAX);
	
	
	// Started at boot
	xTaskCreate(battery_charging_task,
		"battery charging action task",
		TASK_BATTERY_CHARGE_STACK_SIZE,
		NULL,
		TASK_BATTERY_CHARGE_PRIORITY,
		battery_charging_task_handle);// TODO: Should handle be NULL?
		
	xTaskCreate(antenna_deploy_task,
		"antenna deploy action task",
		TASK_ANTENNA_DEPLOY_STACK_SIZE,
		NULL,
		TASK_ANTENNA_DEPLOY_PRIORITY,
		antenna_deploy_task_handle);	// TODO: Should handle be NULL?
	
	xTaskCreate(watchdog_task,
		"watchdog task",
		TASK_WATCHDOG_STACK_SIZE,
		NULL,
		TASK_WATCHDOG_STACK_PRIORITY,
		watchdog_task_handle); // TODO: Should handle be NULL?
		
	// TODO: Will these not be created here?
	// i.e. would they be created when they need to be started?
	// Would that work after the scheduler has been started?
	xTaskCreate(flash_activate_task,
		"flash action task",
		TASK_FLASH_ACTIVATE_STACK_SIZE,
		NULL,
		TASK_FLASH_ACTIVATE_PRIORITY,
		flash_activate_task_handle);
			
	xTaskCreate(transmit_task,
		"transmit action task",
		TASK_TRANSMIT_STACK_SIZE,
		NULL,
		TASK_TRANSMIT_PRIORITY,
		transmit_task_handle);
			
	xTaskCreate(current_data_task,
		"current data reader task",
		TASK_CURRENT_DATA_RD_STACK_SIZE,
		NULL,
		TASK_CURRENT_DATA_RD_PRIORITY,
		current_data_task_handle);

	xTaskCreate(attitude_data_task,
		"attitude data reader task",
		TASK_ATTITUDE_DATA_RD_STACK_SIZE,
		NULL,
		TASK_ATTITUDE_DATA_DATA_RD_PRIORITY,
		attitude_data_task_handle);
		
// 		xTaskCreate(task_suicide_test,
// 		"task suicide testing",
// 		TASK_SENS_RD_IDLE_STACK_SIZE,
// 		NULL,
// 		TASK_SENS_RD_IDLE_PRIORITY,
// 		suicide_test_handle);

// 		xTaskCreate(task_stack_size_overflow_test,
// 			"task stack size purpose test",
// 			320/sizeof(portSTACK_TYPE), // 10 bytes of stack space
// 			NULL,
// 			tskIDLE_PRIORITY,
// 			suicide_test_handle);

  /*xTaskCreate(test_free,
  		"Tests the freeing of structs",
  		TASK_SENS_RD_IDLE_STACK_SIZE,
  		NULL,
  		TASK_SENS_RD_IDLE_PRIORITY,
  		NULL);*/
	
	/*print("hello, world");*/
	
  	// Make sure we define the first state
  	set_state_idle();
			
	/* Start the tasks and timer running. */
	vTaskStartScheduler();
}

void set_state_hello_world()
{
	CurrentState = HELLO_WORLD;
	
	// run only the attitude data task
	vTaskSuspend(current_data_task_handle);
	vTaskSuspend(flash_activate_task_handle);
	vTaskSuspend(transmit_task_handle);
	if (attitude_data_task_handle != NULL) { vTaskResume(attitude_data_task_handle); }
}

void set_state_idle()
{
	CurrentState = IDLE;

	// TODO: we need to suspend the other tasks and somehow immediately add OR delete their interior structs and make a new one
	// Maybe look for changes in state inside the rtos tasks?
	// OR bring their current structs, etc. global so we can manually reset them? -> NOOOOO
	if (current_data_task_handle != NULL) { vTaskResume(current_data_task_handle); }
	if (flash_activate_task_handle != NULL) { vTaskResume(flash_activate_task_handle); }
	if (transmit_task_handle != NULL) { vTaskResume(transmit_task_handle); }
	if (attitude_data_task_handle != NULL) { vTaskResume(attitude_data_task_handle); }
}

void set_state_low_power()
{
	CurrentState = LOW_POWER;
	
	// TODO: ibid
	if (current_data_task_handle != NULL) { vTaskResume(current_data_task_handle); }
	vTaskSuspend(flash_activate_task_handle);
	if (transmit_task_handle != NULL) { vTaskResume(transmit_task_handle); }
	vTaskSuspend(attitude_data_task_handle); // TODO: Do this?
}