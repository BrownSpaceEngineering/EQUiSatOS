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


// start address of program memory (reset handler/main execution point)
// (this is in the flash memory (program memory) address space)
// NOTE: must be a multiple of NVMCTRL_PAGE_SIZE
#if SAM_BA_INTERFACE == SAM_BA_USBCDC_ONLY  ||  SAM_BA_INTERFACE == SAM_BA_UART_ONLY
#define APP_START_ADDRESS                 ((uint8_t *) 0x00003000)
#elif SAM_BA_INTERFACE == SAM_BA_BOTH_INTERFACES
#define APP_START_ADDRESS                 ((uint8_t *) 0x00006000)
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
#define PROG_MEM_SIZE						80996
// address at which binary is stored in mram
#define MRAM_APP_ADDRESS					938
// address at which prog mem rewritten boolean is stored in mram
#define MRAM_PROG_MEM_REWRITTEN_ADDR		42
// size of buffer to use when copying data from mram to flash
// IMPORTANT NOTE: MUST be multiple of NVM page size, i.e. NVMCTRL_PAGE_SIZE = 64
// (use powers of two and you'll be fine)
#define MRAM_READ_BUFFER_SIZE				5120

// DEBUGGING
//#define RUN_TESTS

#ifdef RUN_TESTS
void mram_test(struct spi_module* spi_master_instance, struct spi_slave_inst* slave);
void flash_mem_test(void);
#endif

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
	// TODO: check this!!!!
	application_code_entry = (void (*)(void))(unsigned *)(*(unsigned *)(APP_START_RESET_VEC_ADDRESS));

	/**
	 * Test reset vector of application @APP_START_ADDRESS+4
	 * Stay in SAM-BA if *(APP_START+0x4) == 0xFFFFFFFF
	 * Application erased condition
	 */
	if (*((uint32_t*) APP_START_RESET_VEC_ADDRESS) == 0xFFFFFFFF) {
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
		// TODO: do something more drastic (try to rewrite whole program memory?)
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
	Checks whether the program memory located at flash_addr in the flash memory
	matches the data in the given buffer (read from the MRAM).
	Returns whether it matches.
*/
bool check_prog_mem_integrity(uint32_t flash_addr, uint8_t* mram_buffer, size_t buf_size) {
	// read data at flash_addr into buffer and compare with MRAM buffer
	//uint8_t flash_buffer[buf_size];
	//nvm_read_buffer(flash_addr, flash_buffer, buf_size);
	//return memcmp(flash_buffer, mram_buffer, buf_size) == 0;
	
	// simply compare program memory at address with mram_buffer
	return memcmp((uint8_t*) flash_addr, mram_buffer, buf_size) == 0;
}

/*
	Checks each sub-buffer of program memory stored in the MRAM against that stored
	in the actual flash, and corrects any buffers that don't match.
	Returns whether any were corrected
*/
bool check_and_fix_prog_mem(struct spi_module* spi_master_instance, 
	struct spi_slave_inst* mram_slave1, struct spi_slave_inst* mram_slave2) {
	
	// note: these are big buffers, so we make sure to stored them on the stack rather than
	// in BSS where they'll be part of the bootloader
	uint8_t buffer_mram1[MRAM_READ_BUFFER_SIZE];
	uint8_t buffer_mram2[MRAM_READ_BUFFER_SIZE];
	
	int num_copied = 0;
	uint32_t mram_addr = MRAM_APP_ADDRESS;
	uint32_t flash_addr = (uint32_t) APP_START_ADDRESS;
	bool correction_made = false;

	while (num_copied < PROG_MEM_SIZE) {
		size_t buf_size = min(PROG_MEM_SIZE - num_copied, MRAM_READ_BUFFER_SIZE);
		
		// read the same data from both MRAMs (to compare them)
		mram_read_bytes(spi_master_instance, mram_slave1, buffer_mram1, buf_size, mram_addr);
		mram_read_bytes(spi_master_instance, mram_slave2, buffer_mram2, buf_size, mram_addr);
		
		// check that the buffers copied from the MRAM match
		if (memcmp(buffer_mram1, buffer_mram2, buf_size) != 0) {
			// if they don't, see if either matches the program memory 
			bool buffer_mram1_matched = check_prog_mem_integrity(flash_addr, buffer_mram1, buf_size);
			bool buffer_mram2_matched = check_prog_mem_integrity(flash_addr, buffer_mram1, buf_size);

			// if both failed to match, we're pretty screwed, but take the copy in the MRAM becuase
			// our most likely cause of failure is radiation bit flips...
			if (!buffer_mram1_matched && !buffer_mram2_matched) {
				// (take first buffer arbitrarily)
				
				// TODO TODO TODO TODO: take the one with the shortest section of same bytes 
				// (if it's an MRAM failure, it's likely to be a line either pulled HIGH (all 0xff) or LOW (all 0x00))
				
				flash_mem_write_bytes(buffer_mram1, buf_size, flash_addr);
				correction_made = true;
			}
			// otherwise, if one did match, we're probably okay, because it's unlikely an MRAM and the actual program memory
			// would be corrupted in the same way
			// (note that the case that both matched is handled outside here (below))
			
			// TODO: more decision making with hashes?
		} else {
			// compare this current batch to the actual data stored in the program memory,
			// to determine whether it must be rewritten
			if (!check_prog_mem_integrity(flash_addr, buffer_mram1, buf_size)) {
				// if comparison failed, copy the buffer from mram to the flash memory
				flash_mem_write_bytes(buffer_mram1, buf_size, flash_addr);
				correction_made = true;
			}
		}
		
		num_copied += buf_size;
		mram_addr += buf_size;
		flash_addr += buf_size;
	}
	return correction_made;
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
	// note: don't bother to read it back and confirm because there's nothing we can do and it doesn't really matter
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
	mram_initialize_master(&spi_master_instance, MRAM_SPI_BAUD);
	mram_initialize_slave(&slave1, P_MRAM1_CS);
	mram_initialize_slave(&slave2, P_MRAM2_CS);
	
	#ifdef RUN_TESTS
		//mram_test(&spi_master_instance, &slave1);
		//mram_test(&spi_master_instance, &slave2);
		//flash_mem_test();
	#endif

	// read in batches of program memory from the MRAM, and compare each to its value
	// currently in the flash program memory, and correct any section (batch-sized) if necessary
	bool any_rewritten = check_and_fix_prog_mem(&spi_master_instance, &slave1, &slave2);
	set_prog_memory_rewritten(any_rewritten, &spi_master_instance, &slave1, &slave2);
	
	// jump to start of program in memory
	check_start_application();
	
	return 0;
}

#ifdef RUN_TESTS

void mram_test(struct spi_module* spi_master_instance, struct spi_slave_inst* slave) {
	#define MRAM_TEST_ADDRESS 0x202
	#define MRAM_NUM_BYTES	1024
	uint8_t example_array[8] = {0x01, 0x02, 0x03, 0x04, 0xa5, 0xfc, 0xff, 0x42};
	uint8_t example_output_array_before[MRAM_NUM_BYTES];// = {0, 0, 0, 0, 0, 0, 0, 0};
	uint8_t example_output_array_after[MRAM_NUM_BYTES];// = {0, 0, 0, 0, 0, 0, 0, 0};
	
	uint8_t status_reg = 0x0;
	mram_read_status_register(spi_master_instance, slave, &status_reg);
	mram_read_bytes(spi_master_instance, slave, example_output_array_before, MRAM_NUM_BYTES, MRAM_TEST_ADDRESS);
	mram_write_bytes(spi_master_instance, slave, example_array, 8, MRAM_TEST_ADDRESS);
	mram_read_bytes(spi_master_instance, slave, example_output_array_after, MRAM_NUM_BYTES, MRAM_TEST_ADDRESS);
	
	return;
}

void flash_mem_test(void) {
	#define FLASH_TEST_ADDRESS 0x6000
	#define FLASH_NUM_BYTES 5
	uint8_t example_array[FLASH_NUM_BYTES] =		{0x01, 0x02, 0x03, 0x04, 0x05};
	uint8_t example_out_array[FLASH_NUM_BYTES] =	{0x00, 0x00, 0x00, 0x00, 0x00};
	flash_mem_write_bytes(example_array, FLASH_NUM_BYTES, FLASH_TEST_ADDRESS);
	nvm_read_buffer(FLASH_TEST_ADDRESS, example_out_array, FLASH_NUM_BYTES);
	return;
}

#endif