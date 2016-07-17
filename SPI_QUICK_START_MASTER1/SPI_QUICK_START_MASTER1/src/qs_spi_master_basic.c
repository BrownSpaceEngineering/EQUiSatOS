/**

For a proper connection it is necessary to EXT2_SPI_SERCOM_PINMUX_PAD# that directly map the column SERCOM-ALT
in the page 14 of the SAM D-21 data sheet. If they do not match,  it is possible to send data to the mram,
but not receive from it. For example:

EXT2_SPI_MODULE
EXT2_SPI_SERCOM_MUX_SETTING
EXT2_SPI_SERCOM_PINMUX_PAD0
EXT2_SPI_SERCOM_PINMUX_PAD1
EXT2_SPI_SERCOM_PINMUX_PAD2
EXT2_SPI_SERCOM_PINMUX_PAD3

They are setting for SERCOM 3 in the data sheet, but samd21_xplained_pro.h offers more precise information.

**/
#include <asf.h>
#include <clock.h>
#include <math.h>

#include <stdbool.h>
#include <samd21_xplained_pro.h>
#include <spi.h>

#include "Bootloader/flash_memory.h"
#define READ_LEN 5

struct spi_module spi_master_instance;
struct spi_slave_inst slave;
void configure_spi_master(void);

//! [configure_spi]
void configure_spi_master(void)
{
	struct spi_config config_spi_master;
	struct spi_slave_inst_config slave_dev_config;
	spi_slave_inst_get_config_defaults(&slave_dev_config);
	slave_dev_config.ss_pin = SLAVE_SELECT_PIN;
	spi_attach_slave(&slave, &slave_dev_config);
	spi_get_config_defaults(&config_spi_master);
	config_spi_master.mux_setting = EXT2_SPI_SERCOM_MUX_SETTING;
	config_spi_master.pinmux_pad0 = /*PIN_PA16;*/EXT2_SPI_SERCOM_PINMUX_PAD1;
//! [di]
	/* Configure pad 1 as unused */
//! [ss]
	config_spi_master.pinmux_pad1 = EXT2_SPI_SERCOM_PINMUX_PAD0;
//! [ss]
	/* Configure pad 2 for data out */
//! [do]
	config_spi_master.pinmux_pad2 = /*PIN_PB16;*/EXT2_SPI_SERCOM_PINMUX_PAD2;
//! [do]
	/* Configure pad 3 for SCK */
//! [sck]
	config_spi_master.pinmux_pad3 = /*PIN_PB17;*/EXT2_SPI_SERCOM_PINMUX_PAD3;
//! [sck]
//! [init]
	spi_init(&spi_master_instance, EXT2_SPI_MODULE, &config_spi_master);
//! [init]

//! [enable]
	enum status_code code = spi_set_baudrate(&spi_master_instance, 10000000);

	spi_enable(&spi_master_instance);
//! [enable]

}
/** Basic main function to test SPI, this function should  write a hex in
the address 0x00 and then read it into temp2. Currently the code is   
writing 0x15. The program directly reflect the steps defined in the   
MRAM data sheet.This function is just to test the connection with the
MRAM.                                                                
**/
int main(void)
{
	system_init();

	configure_spi_master();
	
	static uint8_t enable = 0x06;
	static uint8_t status[5] = {0x03, 0x00, 0x00, 0x00, 0x08};
	
	save_binary_into_flash(status, 6, 512);
	
	spi_select_slave(&spi_master_instance, &slave, false);
	spi_select_slave(&spi_master_instance, &slave, true);
	enum status_code code_1 = spi_write_buffer_wait(&spi_master_instance, &enable, 1);
	spi_select_slave(&spi_master_instance, &slave, false);
	
	uint8_t temp[5] = {0x01, 0x01, 0x01, 0x01, 0x01};
	uint8_t temp2[5] = {0x01, 0x01, 0x01, 0x01, 0x01};
	static uint8_t write_8[5] = {0x02, 0x00, 0x00, 0x00, 0x15};
	spi_select_slave(&spi_master_instance, &slave, true);
	enum status_code code_2 = spi_transceive_buffer_wait(&spi_master_instance, write_8, &temp, 5);
	spi_select_slave(&spi_master_instance, &slave, false);	
	
	while (true) {
		spi_select_slave(&spi_master_instance, &slave, true);
		code_2 = spi_transceive_buffer_wait(&spi_master_instance, status, &temp2, 5);
		spi_select_slave(&spi_master_instance, &slave, false);
	}
}
