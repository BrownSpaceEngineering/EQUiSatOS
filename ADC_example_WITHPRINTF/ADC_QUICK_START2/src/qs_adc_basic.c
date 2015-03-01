/**
 * \file
 *
 * \brief SAM ADC Quick Start
 *
 * Copyright (C) 2013-2014 Atmel Corporation. All rights reserved.
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
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <asf.h>
#include "conf_uart_serial.h"
#include <inttypes.h>
#include <delay.h>

/* Function Declarations */
float readVoltage(struct adc_module adc_instance);
float resultToVoltage(float result);
float voltageToTemp(float voltage);


static struct usart_module cdc_uart_module;

/**
 *  Configure UART console.
 */
static void configure_console(void) {
	struct usart_config usart_conf;

	usart_get_config_defaults(&usart_conf);
	usart_conf.mux_setting = CONF_STDIO_MUX_SETTING;
	usart_conf.pinmux_pad0 = CONF_STDIO_PINMUX_PAD0;
	usart_conf.pinmux_pad1 = CONF_STDIO_PINMUX_PAD1;
	usart_conf.pinmux_pad2 = CONF_STDIO_PINMUX_PAD2;
	usart_conf.pinmux_pad3 = CONF_STDIO_PINMUX_PAD3;
	usart_conf.baudrate    = CONF_STDIO_BAUDRATE;

	stdio_serial_init(&cdc_uart_module, CONF_STDIO_USART_MODULE, &usart_conf);
	usart_enable(&cdc_uart_module);
}

void configure_adc(void);

//init adc_instant
struct adc_module adc_instance;

//! [setup]
void configure_adc(void) {
	struct adc_config config_adc;
	// setup_config_defaults
	adc_get_config_defaults(&config_adc);

	config_adc.gain_factor = ADC_GAIN_FACTOR_DIV2;
	config_adc.clock_prescaler = ADC_CLOCK_PRESCALER_DIV4;
	config_adc.reference = ADC_REFERENCE_INT1V;
	config_adc.positive_input = ADC_POSITIVE_INPUT_PIN8; //PB00
	config_adc.resolution = ADC_RESOLUTION_12BIT;

	//setup_set_config
	adc_init(&adc_instance, ADC, &config_adc);
	adc_enable(&adc_instance);
}

float readVoltage(struct adc_module adc_instance) {
	if (!&adc_instance || !adc_instance.hw) {
		//You must configure the adc_instance and set it as a global variable.
		return -1;
	}
	
	uint16_t result = 0;
	int status;
	
	//start conversion
	adc_start_conversion(&adc_instance);
	
	do {
		// Wait for conversion to be done and read out result
		status = adc_read(&adc_instance, &result);
	} while (status == STATUS_BUSY);
	float resFloat = result;
	return resultToVoltage(resFloat);
}
	
float resultToVoltage(float result) {
	return 5.599744685 + 0.4909903275 * result;
}

float voltageToTemp(float voltage) {
	return 0.1*voltage - 50;
}

int main(void) {
	system_init();
	configure_console();
	configure_adc();
	
	for(int i = 0; i < 100; i++) {
		printf("%f ", readVoltage(adc_instance));
	}
}

