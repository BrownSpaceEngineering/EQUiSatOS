/*
 * MRAM_Commands.h
 *
 * Created: 4/10/2016 1:59:16 PM
 *  Author: Gustavo
 */ 

#ifndef MRAM_COMMANDS_H_
#define MRAM_COMMANDS_H_

#include <stdbool.h>
#include <spi.h>
#include <assert.h>

// NOTE: looking at initialize_master in mram.c, the PADX must correspond to the PADX on these
// signals, but the mux setting (a letter) DOES NOT have to correspond with the function of
// these pin's muxes
#define P_SPI_MOSI			PINMUX_PA18C_SERCOM1_PAD2 // corresponds to signal SI
#define P_SPI_MISO			PINMUX_PA16C_SERCOM1_PAD0 // corresponds to signal SO
#define P_SPI_SCK			PINMUX_PA19C_SERCOM1_PAD3
#define MRAM_SPI_SERCOM		SERCOM1
#define MRAM_SPI_BAUD		10000000

#define P_MRAM1_CS			PIN_PA17
#define P_MRAM2_CS			PIN_PB16

#define NUM_CONTROL_BYTES			0x04
#define READ_COMMAND				0x03
#define READ_STATUS_REG_COMMAND		0x05
#define WRITE_COMMAND				0x02
#define WRITE_STATUS_REG_COMMAND	0x01
#define ENABLE_COMMAND				0x06

#define STATUS_REG_PROTECT_ALL		(0b11 << 2)
#define STATUS_REG_PROTECT_TOP_HALF	(0b10 << 2)
#define STATUS_REG_PROTECT_TOP_QUAD (0b01 << 2)
#define STATUS_REG_PROTECT_NONE		(0b00 << 2)

/************************************************************************/
/* Initialize the master, the baudrate should be inside the proper range*/
/************************************************************************/
status_code_genare_t mram_initialize_master(struct spi_module *spi_master_instance, uint32_t baudrate);

/************************************************************************/
/* Initialize the slave, just pass the pointer                          */
/************************************************************************/
void mram_initialize_slave(struct spi_slave_inst *slave, int ss_pin);

/************************************************************************/
/* Reset the MRAM SPI interface and its slaves.							*/
/* Used in bootloader to reset SPI module before booting to the OS      */
/************************************************************************/
void mram_reset(struct spi_module *spi_master_instance);

/************************************************************************/
/* Given master, slave number of bytes and initial address, the content */
/* will be read to data (note only address bits 0-18 (inclusive!) are used*/
/************************************************************************/
status_code_genare_t mram_read_bytes(struct spi_module *spi_master_instance, struct spi_slave_inst *slave, uint8_t *data, int num_bytes, uint32_t address);

/************************************************************************/
/* Given master, slave number of bytes and initial address, the content */
/* of data will be written (note only address bits 0-18 (inclusive!) are used*/
/************************************************************************/
status_code_genare_t mram_write_bytes(struct spi_module *spi_master_instance, struct spi_slave_inst *slave, uint8_t *data, int num_bytes, uint32_t address);

/************************************************************************/
/* Given master and slave, the content of the MRAM's status register	*/
/* will be read				                                        */
/************************************************************************/
status_code_genare_t mram_read_status_register(struct spi_module *spi_master_instance, struct spi_slave_inst *slave, uint8_t *reg_out);

/************************************************************************/
/* Given master and slave, the content of the MRAM's status register	*/
/* will be written				                                        */
/************************************************************************/
status_code_genare_t mram_write_status_register(struct spi_module *spi_master_instance, struct spi_slave_inst *slave, uint8_t register_vals);

/************************************************************************/
/* Rad-safe field helpers                                               */
// Usage:
// /* defines three redundant variables; 
//    type can be anything assignable by = and comparable by == */
// RAD_SAFE_FIELD_DEFINE(uint8_t, cats);
// RAD_SAFE_FIELD_INIT(uint8_t, cats, 55);
// /* careful if this value is a function call! It will be called thrice */
// RAD_SAFE_FIELD_SET(cats, 55); 
// /* Top-voted field chosen; errors will be corrected after read */
// uint8_t cats_res = RAD_SAFE_FIELD_GET(cats)
// foo(RAD_SAFE_FIELD_GET(cats))
/************************************************************************/
#define RAD_SAFE_FIELD_DEFINE(type, field_name)		type field_name[3]
#define RAD_SAFE_FIELD_INIT(type, field_name, val)	type field_name[3] = {val, val, val}
#define RAD_SAFE_FIELD_SET(field_name, val)			field_name[0] = val; field_name[1] = val; field_name[2] = val
#define RAD_SAFE_FIELD_CORRECT(field_name)			if (field_name[0] == field_name[1] && field_name[2] != field_name[0]) { field_name[2] = field_name[0]; }; \
 													if (field_name[0] == field_name[2] && field_name[1] != field_name[0]) { field_name[1] = field_name[0]; }; \
 													if (field_name[1] == field_name[2] && field_name[0] != field_name[1]) { field_name[0] = field_name[1]; }#define RAD_SAFE_FIELD_GET(field_name)				({ /* correct fields by voting, then return first */ \
														RAD_SAFE_FIELD_CORRECT(field_name); \
														field_name[0]; \
													})
/* #define RAD_SAFE_FIELD_CORRECT(field_name)			if (field_name[0] == field_name[1] && field_name[2] != field_name[0]) { field_name[2] = field_name[0]; assert(false); }; \
 													if (field_name[0] == field_name[2] && field_name[1] != field_name[0]) { field_name[1] = field_name[0]; assert(false); }; \
 													if (field_name[1] == field_name[2] && field_name[0] != field_name[1]) { field_name[0] = field_name[1]; assert(false); } */

#endif /* MRAM_COMMANDS_H_ */