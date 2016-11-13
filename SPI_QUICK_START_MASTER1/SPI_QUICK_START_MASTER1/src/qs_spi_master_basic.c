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

#include "Bootloader/mram.h"
#include "Bootloader/flash_memory.h"
#define READ_LEN 5

struct spi_module spi_master_instance;
struct spi_slave_inst slave;

/** Basic main function to test SPI, this function should  write a hex in
the address 0x00 and then read it into temp2. Currently the code is   
writing 0x15. The program directly reflect the steps defined in the   
MRAM data sheet.This function is just to test the connection with the
MRAM.                                                                
**/
int test_mram(void)
{
	system_init();

	initialize_master(&spi_master_instance, 10000000); // seems to be the more "modern" implementation in mram.c
	initialize_slave(&slave);
	
	static uint8_t enable = 0x06;
	static uint8_t status[5] = {0x03, 0x00, 0x00, 0x00, 0x08};
	
	save_binary_into_flash(status, 6, 512); // this saves the array into flash...?
	
	spi_select_slave(&spi_master_instance, &slave, false);
	spi_select_slave(&spi_master_instance, &slave, true);
	enum status_code code_1 = spi_write_buffer_wait(&spi_master_instance, &enable, 1); // what does writing the raw buffer mean?
	spi_select_slave(&spi_master_instance, &slave, false);
	
	uint8_t temp[5] = {0x01, 0x01, 0x01, 0x01, 0x01};
	uint8_t temp2[5] = {0x01, 0x01, 0x01, 0x01, 0x01};
	static uint8_t write_8[5] = {0x02, 0x00, 0x00, 0x00, 0x15};
	spi_select_slave(&spi_master_instance, &slave, true);
	enum status_code code_2 = spi_transceive_buffer_wait(&spi_master_instance, write_8, &temp, 5); // why &temp?
	spi_select_slave(&spi_master_instance, &slave, false);	
	
	while (true) {
		spi_select_slave(&spi_master_instance, &slave, true);
		code_2 = spi_transceive_buffer_wait(&spi_master_instance, status, &temp2, 5); // why &temp2 also?
		spi_select_slave(&spi_master_instance, &slave, false);
	}
	return 0;
}
