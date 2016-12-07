/*
 * stack_tests.c
 *
 * Created: 11/15/2016 8:47:57 PM
 *  Author: rj16
 */ 

#include "stack_tests.h"

Num_Stack s;

void Num_Stack_Write1(void *pvParameters)
{
	for ( ;; )
	{
		vTaskDelay(10);
		Num_Stack_Push(&s, 1);
	}
	
	vTaskDelete( NULL );
}

void Num_Stack_Write2(void *pvParameters)
{
	for ( ;; )
	{
		vTaskDelay(15);
		Num_Stack_Push(&s, 2);
	}
	
	vTaskDelete( NULL );
}

void Num_Stack_Read1(void *pvParameters)
{
	for ( ;; )
	{
		vTaskDelay(20);
		Num_Stack_Top(&s);
	}
	
	vTaskDelete( NULL );
}


void Num_Stack_Read2(void *pvParameters)
{
	for ( ;; )
	{
		vTaskDelay(40);
		Num_Stack_Top(&s);
	}
	
	vTaskDelete( NULL );
}

// Num_Stack tests
void Num_Stack_Test(void *pvParameters)
{
	Num_Stack_Init(&s);
	
	Num_Stack_Push(&s, 1);
	int one = Num_Stack_Top(&s);
	
	Num_Stack_Push(&s, 2);
	int two = Num_Stack_Top(&s);
	
	Num_Stack_Push(&s, 3);
	int three = Num_Stack_Top(&s);
	
	Num_Stack_Push(&s, 4);
	int four = Num_Stack_Top(&s);
	
	Num_Stack_Push(&s, 5);
	int five = Num_Stack_Top(&s);
	
	Num_Stack_Push(&s, 6);
	int six = Num_Stack_Top(&s);
	
	Num_Stack_Push(&s, 7);
	int seven = Num_Stack_Top(&s);
	
	Num_Stack_Push(&s, 8);
	int eight = Num_Stack_Top(&s);
	
	Num_Stack_Push(&s, 9);
	int nine = Num_Stack_Top(&s);
	
	Num_Stack_Push(&s, 10);
	int ten = Num_Stack_Top(&s);
}

void Num_Stack_Test_Mutex()
{
	xTaskCreate(
		Num_Stack_Write1,       /* Function that implements the task. */
		"Write1",          /* Text name for the task. */
		(1024/sizeof(portSTACK_TYPE)),      /* Stack size in words, not bytes. */
		NULL,    /* Parameter passed into the task. */
		tskIDLE_PRIORITY,/* Priority at which the task is created. */
		NULL);      /* Used to pass out the created task's handle. */
	
	vTaskStartScheduler();
}
