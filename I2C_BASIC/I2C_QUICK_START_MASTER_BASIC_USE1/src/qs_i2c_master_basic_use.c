/**
 * \file
 *
 * \brief SAM SERCOM I2C Master Quick Start Guide
 *
 * Copyright (C) 2012-2014 Atmel Corporation. All rights reserved.
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
#include <conf_uart_serial.h>

#define HMC5883L_ADDRESS 0x1E
#define TIMEOUT 1000


static struct usart_module cdc_uart_module;
struct i2c_master_module i2c_master_instance;
static void configure_console(void);
void configure_i2c_master(void);
void i2c_write_command(struct i2c_master_packet* packet_address);
void i2c_read_command(struct i2c_master_packet* packet_address);
void HMC5883L_init(void);
void HMC5883L_read(uint8_t* read_buffer);

int main(void){
	//initialize SAMD21
	system_init();
	
	//configure PUTTY terminal
	configure_console();
	printf("Running...\n\r");
	
	//configure i2c master on SAMD21
	configure_i2c_master();
	printf("I2C Configured\n\r");
	
	//initialize HMC5883L magnetometer with standard settings
	HMC5883L_init();
	printf("HMC5883L (magnetometer) initialized\n\r");	
	
	//Establish buffer to read from
	static uint8_t HMC5883L_read_buffer[7] = {
		'a','a','a','a','a','a',0x0
	};
	
	//take 10 measurements
	int i = 0;
	while(i < 10){
		HMC5883L_read(HMC5883L_read_buffer);
		printf("measurement\r\n");
		printf("%x%x%x%x%x%x\r\n",(char)HMC5883L_read_buffer[0],(char)HMC5883L_read_buffer[0],(char)HMC5883L_read_buffer[0],(char)HMC5883L_read_buffer[0],(char)HMC5883L_read_buffer[0],(char)HMC5883L_read_buffer[0]);
		i++;
	}

}

/*
	Configure UART console. (Terminal accessable through PUTTY)
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

	stdio_serial_init(&cdc_uart_module, CONF_STDIO_USART_MODULE,    
&usart_conf);
	usart_enable(&cdc_uart_module);
}

/*
	Configures I2C connection with standard settings
*/
void configure_i2c_master(void)
{
	/* Initialize config structure and software module. */
	//! [init_conf]
	struct i2c_master_config config_i2c_master;
	i2c_master_get_config_defaults(&config_i2c_master);
	//! [init_conf]

	/* Change buffer timeout to something longer. */
	//! [conf_change]
	config_i2c_master.buffer_timeout = 10000;
	//! [conf_change]

	/* Initialize and enable device with config. */
	//! [init_module]
	i2c_master_init(&i2c_master_instance, SERCOM2, &config_i2c_master);
	//! [init_module]

	//! [enable_module]
	i2c_master_enable(&i2c_master_instance);
	//! [enable_module]
}

/*
	Given a pointer to a packet, perform a write over I2C following the information
	detailed in the packet
*/
void i2c_write_command(struct i2c_master_packet* packet_address){
	uint16_t timeout = 0;
	while (i2c_master_write_packet_wait(&i2c_master_instance, packet_address) !=
	STATUS_OK) {
		/* Increment timeout counter and check if timed out. */
		if (timeout++ == TIMEOUT) {
			break;
		}
	}
}

/*
	Given a pointer to a packet, perform a read over I2C following the information
	detailed in the packet
*/
void i2c_read_command(struct i2c_master_packet* packet_address){
	uint16_t timeout = 0;
	while ((i2c_master_read_packet_wait(&i2c_master_instance, packet_address)) !=
	STATUS_OK) {
		if (timeout++ == TIMEOUT) {
			break;
			printf("timeout");
		}
	}
}

/*
	Perform a standard initialization of the HMC5883L Magnetometer
	CURRENT SETTINGS:
	15HZ default measurement rate
	Gain = 5
	Single Measurement Mode
*/
void HMC5883L_init(void){
	static uint8_t write_buffer_1[2] = {
		0x00, 0x70
	};

	static uint8_t write_buffer_2[2] = {
		0x01, 0xA0
	};
	
	struct i2c_master_packet write_packet_1 = {
		.address     = HMC5883L_ADDRESS,
		.data_length = 2,
		.data        = write_buffer_1,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};
	struct i2c_master_packet write_packet_2 = {
		.address     = HMC5883L_ADDRESS,
		.data_length = 2,
		.data        = write_buffer_2,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};
	
	i2c_write_command(&write_packet_1);
	i2c_write_command(&write_packet_2);
}

/*
	Given an input buffer to read into (6 BYTES IN LENGTH MINIMUM) read from the x,y,z LSB and MSB registers
	on the HMC5883L magnetometer in single measurement mode
*/
void HMC5883L_read(uint8_t* read_buffer){
	struct i2c_master_packet read_packet = {
		.address     = HMC5883L_ADDRESS,
		.data_length = 6,
		.data        = read_buffer,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};
	
	static uint8_t write_buffer[2] = {
		0x02, 0x01
	};
	
	struct i2c_master_packet write_packet = {
		.address     = HMC5883L_ADDRESS,
		.data_length = 2,
		.data        = write_buffer,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};
	
	i2c_write_command(&write_packet);
	i2c_read_command(&read_packet);
}