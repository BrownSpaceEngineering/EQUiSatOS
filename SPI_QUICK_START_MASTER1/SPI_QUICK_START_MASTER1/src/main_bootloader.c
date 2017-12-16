#include <stdio.h>
#include <samd21j18a.h>
#include <stdbool.h>

#include <spi.h>
#include <usart.h>
#include <stdio_serial.h>
#include <delay.h>

#include "conf_uart_serial.h"

#include "Bootloader/mram.h"
#include "Bootloader/flash_memory.h"

#define SAM_BA_BOTH_INTERFACES      0
#define SAM_BA_UART_ONLY            1
#define SAM_BA_USBCDC_ONLY          2

#ifndef SAM_BA_INTERFACE
#define SAM_BA_INTERFACE    SAM_BA_BOTH_INTERFACES
#endif

#if SAM_BA_INTERFACE == SAM_BA_USBCDC_ONLY  ||  SAM_BA_INTERFACE == SAM_BA_UART_ONLY
#define APP_START_ADDRESS                 ((uint8_t *) 0x00003000)
#elif SAM_BA_INTERFACE == SAM_BA_BOTH_INTERFACES
#define APP_START_ADDRESS                 ((uint8_t *) 0x00003000)
#endif

#define APP_START_RESET_VEC_ADDRESS (APP_START_ADDRESS+(uint32_t)0x04)

#define NVM_SW_CALIB_DFLL48M_COARSE_VAL   58
#define NVM_SW_CALIB_DFLL48M_FINE_VAL     64

#define BINSIZE 1848 // size of LEDFlash binary in bytes

//My defines
#define BOOT_LOAD_PIN                     PIN_PA15
#define BOOT_PIN_MASK                     (1U << (BOOT_LOAD_PIN & 0x1f))
#define FLASH_WAIT_STATES                 1

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

static void check_start_application(void);

/**
 * \brief Check the application startup condition
 *
 */
static void check_start_application(void)
{
	uint32_t app_start_address;
	void (*application_code_entry)(void);

	/* Load the Reset Handler address of the application */
	application_code_entry = (void (*)(void))(unsigned *)(*(unsigned *)(APP_START_RESET_VEC_ADDRESS));

	/**
	 * Test reset vector of application @APP_START_ADDRESS+4
	 * Stay in SAM-BA if *(APP_START+0x4) == 0xFFFFFFFF
	 * Application erased condition
	 */
	if (app_start_address == 0xFFFFFFFF) {
		/* Stay in bootloader */
		return;
	}

	volatile PortGroup *boot_port = (volatile PortGroup *)(&(PORT->Group[BOOT_LOAD_PIN / 32]));
	volatile bool boot_en;

	/* Enable the input mode in Boot GPIO Pin */
	boot_port->DIRCLR.reg = BOOT_PIN_MASK;
	boot_port->PINCFG[BOOT_LOAD_PIN & 0x1F].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
	boot_port->OUTSET.reg = BOOT_PIN_MASK;
	/* Read the BOOT_LOAD_PIN status */
	boot_en = (boot_port->IN.reg) & BOOT_PIN_MASK;

	/* Check the bootloader enable condition */
	if (!boot_en) {
		/* Stay in bootloader */
		return;
	}

	/* Rebase the Stack Pointer */
	__set_MSP(*(uint32_t *) APP_START_ADDRESS);

	/* Rebase the vector table base address */
	SCB->VTOR = ((uint32_t) APP_START_ADDRESS & SCB_VTOR_TBLOFF_Msk);

	/* Jump to user Reset Handler in the application */
	application_code_entry();
}

// Returns the size, in bytes, of a 0xFF-terminated region of memory
//  (inspired by my implementation of strlen for x86-64 machines)
// XXX: DURR THIS DOESN'T ACTUALLY WORK
int binsize(void *begin) {
	unsigned int* rsp = (unsigned int*)begin; // Pointing into a 4-byte view of the buffer
	int offset = 0;
	while(true) { // we unroll the loop 4 bytes at a time (width of a pointer)
		int window = *rsp;
		// the following may need to be reversed if processor is big-endian:
		if (!(~window & 0xff)) return offset; // If first byte is 0xFF
		if (!(~window & 0xff00)) return offset + 1; // If second byte is 0xFF
		if (!(~window & 0xff0000)) return offset + 2; // etc...
		if (!(~window & 0xff000000)) return offset + 3;
		++rsp; offset+=4;
	}
}


void mram_test(struct spi_module* spi_master_instance, struct spi_slave_inst* slave) {
	#define TEST_ADDRESS 0x002
	#define NUM_BYTES	1024
	uint8_t example_array[8] = {0x01, 0x02, 0x03, 0x04, 0xa5, 0xfc, 0xff, 0x42};
	uint8_t example_output_array_before[NUM_BYTES];// = {0, 0, 0, 0, 0, 0, 0, 0};
	uint8_t example_output_array_after[NUM_BYTES];// = {0, 0, 0, 0, 0, 0, 0, 0};
	
	uint8_t status_reg = 0x0;
	read_status_register(spi_master_instance, slave, &status_reg);
	read_bytes(spi_master_instance, slave, example_output_array_before, NUM_BYTES, TEST_ADDRESS);
	write_bytes(spi_master_instance, slave, example_array, 8, TEST_ADDRESS);
	read_bytes(spi_master_instance, slave, example_output_array_after, NUM_BYTES, TEST_ADDRESS);
	
	return;
}


#ifdef DEBUG_ENABLE
#	define DEBUG_PIN_HIGH 	port_pin_set_output_level(BOOT_LED, 1)
#	define DEBUG_PIN_LOW 	port_pin_set_output_level(BOOT_LED, 0)
#else
#	define DEBUG_PIN_HIGH 	do{}while(0)
#	define DEBUG_PIN_LOW 	do{}while(0)
#endif

/*
/**
Function to define the bootloader, there are function to write the mram and to read from it,
after that it is necessary to call the function to write the flash and then start the application.
 */
int main(void)
{
	//test_mram();
	
	//uint8_t example_array[5] = {0x01, 0x02, 0x03, 0x04, 0x05};
	//save_binary_into_flash(example_array, 5, 512);
	
	struct spi_module spi_master_instance;
	struct spi_slave_inst slave;
	
	system_init();
	
	initialize_master(&spi_master_instance, 10000000); // seems to be the more "modern" implementation in mram.c
	initialize_slave(&slave);

	while (1) {
		mram_test(&spi_master_instance, &slave);
	}

	// write the binary into MRAM
	write_bytes(&spi_master_instance, &slave, APP_START_ADDRESS, BINSIZE, 0x00);
	
	check_start_application(); // jump to program
	
	return 0;
}
