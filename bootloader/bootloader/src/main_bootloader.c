#include <stdio.h>
#include <samd21j18a.h>
#include <stdbool.h>
#include <spi.h>

#include "Bootloader/MRAM_Commands.h"
#include "Bootloader/flash_memory.h"
#include "Bootloader/Watchdog_Commands.h"

/* CONFIG */
#define ENABLE_REWRITE_FROM_MRAM
#define BOOT_TO_PROG
//#define WRITE_PROG_MEM_TO_MRAM
//#define CONFIRM_PROG_MEM_MRAM_WRITE
//#define RUN_ASSERTS
//#define WAIT_ON_BOOT
//#define WAIT_ON_BOOT_TIME_MS			10000 
//#define SET_INIT_SAT_STATE
//#define PREV_SAT_STATE_VALUE			0		// INITIAL = 0; IDLE_NO_FLASH = 3
//#define RUN_TESTS

#if defined(WRITE_PROG_MEM_TO_MRAM) || defined(RUN_TESTS) || defined(RUN_ASSERTS) || defined(CONFIRM_PROG_MEM_WRITE)
#define configASSERT( x ) \
			 if( ( x ) == 0 ) { for( ;; ) pet_watchdog(); }
#endif

#ifdef WAIT_ON_BOOT
#include <delay.h>
#endif

/************************************************************************/
// start addresses of program memory (reset handler/main execution point)
// (this is in the flash memory (program memory) address space)
// NOTE: must be a multiple of NVMCTRL_PAGE_SIZE                                                            
/************************************************************************/
#define APP_START_ADDRESS                 ((uint8_t *) 0x00006000)

// rad-safe triple-redundant versions (plus other crucial constants)
RAD_SAFE_FIELD_INIT(uint8_t*, app_start_address, APP_START_ADDRESS);
RAD_SAFE_FIELD_INIT(unsigned long, scb_vtor_tbloff_msk, SCB_VTOR_TBLOFF_Msk);

/************************************************************************/
/* program memory copying parameters                                    */
/************************************************************************/
// size of binary in bytes
RAD_SAFE_FIELD_INIT(size_t, prog_mem_size,					112356);
// address at which binary is stored in mram
RAD_SAFE_FIELD_INIT(uint32_t, mram_app_address,				262644);
// address at which prog mem rewritten boolean is stored in mram + size
RAD_SAFE_FIELD_INIT(uint32_t, mram_prog_mem_rewritten_addr, 42);
RAD_SAFE_FIELD_INIT(uint32_t, mram_prog_mem_rewritten_size, 1);
// size of buffer to use when copying data from mram to flash
// IMPORTANT NOTE: MUST be multiple of NVM page size, i.e. NVMCTRL_PAGE_SIZE = 64
// (use powers of two and you'll be fine)
#define MRAM_COPY_BUFFER_SIZE								5120
RAD_SAFE_FIELD_INIT(size_t, mram_copy_buffer_size, MRAM_COPY_BUFFER_SIZE);

// MRAM compare buffers
// note: these are big buffers, so we make sure to stored them in the BSS
// rather than the stack (otherwise we stack overflow)
uint8_t buffer_mram1[MRAM_COPY_BUFFER_SIZE];
uint8_t buffer_mram2[MRAM_COPY_BUFFER_SIZE];

#ifdef WRITE_PROG_MEM_TO_MRAM
void write_cur_prog_mem_to_mram(struct spi_module* spi_master_instance,
	struct spi_slave_inst* mram_slave1, struct spi_slave_inst* mram_slave2);
#endif

#ifdef SET_INIT_SAT_STATE
void write_default_mram_vals(struct spi_module* spi_master_instance, struct spi_slave_inst* mram_slave1, struct spi_slave_inst* mram_slave2);
#endif

void set_mram_protection(struct spi_module* spi_master_instance,
	struct spi_slave_inst* mram_slave1, struct spi_slave_inst* mram_slave2, 
	uint8_t status_reg);

