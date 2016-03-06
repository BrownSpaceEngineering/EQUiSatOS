/**
 * \file
 *
 * \brief SAM SPI Quick Start
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
#include <clock.h>
#include <math.h>
//! [setup]
//! [buf_length]
//! [buf_length]
//! [slave_select_pin]
#define SLAVE_SELECT_PIN EXT1_PIN_SPI_SS_0
//! [slave_select_pin]
//! [buffer]


/*SEE DATASHEET FOR SPI PART*/

#define READ_LEN 5
static uint8_t read0[READ_LEN] = {
		0x03, 0x00, 0x00, 0x00, 0x00
};

#define WRITE_LEN 4
static uint8_t write0[WRITE_LEN] = {
	0x02, 0x00, 0x01, 0x00
};

#define READ_MO_LEN 5
static uint8_t readModeReg[READ_MO_LEN] = {
	0x05, 0x00, 0x00, 0x00, 0x00
};
#define WRITE_MO_LEN 2
static uint8_t writeModeReg[WRITE_MO_LEN] = {
	0x01, 0x00
};

static uint8_t rx[16] = {
	   0xef, 0xef, 0xef, 0xef, 0xef, 0xff, 0xff, 
	   0xff, 0xff, 0xff//, 0xff, 0xff, 0xff, 0xff, 0xff
};
//! [buffer]

//! [dev_inst]
struct spi_module spi_master_instance;
//! [dev_inst]
//! [slave_dev_inst]
struct spi_slave_inst slave;
//! [slave_dev_inst]
//! [setup]

void configure_spi_master(void);

//! [configure_spi]
void configure_spi_master(void)
{
//! [config]
	struct spi_config config_spi_master;
//! [config]
//! [slave_config]
	struct spi_slave_inst_config slave_dev_config;
//! [slave_config]
	/* Configure and initialize software device instance of peripheral slave */
//! [slave_conf_defaults]
	spi_slave_inst_get_config_defaults(&slave_dev_config);
//! [slave_conf_defaults]
//! [ss_pin]
	slave_dev_config.ss_pin = SLAVE_SELECT_PIN;
	//slave_dev_config.ss_pin = PIN_PA20;
//! [ss_pin]
//! [slave_init]
	spi_attach_slave(&slave, &slave_dev_config);
//! [slave_init]
	/* Configure, initialize and enable SERCOM SPI module */
//! [conf_defaults]
	spi_get_config_defaults(&config_spi_master);

//! [conf_defaults]
//! [mux_setting]
	config_spi_master.mux_setting = EXT1_SPI_SERCOM_MUX_SETTING;
//! [mux_setting]
	/* Configure pad 0 for data in */
//! [di]
	config_spi_master.pinmux_pad0 = /*PIN_PA21;*/EXT1_SPI_SERCOM_PINMUX_PAD0;
//! [di]
	/* Configure pad 1 as unused */
//! [ss]
	config_spi_master.pinmux_pad1 = PINMUX_UNUSED;
//! [ss]
	/* Configure pad 2 for data out */
//! [do]
	config_spi_master.pinmux_pad2 = /*PIN_PB16;*/EXT1_SPI_SERCOM_PINMUX_PAD2;
//! [do]
	/* Configure pad 3 for SCK */
//! [sck]
	config_spi_master.pinmux_pad3 = /*PIN_PB17;*/EXT1_SPI_SERCOM_PINMUX_PAD3;
//! [sck]
//! [init]
	spi_init(&spi_master_instance, EXT1_SPI_MODULE, &config_spi_master);
//! [init]

//! [enable]
	enum status_code code = spi_set_baudrate(&spi_master_instance, 1000000);

	spi_enable(&spi_master_instance);
//! [enable]

}
//! [configure_spi]

int main(void)
{
//! [main_setup]
//! [system_init]
	system_init();
	/*struct port_config conf;
	port_get_config_defaults(&conf);
	conf.direction = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(PIN_PA18, &conf);
	port_pin_set_config(PIN_PA19, &conf);
	port_pin_set_output_level(PIN_PA18, true);
	port_pin_set_output_level(PIN_PA19, true);*/
	
//! [system_init]
//! [run_config]
	configure_spi_master();
//! [run_config]
//! [main_setup]

//! [main_use_case]
//! [select_slave]
	//spi_select_slave(&spi_master_instance, &slave, true);
//! [select_slave]
//! [write]
    //enum status_code code_0 = spi_write_buffer_wait(&spi_master_instance, write0, WRITE_LEN);
	//enum status_code code_1 = spi_transceive_buffer_wait(&spi_master_instance,read0,rx, READ_LEN);
	static uint8_t write1[5] = {
	0x02, 0x00, 0x00, 0x00, 0x01
	};
	
	static uint8_t read1[4] = {
		0x03, 0x00, 0x00, 0x00
	};
	static uint8_t enable = 0x06;
	static uint8_t status[6] = {0x03, 0x00, 0x00, 0x00, 0x01, 0x06, 0x06, 0x06, 0x06, 0x06};
	spi_select_slave(&spi_master_instance, &slave, false);
	spi_select_slave(&spi_master_instance, &slave, true);
	enum status_code code_1 = spi_write_buffer_wait(&spi_master_instance, &enable, 1);
	spi_select_slave(&spi_master_instance, &slave, false);
	
	uint8_t temp[5];
	static uint8_t write_8[6] = {0x02, 0x00, 0x00, 0x00, 0x10};
	spi_select_slave(&spi_master_instance, &slave, true);
	enum status_code code_2 = spi_transceive_buffer_wait(&spi_master_instance, write_8, &temp, 5);
	spi_select_slave(&spi_master_instance, &slave, false);	
	
	
//! [inf_loop]
	delay_init();
	while (true) {
		spi_select_slave(&spi_master_instance, &slave, true);
		//status[0] = (uint8_t)rand();
		enum status_code code_2 = spi_transceive_buffer_wait(&spi_master_instance, status, &temp, 5);
		//delay_us(100);
		spi_select_slave(&spi_master_instance, &slave, false);
		//delay_us(100);
		int a = 9 ;
	}
//! [inf_loop]
//! [main_use_case]
}
