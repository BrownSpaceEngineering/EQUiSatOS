#include <stdio.h>
#include <samd21j18a.h>
#include <stdbool.h>

#include <spi.h>
#include <usart.h>
#include <stdio_serial.h>
#include <delay.h>

#include "conf_uart_serial.h"

#include "Bootloader/MRAM_Commands.h"
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
#define APP_START_ADDRESS                 ((uint8_t *) 0x00003000) // NOTE: must be a multiple of NVMCTRL_PAGE_SIZE
#endif

#define APP_START_RESET_VEC_ADDRESS (APP_START_ADDRESS+(uint32_t)0x04)

#define NVM_SW_CALIB_DFLL48M_COARSE_VAL   58
#define NVM_SW_CALIB_DFLL48M_FINE_VAL     64

//My defines
#define BOOT_LOAD_PIN                     PIN_PA15
#define BOOT_PIN_MASK                     (1U << (BOOT_LOAD_PIN & 0x1f))

/************************************************************************/
/* program memory copying parameters                                    */
/************************************************************************/ 
// size of binary in bytes
#define BINSIZE							139520
// address at which binary is stored in mram
#define MRAM_APP_ADDRESS				938
// address at which prog mem rewritten boolean is stored in mram
#define MRAM_PROG_MEM_REWRITTEN_ADDR	42
// size of buffer to use when copying data from mram to flash
// IMPORTANT NOTE: MUST be multiple of NVM page size, i.e. NVMCTRL_PAGE_SIZE = 64
// (use powers of two and you'll be fine)
#define MRAM_READ_BUFFER_SIZE			5120

// DEBUGGING
#define RUN_TESTS

void mram_test(struct spi_module* spi_master_instance, struct spi_slave_inst* slave);
static void check_start_application(void);

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

    stdio_serial_init(&cdc_uart_module, CONF_STDIO_USART_MODULE, &usart_conf);
    usart_enable(&cdc_uart_module);
}

/**
 * \brief Check the application startup condition
 *
 */