#ifdef RUN_TESTS
void mram_test(struct spi_module* spi_master_instance, struct spi_slave_inst* slave);
void flash_mem_test(void);
void corrupt_prog_mem(void);
#endif

/**
 * \brief Check the application startup condition
 *
 */
static void start_application(void)
{
	uint32_t reset_handler_address;

	/* Load the Reset Handler address of the application */
	reset_handler_address = *(uint32_t *)(RAD_SAFE_FIELD_GET(app_start_address) + 4);

	/**
	 * Test reset vector of application @APP_START_ADDRESS+4
	 * (check if application was erased)
	 * Stay in SAM-BA if *(APP_START+0x4) == 0xFFFFFFFF
	 */
	if (reset_handler_address == 0xFFFFFFFF) {
		// try again...
		system_reset();
		return;
	}

	/* Rebase the Stack Pointer */
	__set_MSP(*(uint32_t *) RAD_SAFE_FIELD_GET(app_start_address));

	/* Rebase the vector table base address */
	SCB->VTOR = ((uint32_t) RAD_SAFE_FIELD_GET(app_start_address) & RAD_SAFE_FIELD_GET(scb_vtor_tbloff_msk));

	/* Jump to user Reset Handler in the application */
	asm("bx %0"::"r"(reset_handler_address));
}

#ifdef ENABLE_REWRITE_FROM_MRAM

// Returns the number of matching bytes in the two buffers up to len.
static size_t num_similar(uint8_t* data1, uint8_t* data2, size_t len) {
	size_t num_same = 0;
	for (size_t i = 0; i < len; i++) {
		if (data1[i] == data2[i]) {
			num_same++;
		}
	}
	return num_same;
}

/*
	Checks each sub-buffer of program memory stored in the MRAM against that stored
	in the actual flash, and corrects any buffers that don't match.
	Returns whether any were corrected
*/
static int check_and_fix_prog_mem(struct spi_module* spi_master_instance,
	struct spi_slave_inst* mram_slave1, struct spi_slave_inst* mram_slave2) {

	uint num_copied = 0;
	uint32_t mram_addr = RAD_SAFE_FIELD_GET(mram_app_address);
	uint32_t flash_addr = (uint32_t) RAD_SAFE_FIELD_GET(app_start_address);
	uint corrections_made = 0;

	while (num_copied < RAD_SAFE_FIELD_GET(prog_mem_size)) {
		size_t buf_size = min(RAD_SAFE_FIELD_GET(prog_mem_size) - num_copied, RAD_SAFE_FIELD_GET(mram_copy_buffer_size));

		pet_watchdog(); // pet watchdog before long read
		// read the same data from both MRAMs (to compare them)
		mram_read_bytes(spi_master_instance, mram_slave1, buffer_mram1, buf_size, mram_addr);
		pet_watchdog(); // pet after long read
		mram_read_bytes(spi_master_instance, mram_slave2, buffer_mram2, buf_size, mram_addr);
		pet_watchdog(); // pet after long read

		// check that the buffers copied from the MRAM match
		if (memcmp(buffer_mram1, buffer_mram2, buf_size) != 0) {
			#ifdef RUN_ASSERTS
				configASSERT(false); // shouldn't happen on loading bootloader
			#endif
			
			// if they don't, see if either matches the program memory
			bool buffer_mram1_matched_flash = memcmp((uint8_t*) flash_addr, buffer_mram1, buf_size) == 0;
			bool buffer_mram2_matched_flash = memcmp((uint8_t*) flash_addr, buffer_mram2, buf_size) == 0;

			// if both failed to match, we're pretty screwed, but take the copy in the MRAM because
			// our most likely cause of failure is radiation bit flips...
			if (!buffer_mram1_matched_flash && !buffer_mram2_matched_flash) {
				// Take the one most similar to our "third buffer," the flash
				// (if it's an MRAM failure, it's likely to be a line either pulled HIGH (all 0xff) or LOW (all 0x00))
				size_t mram1_same_seq_len = num_similar((uint8_t*) flash_addr, buffer_mram1, buf_size);
				size_t mram2_same_seq_len = num_similar((uint8_t*) flash_addr, buffer_mram2, buf_size);
				
				pet_watchdog(); // pet before flash write
				// if mram1 has less similarity than mram2, write mram2 buffer + vise versa
				if (mram1_same_seq_len <= mram2_same_seq_len) {
					flash_mem_write_bytes(buffer_mram2, buf_size, flash_addr);
				} else {
					flash_mem_write_bytes(buffer_mram1, buf_size, flash_addr);
				}
				corrections_made++;
			}
			// otherwise, if one did match, we're probably okay, because it's unlikely an MRAM and the actual program memory
			// would be corrupted in the same way
			// (note that the case that both matched is handled outside here (below))

		} else {
			// compare this current batch to the actual data stored in the program memory,
			// to determine whether it must be rewritten
			if (memcmp((uint8_t*) flash_addr, buffer_mram1, buf_size) != 0) {
				#ifdef RUN_ASSERTS
					configASSERT(false); // shouldn't happen on loading bootloader
				#endif
				
				pet_watchdog(); // pet before flash write
				
				// if comparison failed, copy the buffer from mram to the flash memory
				flash_mem_write_bytes(buffer_mram1, buf_size, flash_addr);
				corrections_made++;
			}
		}

		num_copied += buf_size;
		mram_addr += buf_size;
		flash_addr += buf_size;
	}
	return corrections_made;
}

