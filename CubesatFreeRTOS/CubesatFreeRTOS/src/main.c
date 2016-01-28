/**
 * \file
 *
 * \brief FreeRTOS Real Time Kernel example.
 *
 * Copyright (c) 2012-2014 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

/**
 * \mainpage FreeRTOS Real Time Kernel example
 *
 * \section Purpose
 *
 * The FreeRTOS example will help users how to use FreeRTOS in SAM boards.
 * This basic application shows how to create task and get information of
 * created task.
 *
 * \section Requirements
 *
 * This package can be used with SAM boards.
 *
 * \section Description
 *
 * The demonstration program create two task, one is make LED on the board
 * blink at a fixed rate, and another is monitor status of task.
 *
 * \section Usage
 *
 * -# Build the program and download it inside the evaluation board. Please
 *    refer to the
 *    <a href="http://www.atmel.com/dyn/resources/prod_documents/doc6224.pdf">
 *    SAM-BA User Guide</a>, the
 *    <a href="http://www.atmel.com/dyn/resources/prod_documents/doc6310.pdf">
 *    GNU-Based Software Development</a>
 *    application note or to the
 *    <a href="ftp://ftp.iar.se/WWWfiles/arm/Guides/EWARM_UserGuide.ENU.pdf">
 *    IAR EWARM User Guide</a>,
 *    depending on your chosen solution.
 * -# On the computer, open and configure a terminal application
 *    (e.g. HyperTerminal on Microsoft Windows) with these settings:
 *   - 115200 bauds
 *   - 8 bits of data
 *   - No parity
 *   - 1 stop bit
 *   - No flow control
 * -# Start the application.
 * -# LED should start blinking on the board. In the terminal window, the
 *    following text should appear (values depend on the board and chip used):
 *    \code
	-- Freertos Example xxx --
	-- xxxxxx-xx
	-- Compiled: xxx xx xxxx xx:xx:xx --
\endcode
 *
 */

#include "main.h"

//extern void xPortSysTickHandler(void);

static void configure_console(void) {
	struct usart_config usart_conf;

	usart_get_config_defaults(&usart_conf);
	usart_conf.mux_setting = CONF_STDIO_MUX_SETTING;
	usart_conf.pinmux_pad0 = CONF_STDIO_PINMUX_PAD0;
	usart_conf.pinmux_pad1 = CONF_STDIO_PINMUX_PAD1;
	usart_conf.pinmux_pad2 = CONF_STDIO_PINMUX_PAD2;
	usart_conf.pinmux_pad3 = CONF_STDIO_PINMUX_PAD3;
	usart_conf.baudrate    = CONF_STDIO_BAUDRATE;

	stdio_serial_init(&cdc_uart_module, CONF_STDIO_USART_MODULE,
	&usart_conf);
	usart_enable(&cdc_uart_module);
}




/**
 * \brief Called if stack overflow during execution
 */
extern void vApplicationStackOverflowHook(TaskHandle_t *pxTask,
		signed char *pcTaskName)
{
	//printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
	/* If the parameters have been corrupted then inspect pxCurrentTCB to
	 * identify which task has overflowed its stack.
	 */
	for (;;) {
	}
}

/**
 * \brief This function is called by FreeRTOS idle task
 */
extern void vApplicationIdleHook(void)
{
}

/**
 * \brief This function is called by FreeRTOS each tick
 */
extern void vApplicationTickHook(void)
{
}

/**
 * \brief This task, when activated, send every ten seconds on debug UART
 * the whole report of free heap and total tasks status
 */
static void task_monitor(void *pvParameters)
{
	UNUSED(pvParameters);

    configure_console();
	for (;;) {
		//printf("hi");
		vTaskDelay(1000);
	}
}

/**
 * \brief This task, when activated, make LED blink at a fixed rate
 */
static void task_led(void *pvParameters)
{
	UNUSED(pvParameters);
	for (;;) {
		//printf("blinking\n");
	#if SAM4CM
		LED_Toggle(LED4);
	#else
		LED_Toggle(LED0);
	#endif
		vTaskDelay(1000);
	}
}

static void task_adc_read(void *pvParameters) {
	UNUSED(pvParameters);
	for (;;) {
		
	}
}

static void task_spi_read(void *pvParameters) {
	UNUSED(pvParameters);
	for (;;) {
		
	}
}

/**
 *  \brief FreeRTOS Real Time Kernel example entry point.
 *
 *  \return Unused (ANSI-C compatibility).
 */
int main(void)
{
	// Initilize the SAM system
	system_init();

	// Initialize the console uart
	configure_console();
	
	// Output demo infomation.
	printf("-- Freertos Example --\n\r");
	printf("-- %s\n\r", BOARD_NAME);
	printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);
	
	configure_i2c_master();
	
	while(true){
		float x = MLX90614_readObjectTempC();
		uint16_t data = MLX90614_readRawIRData(MLX90614_RAWIR1);
	}
	//READ FROM MAGNETOMETER
	HMC5883L_init();
	for (int j = 0; j < 15; j++) {				
	uint8_t readBuff[6] = {0, 0, 0, 0, 0, 0};
	HMC5883L_read(readBuff);
	int16_t xyzBuff[3] = {0, 0, 0};
	getXYZ(readBuff, xyzBuff);
	int i;
	for (i = 0; i < 6; i++) {		
		printf("%x ", readBuff[i]);					
	}
	printf("\n\r");
	printf("%d %d %d\n\r", xyzBuff[0], xyzBuff[1], xyzBuff[2]);	
	float heading = computeCompassDir(xyzBuff[0], xyzBuff[1], xyzBuff[2]);	
	int a = 0;
	}
	
	
	/*struct adc_module temp_instance;
	configure_adc(&temp_instance);
	int i = 0;
	int cum = 0;
	while(i<50){
		int x = readVoltagemV(temp_instance);
		cum += x;
		printf("%d\n\r",x);
		i++;
	}
	printf("AVG: %d\n\r", cum/i);*/
    //printf("Temperature in F: %f\r\n", MLX90614_read_temperature());
	
	
	// motor controller test that says bad address
	/*uint8_t write_buffer[3] = {
		0xaa, 0x0a, 0x00
	};
	
	struct i2c_master_packet write_packet = {
		.address     = 0x0f,
		.data_length = 3,
		.data        = write_buffer,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};
	i2c_write_command(&write_packet);
	
	// Create task to monitor processor activity
	if (xTaskCreate(task_monitor, "Monitor", TASK_MONITOR_STACK_SIZE, NULL,
			TASK_MONITOR_STACK_PRIORITY, NULL) != pdPASS) {
		//printf("Failed to create Monitor task\r\n");
	}

	// Create task to make led blink
	if (xTaskCreate(task_led, "Led", TASK_LED_STACK_SIZE, NULL,
			TASK_LED_STACK_PRIORITY, NULL) != pdPASS) {
		//printf("Failed to create test led task\r\n");
	}
	
	// Create task to make led blink
	if (xTaskCreate(task_adc_read, "ADC", TASK_ADC_STACK_SIZE, NULL,
	TASK_ADC_STACK_PRIORITY, NULL) != pdPASS) {
		//printf("Failed to create test led task\r\n");
	}

	// Start the scheduler.
	vTaskStartScheduler();

	// Will only get here if there was insufficient memory to create the idle task.
	*/
	return 0;
}