static void check_start_application(void)
{
	void (*application_code_entry)(void);

	/* Load the Reset Handler address of the application */
	application_code_entry = (void (*)(void))(unsigned *)(*(unsigned *)(APP_START_RESET_VEC_ADDRESS));

	/**
	 * Test reset vector of application @APP_START_ADDRESS+4
	 * Stay in SAM-BA if *(APP_START+0x4) == 0xFFFFFFFF
	 * Application erased condition
	 */
	if ((uint32_t) APP_START_ADDRESS == 0xFFFFFFFF) {
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



#ifdef DEBUG_ENABLE
#	define DEBUG_PIN_HIGH 	port_pin_set_output_level(BOOT_LED, 1)
#	define DEBUG_PIN_LOW 	port_pin_set_output_level(BOOT_LED, 0)
#else
#	define DEBUG_PIN_HIGH 	do{}while(0)
#	define DEBUG_PIN_LOW 	do{}while(0)
#endif


/*
	Function to read the program memory from flash and its hashes from the MRAM, and
	check each section against its hash to determine if the entire set of memory is intact.
*/
bool check_prog_mem_integrity(struct spi_module* spi_master_instance,
	struct spi_slave_inst* mram_slave1, struct spi_slave_inst* mram_slave2) {
	// TODO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	return false;
}

/*
	Function to perform a buffered copy (we can't fit the entire program memory in RAM!)
	from the MRAM to the flash memory
*/
void copy_prog_mem_mram_to_flash(struct spi_module* spi_master_instance, 
	struct spi_slave_inst* mram_slave1, struct spi_slave_inst* mram_slave2) {
	uint8_t buffer_mram1[MRAM_READ_BUFFER_SIZE];
	uint8_t buffer_mram2[MRAM_READ_BUFFER_SIZE];
	int num_copied = 0;
	uint32_t mram_addr = MRAM_APP_ADDRESS;
	uint32_t flash_addr = (uint32_t) APP_START_ADDRESS;

	while (num_copied < BINSIZE) {
		// read the same data from both MRAMs (to compare them)
		mram_read_bytes(spi_master_instance, mram_slave1, buffer_mram1, MRAM_READ_BUFFER_SIZE, mram_addr);
		mram_read_bytes(spi_master_instance, mram_slave2, buffer_mram2, MRAM_READ_BUFFER_SIZE, mram_addr);
		
		if (memcmp(buffer_mram1, buffer_mram2, MRAM_READ_BUFFER_SIZE) != 0) {
			// TODO: what in the world to do? 
			// - hopefully we hash the same size sections as these buffers so we can compare hashes
		}
		
		// copy the buffer into the flash memory
		flash_mem_write_bytes(buffer_mram1, MRAM_READ_BUFFER_SIZE, flash_addr);
		
		num_copied += MRAM_READ_BUFFER_SIZE;
		mram_addr += MRAM_READ_BUFFER_SIZE;
		flash_addr += MRAM_READ_BUFFER_SIZE;
	}
}

/*
	Function to write to "program memory rewritten" field in MRAM.
*/
void set_prog_memory_rewritten(uint8_t was_rewritten, struct spi_module* spi_master_instance,
	struct spi_slave_inst* mram_slave1, struct spi_slave_inst* mram_slave2) {
	// write duplicate fields to both mrams (no spacing)
	mram_write_bytes(spi_master_instance, mram_slave1, &was_rewritten, 1, MRAM_PROG_MEM_REWRITTEN_ADDR);
	mram_write_bytes(spi_master_instance, mram_slave1, &was_rewritten, 1, MRAM_PROG_MEM_REWRITTEN_ADDR + 1);
	mram_write_bytes(spi_master_instance, mram_slave2, &was_rewritten, 1, MRAM_PROG_MEM_REWRITTEN_ADDR);
	mram_write_bytes(spi_master_instance, mram_slave2, &was_rewritten, 1, MRAM_PROG_MEM_REWRITTEN_ADDR + 1);
	// note: don't bother to read it back and confirm becasue there's nothing we can do and it doesn't really matter
}

/*
Function to define the bootloader, there are function to write the mram and to read from it,
after that it is necessary to call the function to write the flash and then start the application.
 */
int main(void)
{
	struct spi_module spi_master_instance;
	struct spi_slave_inst slave1;
	struct spi_slave_inst slave2;
	
	system_init();
	flash_mem_init();
	
	#ifdef RUN_TESTS
		mram_test(&spi_master_instance, &slave1);
		mram_test(&spi_master_instance, &slave2);
		uint8_t example_array[5] = {0x01, 0x02, 0x03, 0x04, 0x05};
		flash_mem_write_bytes(example_array, 5, 0x3000);
	#endif

	mram_initialize_master(&spi_master_instance, MRAM_SPI_BAUD);
	mram_initialize_slave(&slave1, P_MRAM1_CS);
	mram_initialize_slave(&slave2, P_MRAM2_CS);
	
	// TODO: Instead of checking integrity of ENTIRE thing and then rewriting ENTIRE thing, 
	// match the size of the buffered read with the hashed sections of the program memory, 
	// and only correct the "sectors" which don't match their hashes
	bool prog_mem_hash_matches = check_prog_mem_integrity(&spi_master_instance, &slave1, &slave2);
	if (!prog_mem_hash_matches) {
		// if the program memory in the flash doesn't match the stored hash, rewrite it
		copy_prog_mem_mram_to_flash(&spi_master_instance, &slave1, &slave2);
	}
	set_prog_memory_rewritten(!prog_mem_hash_matches, &spi_master_instance, &slave1, &slave2);
	
	// jump to start of program in memory
	check_start_application();
	
	return 0;
}

void mram_test(struct spi_module* spi_master_instance, struct spi_slave_inst* slave) {
	#define TEST_ADDRESS 0x002
	#define NUM_BYTES	1024
	uint8_t example_array[8] = {0x01, 0x02, 0x03, 0x04, 0xa5, 0xfc, 0xff, 0x42};
	uint8_t example_output_array_before[NUM_BYTES];// = {0, 0, 0, 0, 0, 0, 0, 0};
	uint8_t example_output_array_after[NUM_BYTES];// = {0, 0, 0, 0, 0, 0, 0, 0};
	
	uint8_t status_reg = 0x0;
	mram_read_status_register(spi_master_instance, slave, &status_reg);
	mram_read_bytes(spi_master_instance, slave, example_output_array_before, NUM_BYTES, TEST_ADDRESS);
	mram_write_bytes(spi_master_instance, slave, example_array, 8, TEST_ADDRESS);
	mram_read_bytes(spi_master_instance, slave, example_output_array_after, NUM_BYTES, TEST_ADDRESS);
	
	return;
}