#endif

/*
	Function to write to "program memory rewritten" field in MRAM.
*/
static void set_prog_memory_rewritten(uint8_t was_rewritten, struct spi_module* spi_master_instance,
	struct spi_slave_inst* mram_slave1, struct spi_slave_inst* mram_slave2) {
	// write duplicate fields to both mrams (no spacing)
	pet_watchdog();
	uint8_t field_size = RAD_SAFE_FIELD_GET(mram_prog_mem_rewritten_size); // == 1
	mram_write_bytes(spi_master_instance, mram_slave1, &was_rewritten, field_size, RAD_SAFE_FIELD_GET(mram_prog_mem_rewritten_addr));
	mram_write_bytes(spi_master_instance, mram_slave1, &was_rewritten, field_size, RAD_SAFE_FIELD_GET(mram_prog_mem_rewritten_addr) + field_size);
	mram_write_bytes(spi_master_instance, mram_slave2, &was_rewritten, field_size, RAD_SAFE_FIELD_GET(mram_prog_mem_rewritten_addr));
	mram_write_bytes(spi_master_instance, mram_slave2, &was_rewritten, field_size, RAD_SAFE_FIELD_GET(mram_prog_mem_rewritten_addr) + field_size);
	// note: don't bother to read it back and confirm because there's nothing we can do and it doesn't really matter
	#if defined(RUN_ASSERTS)
		uint8_t was_rewritten_temp1, was_rewritten_temp2;
		mram_read_bytes(spi_master_instance, mram_slave1, &was_rewritten_temp1, field_size, RAD_SAFE_FIELD_GET(mram_prog_mem_rewritten_addr) + field_size);
		mram_read_bytes(spi_master_instance, mram_slave2, &was_rewritten_temp2, field_size, RAD_SAFE_FIELD_GET(mram_prog_mem_rewritten_addr) + field_size);
		configASSERT(was_rewritten == was_rewritten_temp1);
		configASSERT(was_rewritten == was_rewritten_temp2);
	#endif
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
	
	#ifdef WAIT_ON_BOOT
		delay_init();
		delay_ms(WAIT_ON_BOOT_TIME_MS);
	#endif
	
	configure_watchdog(NULL);
	flash_mem_init();
	mram_initialize_master(&spi_master_instance, MRAM_SPI_BAUD);
	mram_initialize_slave(&slave1, P_MRAM1_CS);
	mram_initialize_slave(&slave2, P_MRAM2_CS);

	#ifdef RUN_TESTS
		//write_default_mram_vals(&spi_master_instance, &slave1, &slave2);
		//mram_test(&spi_master_instance, &slave1);
		//mram_test(&spi_master_instance, &slave2);
		//flash_mem_test();
		//corrupt_prog_mem();
	#endif

	#ifdef SET_INIT_SAT_STATE
		write_default_mram_vals(&spi_master_instance, &slave1, &slave2);
	#endif

	#ifdef WRITE_PROG_MEM_TO_MRAM
		write_cur_prog_mem_to_mram(&spi_master_instance, &slave1, &slave2);
	#endif

	#ifdef ENABLE_REWRITE_FROM_MRAM
		// read in batches of program memory from the MRAM, and compare each to its value
		// currently in the flash program memory, and correct any section (batch-sized) if necessary
 		int num_bufs_rewritten = check_and_fix_prog_mem(&spi_master_instance, &slave1, &slave2);
 		set_prog_memory_rewritten(num_bufs_rewritten > 0, &spi_master_instance, &slave1, &slave2);
	#else 
		// note we didn't rewrite for consistency
		set_prog_memory_rewritten(false, &spi_master_instance, &slave1, &slave2);
	#endif

	// reset SPI module to avoid conflicts when OS uses it
	mram_reset(&spi_master_instance);
	
	pet_watchdog(); // pet before initial app jump

	#ifdef BOOT_TO_PROG
		// jump to start of program in memory
		start_application();
	#else
		// ensure no watchdog reboot
		while (true) {
			pet_watchdog();
		}
	#endif

	return 0;
}

