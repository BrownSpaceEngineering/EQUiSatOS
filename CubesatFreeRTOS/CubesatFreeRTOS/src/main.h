/*
 * IncFile1.h
 *
 * Created: 5/8/2015 3:34:09 PM
 *  Author: Daniel
 */ 


#ifndef MAIN_H
#define MAIN_H

#include <asf.h>
#include "conf_board.h"
#include "conf_uart_serial.h"
#include "task.h"
#include <inttypes.h>
#include "adc.h"
#include "spi.h"
#include "spi_interrupt.h"

#include "console.h"
#include "I2C_Commands.h"
#include "SPI_Commands.h"
#include "ADC_Commands.h"
#include "Direct_Pin_Commands.h"

#include "HMC5883L_Magnetometer_Commands.h"
#include "MLX90614_IR_Sensor.h"
#include "TEMD6200_Commands.h"
#include "switching_commands.h"

#define TASK_MONITOR_STACK_SIZE            (2048/sizeof(portSTACK_TYPE))
#define TASK_MONITOR_STACK_PRIORITY        (tskIDLE_PRIORITY)
#define TASK_LED_STACK_SIZE                (1024/sizeof(portSTACK_TYPE))
#define TASK_LED_STACK_PRIORITY            (tskIDLE_PRIORITY)
#define TASK_ADC_STACK_SIZE                (4096/sizeof(portSTACK_TYPE))
#define TASK_ADC_STACK_PRIORITY	           (tskIDLE_PRIORITY)


static struct usart_module cdc_uart_module;

extern void vApplicationStackOverflowHook(TaskHandle_t *pxTask,
signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
static void task_monitor(void *pvParameters);
static void task_led(void *pvParameters);
static void task_adc_read(void *pvParameters);
static void task_spi_read(void *pvParameters);


#endif /* MAIN_H */
