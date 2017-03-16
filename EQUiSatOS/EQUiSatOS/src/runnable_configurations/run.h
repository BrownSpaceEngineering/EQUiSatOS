/*
 * run.h
 *
 * Created: 9/18/2016 4:07:44 PM
 *  Author: Daniel
 */ 


#ifndef RUN_H_
#define RUN_H_

#include <asf.h>
#include <inttypes.h>

#include "processor_drivers/I2C_Commands.h"
#include "processor_drivers/SPI_Commands.h"
#include "processor_drivers/ADC_Commands.h"
#include "processor_drivers/Direct_Pin_Commands.h"
#include "processor_drivers/USART_Commands.h"
#include "processor_drivers/PWM_Commands.h"
#include "processor_drivers/Watchdog_Commands.h"
#include "sensor_drivers/MLX90614_IR_Sensor.h"
#include "sensor_drivers/TEMD6200_Commands.h"
#include "sensor_drivers/switching_commands.h"
#include "sensor_drivers/MPU9250_9axis_Commands.h"
#include "sensor_drivers/LTC1380_Multiplexer_Commands.h"
#include "radio/Radio_Commands.h"
#include "processor_drivers/Watchdog_Commands.h"

#define TASK_MONITOR_STACK_SIZE            (2048/sizeof(portSTACK_TYPE))
#define TASK_MONITOR_STACK_PRIORITY        (tskIDLE_PRIORITY)
#define TASK_LED_STACK_SIZE                (1024/sizeof(portSTACK_TYPE))
#define TASK_LED_STACK_PRIORITY            (tskIDLE_PRIORITY)
#define TASK_ADC_STACK_SIZE                (4096/sizeof(portSTACK_TYPE))
#define TASK_ADC_STACK_PRIORITY	           (tskIDLE_PRIORITY)

extern void vApplicationStackOverflowHook(TaskHandle_t *pxTask,
signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
static void task_monitor(void *pvParameters);
static void task_led(void *pvParameters);
static void task_adc_read(void *pvParameters);
static void task_spi_read(void *pvParameters);

#endif /* RUN_H_ */