#if defined(SET_INIT_SAT_STATE) || defined(RUN_TESTS)

// writes default application MRAM values
#define APPLICATION_MRAM_VALS_START		20
#define CUMULATIVE_FIELDS_SIZE			844
#define PERSISTENT_BAT_DATA_ADDR		46
#define PREV_SAT_STATE_ADDR				34
void write_default_mram_vals(struct spi_module* spi_master_instance,
	struct spi_slave_inst* mram_slave1, struct spi_slave_inst* mram_slave2) {
	// write zeros for all fields
	uint8_t zeros[CUMULATIVE_FIELDS_SIZE];
	// these lines just included so the stored state can be viewed in a debugger
	mram_read_bytes(spi_master_instance, mram_slave1, zeros, CUMULATIVE_FIELDS_SIZE, APPLICATION_MRAM_VALS_START);
	mram_read_bytes(spi_master_instance, mram_slave2, zeros, CUMULATIVE_FIELDS_SIZE, APPLICATION_MRAM_VALS_START);
	
	// write other values first while unlocked to ensure lock doesn't affect this part
	set_mram_protection(spi_master_instance, mram_slave1, mram_slave2, STATUS_REG_PROTECT_NONE);
	memset(zeros, 1, CUMULATIVE_FIELDS_SIZE);
	mram_write_bytes(spi_master_instance, mram_slave1, zeros, CUMULATIVE_FIELDS_SIZE, APPLICATION_MRAM_VALS_START);
	mram_write_bytes(spi_master_instance, mram_slave2, zeros, CUMULATIVE_FIELDS_SIZE, APPLICATION_MRAM_VALS_START);
	
	set_mram_protection(spi_master_instance, mram_slave1, mram_slave2, STATUS_REG_PROTECT_TOP_HALF);
	memset(zeros, 0, CUMULATIVE_FIELDS_SIZE);
	mram_write_bytes(spi_master_instance, mram_slave1, zeros, CUMULATIVE_FIELDS_SIZE, APPLICATION_MRAM_VALS_START);
	mram_write_bytes(spi_master_instance, mram_slave2, zeros, CUMULATIVE_FIELDS_SIZE, APPLICATION_MRAM_VALS_START);
	
	// update persistent charging data field to be 0xff
	uint8_t persistent_charging_data = 0xff;
	mram_write_bytes(spi_master_instance, mram_slave1, &persistent_charging_data, 1, PERSISTENT_BAT_DATA_ADDR);
	mram_write_bytes(spi_master_instance, mram_slave1, &persistent_charging_data, 1, PERSISTENT_BAT_DATA_ADDR + 1);
	mram_write_bytes(spi_master_instance, mram_slave2, &persistent_charging_data, 1, PERSISTENT_BAT_DATA_ADDR);
	mram_write_bytes(spi_master_instance, mram_slave2, &persistent_charging_data, 1, PERSISTENT_BAT_DATA_ADDR + 1);
	
	uint8_t prev_sat_state = PREV_SAT_STATE_VALUE;
	mram_write_bytes(spi_master_instance, mram_slave1, &prev_sat_state, 1, PREV_SAT_STATE_ADDR);
	mram_write_bytes(spi_master_instance, mram_slave1, &prev_sat_state, 1, PREV_SAT_STATE_ADDR + 1);
	mram_write_bytes(spi_master_instance, mram_slave2, &prev_sat_state, 1, PREV_SAT_STATE_ADDR);
	mram_write_bytes(spi_master_instance, mram_slave2, &prev_sat_state, 1, PREV_SAT_STATE_ADDR + 1);
	
	#ifdef RUN_ASSERTS
		uint8_t written_data[CUMULATIVE_FIELDS_SIZE];
		memset(written_data, 12, CUMULATIVE_FIELDS_SIZE); // set random value to distinguish
		uint8_t expected_data[CUMULATIVE_FIELDS_SIZE];
		memset(expected_data, 0, CUMULATIVE_FIELDS_SIZE);
		expected_data[PERSISTENT_BAT_DATA_ADDR - APPLICATION_MRAM_VALS_START] = 0xff;
		expected_data[1 + PERSISTENT_BAT_DATA_ADDR - APPLICATION_MRAM_VALS_START] = 0xff;
		expected_data[PREV_SAT_STATE_ADDR - APPLICATION_MRAM_VALS_START] = PREV_SAT_STATE_VALUE;
		expected_data[1 + PREV_SAT_STATE_ADDR - APPLICATION_MRAM_VALS_START] = PREV_SAT_STATE_VALUE;
	
		mram_read_bytes(spi_master_instance, mram_slave1, written_data, CUMULATIVE_FIELDS_SIZE, APPLICATION_MRAM_VALS_START);
		configASSERT(memcmp(written_data, expected_data, CUMULATIVE_FIELDS_SIZE) == 0);
		mram_read_bytes(spi_master_instance, mram_slave2, written_data, CUMULATIVE_FIELDS_SIZE, APPLICATION_MRAM_VALS_START);
		configASSERT(memcmp(written_data, expected_data, CUMULATIVE_FIELDS_SIZE) == 0);
	#endif
}

