/*
 * SPI_Commands.h
 *
 * Created: 5/8/2015 3:45:43 PM
 *  Author: Daniel
 */ 


#ifndef SPI_COMMANDS_H
#define SPI_COMMANDS_H

#include <spi.h>
#include <board.h>

struct spi_module spi_master_instance;
struct spi_slave_inst slave;

void configure_spi_master(void);

#define SLAVE_SELECT_PIN EXT1_PIN_SPI_SS_0

#endif /* SPI_COMMANDS_H */