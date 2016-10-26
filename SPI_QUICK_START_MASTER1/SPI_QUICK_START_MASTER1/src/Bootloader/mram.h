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

//#define SLAVE_SELECT_PIN EXT1_PIN_SPI_SS_0

#define NUM_CONTROL_BYTES 0x04
#define READ_COMMAND 0x02
#define WRITE_COMMAND 0x03
#define ENABLE_COMMAND 0x06

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
uint8_t read_bytes(struct spi_module *spi_master_instance, struct spi_slave_inst *slave, uint8_t *data, int num_bytes, uint16_t address);

/************************************************************************/
/* Given master, slave number of bytes and initial address, the content */
/* of data will be written                                              */
/************************************************************************/
void write_bytes(struct spi_module *spi_master_instance, struct spi_slave_inst *slave, uint8_t *data, int num_bytes, uint16_t address);

int test_mram(void);

#endif /* MRAM_H_ */