void set_mram_protection(struct spi_module* spi_master_instance,
	struct spi_slave_inst* mram_slave1, struct spi_slave_inst* mram_slave2, 
	uint8_t status_reg) {
	uint8_t status_reg1;
	uint8_t status_reg2;
	mram_read_status_register(spi_master_instance, mram_slave1, &status_reg1); // just to debug value
	mram_write_status_register(spi_master_instance, mram_slave1, status_reg);
	#ifdef RUN_ASSERTS
		mram_read_status_register(spi_master_instance, mram_slave1, &status_reg2);
		configASSERT(status_reg2 == (status_reg | 0b10)); // WEN bit will be set
	#endif
	mram_read_status_register(spi_master_instance, mram_slave2, &status_reg1); // just to debug value
	mram_write_status_register(spi_master_instance, mram_slave2, status_reg);
	#ifdef RUN_ASSERTS
		mram_read_status_register(spi_master_instance, mram_slave2, &status_reg2);
		configASSERT(status_reg2 == (status_reg | 0b10)); // WEN bit will be set
	#endif
}

#endif

#ifndef ENABLE_REWRITE_FROM_MRAM

// writes the currently-loaded program memory (in that flash) into the MRAM using a buffered copy
void write_cur_prog_mem_to_mram(struct spi_module* spi_master_instance,
	struct spi_slave_inst* mram_slave1, struct spi_slave_inst* mram_slave2) {
	size_t num_copied = 0;
	uint32_t flash_addr = (uint32_t) RAD_SAFE_FIELD_GET(app_start_address);
	uint32_t mram_addr = RAD_SAFE_FIELD_GET(mram_app_address);
	
	// disable write protection
	set_mram_protection(spi_master_instance, mram_slave1, mram_slave2, STATUS_REG_PROTECT_NONE);

	while (num_copied < RAD_SAFE_FIELD_GET(prog_mem_size)) {
		size_t buf_size = min(RAD_SAFE_FIELD_GET(prog_mem_size) - num_copied, RAD_SAFE_FIELD_GET(mram_copy_buffer_size));
		
		pet_watchdog(); // pet before long write
		
		// write this buffer section directly from the flash (program memory) address space into both MRAMs
		mram_write_bytes(spi_master_instance, mram_slave1, (uint8_t*) flash_addr, buf_size, mram_addr);
		mram_write_bytes(spi_master_instance, mram_slave2, (uint8_t*) flash_addr, buf_size, mram_addr);
		
		pet_watchdog(); // pet after long write
		
		// checks to confirm it matches program memory
		#if defined(RUN_ASSERTS) && defined(CONFIRM_PROG_MEM_MRAM_WRITE)
			mram_read_bytes(spi_master_instance, mram_slave1, buffer_mram1, buf_size, mram_addr);
			configASSERT(memcmp((uint8_t*) flash_addr, buffer_mram1, buf_size) == 0);
			mram_read_bytes(spi_master_instance, mram_slave2, buffer_mram1, buf_size, mram_addr); // use same buffer...
			configASSERT(memcmp((uint8_t*) flash_addr, buffer_mram1, buf_size) == 0);
			pet_watchdog(); // pet after long read
		#endif
		
		num_copied += buf_size;
		mram_addr += buf_size;
		flash_addr += buf_size;
	}
	
	// set write protection on the top half of the MRAM from writes
	set_mram_protection(spi_master_instance, mram_slave1, mram_slave2, STATUS_REG_PROTECT_TOP_HALF);
}

