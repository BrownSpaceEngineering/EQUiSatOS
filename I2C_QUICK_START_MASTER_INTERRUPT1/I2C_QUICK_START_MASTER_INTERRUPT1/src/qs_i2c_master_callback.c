/**
 * \file
 *
 * \brief SAM SERCOM I2C Master Quick Start Guide with Callbacks
 *
 * Copyright (C) 2012-2014 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
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
#include <system_interrupt.h>

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

    stdio_serial_init(&cdc_uart_module, CONF_STDIO_USART_MODULE,    
&usart_conf);
    usart_enable(&cdc_uart_module);
}

void i2c_read_complete_callback(
		struct i2c_master_module *const module);
void configure_i2c(void);
void configure_i2c_callbacks(void);

//! [packet_data]
#define DATA_LENGTH 6

#define WRITE_DATA_LENGTH 3

#define TIMEOUT 1000

static uint8_t cra_wr_buffer[WRITE_DATA_LENGTH] = {
	0x3C, 0x00, 0x70
};
static uint8_t crb_wr_buffer[WRITE_DATA_LENGTH] = {
	0x3C, 0x01, 0xA0
};
static uint8_t measure_mode_wr_buffer[WRITE_DATA_LENGTH] = {
	0x3C, 0x02, 0x01
};

static uint8_t rd_buffer[DATA_LENGTH];
//! [packet_data]

//! [address]
#define SLAVE_WRITE_ADDRESS 0x3C
#define SLAVE_READ_ADDRESS 0x3D
//! [address]

//! [packet_glob]
struct i2c_master_packet cra_wr_packet, crb_wr_packet, measure_mode_wr_packet;
struct i2c_master_packet rd_packet;
//! [packet_glob]

/* Init software module instance. */
//! [dev_inst]
struct i2c_master_module i2c_master_instance;
//! [dev_inst]

//! [callback_func]
void i2c_read_complete_callback(
		struct i2c_master_module *const module)
{
	printf("Read: ");
	for(int i=0; i<8; i++) {
		printf(rd_buffer[i]);
		printf(" ");
	}
	printf("\n");
}
//! [callback_func]

//! [initialize_i2c]
void configure_i2c(void)
{
	/* Initialize config structure and software module */
	//! [init_conf]
	struct i2c_master_config config_i2c_master;
	i2c_master_get_config_defaults(&config_i2c_master);
	//! [init_conf]

	/* Change buffer timeout to something longer */
	//! [conf_change]
	config_i2c_master.buffer_timeout = 65535;
	//! [conf_change]

	/* Initialize and enable device with config */
	//! [init_module]
	while(i2c_master_init(&i2c_master_instance, SERCOM2, &config_i2c_master)
			!= STATUS_OK);
	//! [init_module]

	//! [enable_module]
	i2c_master_enable(&i2c_master_instance);
	//! [enable_module]
}
//! [initialize_i2c]

//! [setup_callback]
void configure_i2c_callbacks(void)
{
	/* Register callback function. */
	//! [callback_reg]
	/*i2c_master_register_callback(&i2c_master_instance, i2c_read_complete_callback,
			I2C_MASTER_CALLBACK_READ_COMPLETE);*/
	//! [callback_reg]
	//! [callback_en]
	/*i2c_master_enable_callback(&i2c_master_instance,
			I2C_MASTER_CALLBACK_READ_COMPLETE);*/
	//! [callback_en]
}
//! [setup_callback]

int main(void)
{
	system_init();

	//! [run_initialize_i2c]
	/* Configure device and enable. */
	//! [config]
	configure_console();
	printf("Running...\n\r");
	configure_i2c();
	//! [config]
	/* Configure callbacks and enable. */
	//! [config_callback]
	//configure_i2c_callbacks();
	//! [config_callback]
	//! [run_initialize_i2c]

	/* Init i2c packet. */
	//! [write_packet]
	cra_wr_packet.address     = SLAVE_WRITE_ADDRESS;
	cra_wr_packet.data_length = WRITE_DATA_LENGTH;
	cra_wr_packet.data        = cra_wr_buffer;
	crb_wr_packet.address     = SLAVE_WRITE_ADDRESS;
	crb_wr_packet.data_length = WRITE_DATA_LENGTH;
	crb_wr_packet.data        = crb_wr_buffer;
	measure_mode_wr_packet.address     = SLAVE_WRITE_ADDRESS;
	measure_mode_wr_packet.data_length = WRITE_DATA_LENGTH;
	measure_mode_wr_packet.data        = measure_mode_wr_buffer;
	
	
	//! [write_packet]
	//! [read_packet]
	rd_packet.address     = SLAVE_READ_ADDRESS;
	rd_packet.data_length = DATA_LENGTH;
	rd_packet.data        = rd_buffer;
	//! [read_packet]

	//! [while]
	int timeout = 1000;
	while (true) {
		/* Infinite loop */
		/*if (!port_pin_get_input_level(BUTTON_0_PIN)) {
			// Send every other packet with reversed data
			//! [revert_order]
			if (wr_packet.data[0] == 0x00) {
				wr_packet.data = &wr_buffer_reversed[0];
			} else {
				wr_packet.data = &wr_buffer[0];
			}
			//! [revert_order]
			//! [write_packet]
			i2c_master_write_packet_job(&i2c_master_instance, &wr_packet);
			//! [write_packet]
		}*/
		//printf("About to read...\n\n");
		printf("Writing CRA...\r\n");
		int status = i2c_master_write_packet_wait(&i2c_master_instance, &cra_wr_packet);
		printf("CRA write status: %d\r\r", status);
		printf("Writing CRB...\r\n");
		status = i2c_master_write_packet_wait(&i2c_master_instance, &crb_wr_packet);
		printf("CRB write status: %d\r\r", status);
		printf("Writing measure mode...\r\n");
		status = i2c_master_write_packet_wait(&i2c_master_instance, &measure_mode_wr_packet);
		printf("Measure mode write status: %d\r\r", status);
		//usleep(6000);
		int a=0;
		for(int i=0; i<60000; i++) {
			a++;
		}
		
		printf("Reading...\r\n");
		status = i2c_master_read_packet_wait(&i2c_master_instance, &rd_packet);
		printf("read status: %d\n\r", status);
		if(status != STATUS_OK) {
			if(timeout++ == TIMEOUT) {
				break;
			}
		}
		printf("Data: %d %d %d %d %d %d\r\n", rd_buffer[0], rd_buffer[1], rd_buffer[2], rd_buffer[3], rd_buffer[4], rd_buffer[5]);
	}

	//! [while]
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
		}
	}
}
