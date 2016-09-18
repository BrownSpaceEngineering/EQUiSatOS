/*
 * SPI_Commands.c
 *
 * Created: 5/8/2015 3:45:25 PM
 *  Author: Daniel
 */ 

#include "SPI_Commands.h"

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
	config_spi_master.pinmux_pad0 = EXT1_SPI_SERCOM_PINMUX_PAD0;
	//! [di]
	/* Configure pad 1 as unused */
	//! [ss]
	config_spi_master.pinmux_pad1 = PINMUX_UNUSED;
	//! [ss]
	/* Configure pad 2 for data out */
	//! [do]
	config_spi_master.pinmux_pad2 = EXT1_SPI_SERCOM_PINMUX_PAD2;
	//! [do]
	/* Configure pad 3 for SCK */
	//! [sck]
	config_spi_master.pinmux_pad3 = EXT1_SPI_SERCOM_PINMUX_PAD3;
	//! [sck]
	//! [init]
	spi_init(&spi_master_instance, EXT1_SPI_MODULE, &config_spi_master);
	//! [init]

	//! [enable]
	spi_enable(&spi_master_instance);
	//! [enable]

}