#endif



#ifdef RUN_TESTS

void mram_test(struct spi_module* spi_master_instance, struct spi_slave_inst* slave) {
	#define MRAM_TEST_ADDRESS 0x50000
	#define MRAM_NUM_BYTES	20
	uint8_t example_array[8] = {0x01, 0x02, 0x03, 0x04, 0xa5, 0xfc, 0xff, 0x42};
	uint8_t example_output_array_before[MRAM_NUM_BYTES];// = {0, 0, 0, 0, 0, 0, 0, 0};
	uint8_t example_output_array_after[MRAM_NUM_BYTES];// = {0, 0, 0, 0, 0, 0, 0, 0};

	uint8_t status_reg1 = 0xff;
	uint8_t status_reg2 = 0xff;
// 	mram_read_status_register(spi_master_instance, slave, &status_reg1);
// 	mram_read_bytes(spi_master_instance, slave, example_output_array_before, MRAM_NUM_BYTES, MRAM_TEST_ADDRESS);
// 	mram_write_bytes(spi_master_instance, slave, example_array, 8, MRAM_TEST_ADDRESS);
// 	mram_read_bytes(spi_master_instance, slave, example_output_array_after, MRAM_NUM_BYTES, MRAM_TEST_ADDRESS);

 
	/* protection test */
	uint32_t write_lock_test_addr = 0x51000;
	uint32_t write_lock_unlocked_test_addr = 0x10;
	uint8_t example_write_test_array1[8] = {0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12};
	uint8_t example_write_test_array2[8] = {0x10, 0x54, 0x97, 0x99, 0x23, 0x45, 0x56, 0x67};
	uint8_t example_write_test_array3[8] = {0x25, 0x26, 0x27, 0x28, 0x29, 0x30, 0x31, 0x32};
		
	// overwrite old for normal section test
	mram_read_bytes(spi_master_instance, slave, example_output_array_before, 8, write_lock_unlocked_test_addr);
	mram_write_bytes(spi_master_instance, slave, example_write_test_array2, 8, write_lock_unlocked_test_addr);
	mram_read_bytes(spi_master_instance, slave, example_output_array_after, 8, write_lock_unlocked_test_addr);
	configASSERT(memcmp(example_write_test_array2, example_output_array_after, 8) == 0);
	
	// test we can write it now
	mram_read_bytes(spi_master_instance, slave, example_output_array_before, 8, write_lock_test_addr);
	mram_write_bytes(spi_master_instance, slave, example_write_test_array1, 8, write_lock_test_addr);
	mram_read_bytes(spi_master_instance, slave, example_output_array_after, 8, write_lock_test_addr);
	configASSERT(memcmp(example_output_array_after, example_write_test_array1, 8) == 0);
		
	// lock upper half
	mram_read_status_register(spi_master_instance, slave, &status_reg1);
	mram_write_status_register(spi_master_instance, slave, STATUS_REG_PROTECT_TOP_HALF);
	mram_read_status_register(spi_master_instance, slave, &status_reg2);
	configASSERT(status_reg2 == 0x0a);
	
	// try and write in protected section
	mram_read_bytes(spi_master_instance, slave, example_output_array_before, 8, write_lock_test_addr);
	mram_write_bytes(spi_master_instance, slave, example_write_test_array2, 8, write_lock_test_addr);
	mram_read_bytes(spi_master_instance, slave, example_output_array_after, 8, write_lock_test_addr);
	configASSERT(memcmp(example_output_array_before, example_output_array_after, 8) == 0);
	
	// try and write in normal section
	mram_read_bytes(spi_master_instance, slave, example_output_array_before, 8, write_lock_unlocked_test_addr);
	mram_write_bytes(spi_master_instance, slave, example_write_test_array1, 8, write_lock_unlocked_test_addr);
	mram_read_bytes(spi_master_instance, slave, example_output_array_after, 8, write_lock_unlocked_test_addr);
	configASSERT(memcmp(example_write_test_array1, example_output_array_after, 8) == 0);
	
	// stop protecting
	mram_read_status_register(spi_master_instance, slave, &status_reg1);
	mram_write_status_register(spi_master_instance, slave, STATUS_REG_PROTECT_NONE);
	mram_read_status_register(spi_master_instance, slave, &status_reg2);
	configASSERT(status_reg2 == 0x02);

	// try and write now
	mram_read_bytes(spi_master_instance, slave, example_output_array_before, 8, write_lock_test_addr);
	mram_write_bytes(spi_master_instance, slave, example_write_test_array3, 8, write_lock_test_addr);
	mram_read_bytes(spi_master_instance, slave, example_output_array_after, 8, write_lock_test_addr);
	configASSERT(memcmp(example_output_array_after, example_write_test_array3, 8) == 0);

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

// sets num bytes starting at start to random values
void set_random_bytes(uint8_t* start, size_t num) {
	for (size_t i = 0; i < num; i++) {
		start[i] = rand();
	}
}

void corrupt_prog_mem(void) {
	#define NUM_RAND_SEQS			4
	#define RAND_SEQ_LEN			1
	#define INSERT_POINT_STEP_AVG	(RAD_SAFE_FIELD_GET(prog_mem_size) / NUM_RAND_SEQS)

	uint32_t insert_ptr = (uint32_t) RAD_SAFE_FIELD_GET(app_start_address);
	while (true) {
		// randomly shift sequence insertion pointer
		// (between 0x and 2x increase relative to INSERT_POINT_STEP_AVG)
		uint32_t rand_step = ((rand() % 200) * INSERT_POINT_STEP_AVG) / 100;
		insert_ptr += rand_step;

		// correct the random to change to be aligned with flash page (we have to)
		insert_ptr -= insert_ptr % NVMCTRL_PAGE_SIZE;

		// make sure not overflowing
		if (insert_ptr > (uint32_t) (RAD_SAFE_FIELD_GET(app_start_address) + RAD_SAFE_FIELD_GET(prog_mem_size))) {
			break;
		}

		// generate and write random bytes
		uint8_t seq_buf[RAND_SEQ_LEN];
		set_random_bytes(seq_buf, RAND_SEQ_LEN);
		flash_mem_write_bytes(seq_buf, RAND_SEQ_LEN, insert_ptr);
	}
}

#endif
