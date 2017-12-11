/*
 * spi.h
 *
 * Created: 4/10/2016 1:59:16 PM
 *  Author: Gustavo
 */ 


#ifndef MRAM_H_
#define MRAM_H_

#include <stdbool.h>
#include <spi.h>

#define SLAVE_SELECT_PIN	PIN_PA17C_SERCOM1_PAD1 // PIN_PB16C_SERCOM5_PAD0 (probably not it by eagle)
#define MRAM_SPI_MOSI		PINMUX_PA16C_SERCOM1_PAD0 // corresponds to signal SO for Serial Out (from master)
#define MRAM_SPI_MISO		PINMUX_PA18C_SERCOM1_PAD2 // corresponds to signal SI for Serial In (into master)
#define MRAM_SPI_SCK		PINMUX_PA19C_SERCOM1_PAD3

extern const uint8_t NUM_CONTROL_BYTES, READ_COMMAND, WRITE_COMMAND, ENABLE_COMMAND;

/************************************************************************/
/* Initialize the master, the baudrate should be inside the proper range*/
/************************************************************************/
uint8_t initialize_master(struct spi_module *spi_master_instance, uint32_t baudrate);

/************************************************************************/
/* Initialize the slave, just pass the pointer                          */
/************************************************************************/
uint8_t initialize_slave(struct spi_slave_inst *slave);

/************************************************************************/
/* Given master, slave number of bytes and initial address, the content */ 
/* will be read to data                                                 */
/************************************************************************/
status_code_genare_t read_bytes(struct spi_module *spi_master_instance, struct spi_slave_inst *slave, uint8_t *data, int num_bytes, uint16_t address);

/************************************************************************/
/* Given master, slave number of bytes and initial address, the content */
/* of data will be written                                              */
/************************************************************************/
status_code_genare_t write_bytes(struct spi_module *spi_master_instance, struct spi_slave_inst *slave, uint8_t *data, int num_bytes, uint16_t address);

/************************************************************************/
/* Given master and slave, the content of the MRAM's status register	*/
/* will be written				                                        */
/************************************************************************/
status_code_genare_t read_status_register(struct spi_module *spi_master_instance, struct spi_slave_inst *slave, uint8_t *reg_out);

int test_mram(void);

#endif /* MRAM_H_ */