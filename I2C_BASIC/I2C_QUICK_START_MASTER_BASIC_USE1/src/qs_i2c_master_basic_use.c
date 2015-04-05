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

static struct usart_module cdc_uart_module;
void write_command(struct i2c_master_packet* packet_address);
void read_command(struct i2c_master_packet* packet_address);
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

    stdio_serial_init(&cdc_uart_module, CONF_STDIO_USART_MODULE,    
&usart_conf);
    usart_enable(&cdc_uart_module);
}


//! [packet_data]
static uint8_t write_buffer_1[2] = {
	 0x00, 0x70
};

static uint8_t write_buffer_2[2] = {
	 0x01, 0xA0
};

static uint8_t write_buffer_3[2] = {
	 0x02, 0x01
};

static uint8_t read_buffer[7] = {
		'a','a','a','a','a','a',0x0
};
//! [packet_data]

//! [address]
#define READ_ADDRESS 0x1E
#define WRITE_ADDRESS 0x1E
//! [address]

/* Number of times to try to send packet if failed. */
//! [timeout]
#define TIMEOUT 1000
//! [timeout]

/* Init software module. */
//! [dev_inst]
struct i2c_master_module i2c_master_instance;
//! [dev_inst]

void configure_i2c_master(void);

//! [initialize_i2c]
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
//! [initialize_i2c]

int main(void)
{
	system_init();
	
	configure_console();
	//console configuration

	//! [init]
	/* Configure device and enable. */
	//! [config]
	printf("Running...\n\r");
	configure_i2c_master();
	//! [config]

	/* Timeout counter. */
	//! [timeout_counter]

	//! [timeout_counter]

	/* Init i2c packet. */
	//! [packet]
	struct i2c_master_packet read_packet = {
		.address     = READ_ADDRESS,
		.data_length = 6,
		.data        = read_buffer,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};
	
	struct i2c_master_packet write_packet_1 = {
		.address     = WRITE_ADDRESS,
		.data_length = 2,
		.data        = write_buffer_1,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};
	struct i2c_master_packet write_packet_2 = {
		.address     = WRITE_ADDRESS,
		.data_length = 2,
		.data        = write_buffer_2,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};
	struct i2c_master_packet write_packet_3 = {
		.address     = WRITE_ADDRESS,
		.data_length = 2,
		.data        = write_buffer_3,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};
	//! [packet]
	//! [init]
	//! [main]
	/* Write buffer to slave until success. */
	//! [write_packet]
	//setup
	write_command(&write_packet_1);
	write_command(&write_packet_2);
	
	//measurement
	while(true){
	write_command(&write_packet_3);
	read_command(&read_packet);
	
	//! [write_packet]

	/* Read from slave until success. */
	//! [read_packet]
	printf("measurement\r\n");
	printf("%x%x%x%x%x%x\r\n",(char)read_buffer[0],(char)read_buffer[0],(char)read_buffer[0],(char)read_buffer[0],(char)read_buffer[0],(char)read_buffer[0]);
	//! [main]
	}
	while (true) {
		/* Infinite loop */
	}

}

void write_command(struct i2c_master_packet* packet_address){
	uint16_t timeout = 0;
	while (i2c_master_write_packet_wait(&i2c_master_instance, packet_address) !=
	STATUS_OK) {
		/* Increment timeout counter and check if timed out. */
		if (timeout++ == TIMEOUT) {
			break;
		}
	}
}

void read_command(struct i2c_master_packet* packet_address){
	uint16_t timeout = 0;
	while ((i2c_master_read_packet_wait(&i2c_master_instance, packet_address)) !=
	STATUS_OK) {
		if (timeout++ == TIMEOUT) {
			break;
			printf("timeout");
		}
	}
}