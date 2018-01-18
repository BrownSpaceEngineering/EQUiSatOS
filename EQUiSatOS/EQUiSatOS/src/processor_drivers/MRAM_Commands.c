/*
 * MRAM_Commands.c
 *
 * Created: 4/10/2016 2:27:08 PM
 *  Author: Gustavo
 */ 

#include "MRAM_Commands.h"

void copy_control_data(uint8_t *buffer, uint16_t address, uint8_t command){
	buffer[0] = command;
	buffer[1] = 0x0; // only bottom 16 bits of address are decoded, so skip a byte
	buffer[2] = address >> 8;		// upper byte
	buffer[3] = address;			// lower byte
}

uint8_t mram_initialize_master(struct spi_module *spi_master_instance, uint32_t baudrate){
	struct spi_config config_spi_master;
	spi_get_config_defaults(&config_spi_master);
	config_spi_master.mux_setting = SPI_SIGNAL_MUX_SETTING_E;
	// see here for correspondence: http://asf.atmel.com/docs/latest/samd21/html/asfdoc_sam0_sercom_spi_mux_settings.html
	config_spi_master.pinmux_pad0 = P_SPI_MISO;
	config_spi_master.pinmux_pad1 = PINMUX_UNUSED;
	config_spi_master.pinmux_pad2 = P_SPI_MOSI;
	config_spi_master.pinmux_pad3 = P_SPI_SCK;
	spi_init(spi_master_instance, MRAM_SPI_SERCOM, &config_spi_master);
	enum status_code code = spi_set_baudrate(spi_master_instance, baudrate);
	spi_enable(spi_master_instance);
}

uint8_t mram_initialize_slave(struct spi_slave_inst *slave, int ss_pin) {
	struct spi_slave_inst_config slave_dev_config;
	spi_slave_inst_get_config_defaults(&slave_dev_config);
	slave_dev_config.ss_pin = ss_pin;
	spi_attach_slave(slave, &slave_dev_config);
}

void enable_write(struct spi_module *spi_master_instance, struct spi_slave_inst *slave){
	const uint16_t enable = ENABLE_COMMAND;
	spi_select_slave(spi_master_instance, slave, false);
	spi_select_slave(spi_master_instance, slave, true);
	enum status_code code_1 = spi_write(spi_master_instance, enable);
	spi_select_slave(spi_master_instance, slave, false);
}

// returns true (1) if stat is a STATUS_CATEGORY_OK
uint8_t status_ok(status_code_genare_t stat) {
	return (stat < STATUS_CATEGORY_COMMON);
}

// returns a status code, and will fail partway through if the write fails
status_code_genare_t mram_write_bytes(struct spi_module *spi_master_instance, struct spi_slave_inst *slave, uint8_t *data, int num_bytes, uint16_t address){
	enable_write(spi_master_instance, slave);
	uint8_t write_control[NUM_CONTROL_BYTES], data_rx_temp[num_bytes], control_rx_temp[NUM_CONTROL_BYTES];
	copy_control_data(write_control, address, WRITE_COMMAND);
	status_code_genare_t s = spi_select_slave(spi_master_instance, slave, true);
	if (!status_ok(s)) return s;
	
	// write our 4-byte write command to the MRAM with the address
	// (storing trash in rx buffer in a dummy temp variable while doing so)
	s = spi_transceive_buffer_wait(spi_master_instance, write_control, control_rx_temp, NUM_CONTROL_BYTES);
	if (!status_ok(s)) return s;
	
	// write the number of bytes specified into the MRAM
	// (storing trash in rx buffer in a dummy temp variable while doing so)
	s = spi_transceive_buffer_wait(spi_master_instance, data, data_rx_temp, num_bytes);
	if (!status_ok(s)) return s;
	
	s = spi_select_slave(spi_master_instance, slave, false);
	return s;
}

status_code_genare_t mram_read_bytes(struct spi_module *spi_master_instance, struct spi_slave_inst *slave, uint8_t *data, int num_bytes, uint16_t address){
	uint8_t read_control[NUM_CONTROL_BYTES], data_tx_temp[num_bytes], control_rx_temp[NUM_CONTROL_BYTES];
	copy_control_data(read_control, address, READ_COMMAND);
	status_code_genare_t s = spi_select_slave(spi_master_instance, slave, true);
	if (!status_ok(s)) return s;
	
	// write our 4-byte read command to the MRAM with the address
	// (storing trash in rx buffer in a dummy temp variable while doing so)
	// for some reason we have to suck up a byte before we get to the start of the data 
	// at the address we specified, so read an extra control byte
	s = spi_transceive_buffer_wait(spi_master_instance, read_control, control_rx_temp, NUM_CONTROL_BYTES);
	if (!status_ok(s)) return s;
	
	// read the number of bytes specified coming from the MRAM
	// (sending trash over tx buffer from a dummy temp variable while doing so)
	s = spi_transceive_buffer_wait(spi_master_instance, data_tx_temp, data, num_bytes);
	if (!status_ok(s)) return s;
	
	s = spi_select_slave(spi_master_instance, slave, false);
	return s;
}

status_code_genare_t mram_read_status_register(struct spi_module *spi_master_instance, struct spi_slave_inst *slave, uint8_t *reg_out) {
	uint8_t control_rx_temp, data_tx_temp;
	uint8_t read_control = READ_STATUS_REG_COMMAND;
	status_code_genare_t s = spi_select_slave(spi_master_instance, slave, true);
	if (!status_ok(s)) return s;

	// write 1 byte command to read register 
	// (storing trash in rx buffer in a dummy temp variable while doing so)
	s = spi_transceive_buffer_wait(spi_master_instance, &read_control, &control_rx_temp, 1);
	if (!status_ok(s)) return s;
	
	// read 1 byte register data 
	// (sending trash over tx buffer from a dummy temp variable while doing so)
	s = spi_transceive_buffer_wait(spi_master_instance, &data_tx_temp, reg_out, 1);
	if (!status_ok(s)) return s;
	
	s = spi_select_slave(spi_master_instance, slave, false);
	return s;
}
