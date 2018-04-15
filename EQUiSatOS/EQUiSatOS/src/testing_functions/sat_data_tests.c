/*
 * sat_data_tests.c
 *
 * Created: 1/18/2018 23:13:08
 *  Author: mcken
 */ 
#include "sat_data_tests.h"

uint8_t msg_buffer[MSG_BUFFER_SIZE]; // don't store on stack of testing functions


// returns the hex string representation of the given nibble
static char get_hex(uint8_t nibble) {
	if (nibble < 10) {
		return '0' + nibble;
	} else if (nibble <= 16) {
		return 'a' + (nibble - 10);
	} else {
		return '!';
	}
}

static void print_buf_hex(uint8_t* buf, size_t len) {
	for (uint i = 0; i < len; i++) {
		char lsb_nibble = get_hex(buf[i] & 0xf);
		char msg_nibble = get_hex(buf[i] >> 4);
		print("%c%c", msg_nibble, lsb_nibble);
	}
}

static void fill_random_data(uint8_t* data, int num) {
	for (int i = 0; i < num; i++) {
		data[i] = rand() % 256;
	}
}

void populate_equistack(equistack* stack) {
	uint8_t* data = equistack_Initial_Stage(stack);
	for (int i = 0; i <= stack->max_size; i++) {
		fill_random_data(data, stack->data_size);
		data = equistack_Stage(stack);
	}
}

// fills all data equistacks with random data
void populate_equistacks(void) {
	populate_equistack(&idle_readings_equistack);
	populate_equistack(&attitude_readings_equistack);
	populate_equistack(&flash_readings_equistack);
	populate_equistack(&flash_cmp_readings_equistack);
	populate_equistack(&low_power_readings_equistack);
}

void clear_equistacks(void) {
	__equistack_Clear(&idle_readings_equistack);
	__equistack_Clear(&attitude_readings_equistack);
	__equistack_Clear(&flash_readings_equistack);
	__equistack_Clear(&flash_cmp_readings_equistack);
	__equistack_Clear(&low_power_readings_equistack);
}

// fills all data equistacks and then tests packaging that data
// using the transmission packaging code
void test_message_packaging(void) {
	populate_equistacks();
	
	uint32_t current_timestamp = get_current_timestamp();
	uint8_t cur_data_buf[MSG_CUR_DATA_LEN];
	read_current_data(cur_data_buf, current_timestamp);
	write_packet(msg_buffer, IDLE_DATA, current_timestamp, cur_data_buf);
	write_packet(msg_buffer, ATTITUDE_DATA, current_timestamp, cur_data_buf);
	write_packet(msg_buffer, FLASH_DATA, current_timestamp, cur_data_buf);
	write_packet(msg_buffer, FLASH_CMP_DATA, current_timestamp, cur_data_buf);
	write_packet(msg_buffer, LOW_POWER_DATA, current_timestamp, cur_data_buf);
}

void stress_test_message_packaging(void) {
	
	/* Test with empty equistacks */
	// NOTE: the priority of the task running this must be high enough so nothing is written in this period!!!
	clear_equistacks();
	__equistack_Clear(&error_equistack);
	
	uint32_t current_timestamp = get_current_timestamp();
	uint8_t cur_data_buf[MSG_CUR_DATA_LEN];
	read_current_data(cur_data_buf, current_timestamp);
	write_packet(msg_buffer, IDLE_DATA, current_timestamp, cur_data_buf);
	write_packet(msg_buffer, ATTITUDE_DATA, current_timestamp, cur_data_buf);
	write_packet(msg_buffer, FLASH_DATA, current_timestamp, cur_data_buf);
	write_packet(msg_buffer, FLASH_CMP_DATA, current_timestamp, cur_data_buf);
	write_packet(msg_buffer, LOW_POWER_DATA, current_timestamp, cur_data_buf);
}

static void print_transmission_info(msg_data_type_t type, uint32_t current_timestamp, uint8_t* cur_data_buf) {
	print("timestamp: \t%d\n", current_timestamp);
	print("sat state: \t%s\n", get_sat_state_str(get_sat_state()));
	print("reboot #:  \t%d\n", cache_get_reboot_count());
	print("num errors:\t%d\n", error_equistack.cur_size);
	print_cur_data_buf(cur_data_buf);
	print_equistack(&error_equistack, print_sat_error, "Error Stack", -1);
	print("\nnote: there's more information here than is in the packet (look for non-transmitted packets)\n");
	switch (type) {
		case IDLE_DATA:
			print_equistack(&idle_readings_equistack,		print_idle_data,		"Idle Data Stack", -1);
			return;
		case ATTITUDE_DATA:
			print_equistack(&attitude_readings_equistack,	print_attitude_data,	"Attitude Data Stack", -1);
			return;
		case FLASH_DATA:
			print_equistack(&flash_readings_equistack,		print_flash_data,		"Flash Data Stack", -1);
			return;
		case FLASH_CMP_DATA:
			print_equistack(&flash_cmp_readings_equistack,	print_flash_cmp_data,	"Flash Cmp Data Stack", -1);
			return;
		case LOW_POWER_DATA:
			print_equistack(&low_power_readings_equistack,	print_low_power_data,	"Low Power Data Stack", -1);
			return;
		default: return;
	}
}

// print high level information on the given message
void print_sample_transmission(uint8_t* msg_buf, msg_data_type_t type, uint32_t current_timestamp, uint8_t* cur_data_buf) {
	print("=========Sample Transmission=========\n");
	print("type: %s\n", get_msg_type_str(type));
	print("\n----Data Summary----\n");
	print_transmission_info(type, current_timestamp, cur_data_buf);
	print("\n----Raw Hex Data----\n\n");
	print_buf_hex(msg_buf, MSG_SIZE);
	print("\n\n=======End Sample Transmission=======\n");
}

static void generate_print_sample_transmission(msg_data_type_t type, uint32_t current_timestamp, uint8_t* cur_data_buf) {
	write_packet(msg_buffer, type, current_timestamp, cur_data_buf);
	print_sample_transmission(msg_buffer, type, current_timestamp, cur_data_buf);
}


void print_sample_transmissions(void) {
	suppress_other_prints(true);
	uint32_t current_timestamp = get_current_timestamp();
	uint8_t cur_data_buf[MSG_CUR_DATA_LEN];
	read_current_data(cur_data_buf, current_timestamp);
	generate_print_sample_transmission(IDLE_DATA, current_timestamp, cur_data_buf);
	generate_print_sample_transmission(ATTITUDE_DATA, current_timestamp, cur_data_buf);
	generate_print_sample_transmission(FLASH_DATA, current_timestamp, cur_data_buf);
	generate_print_sample_transmission(FLASH_CMP_DATA, current_timestamp, cur_data_buf);
	generate_print_sample_transmission(LOW_POWER_DATA, current_timestamp, cur_data_buf);
	suppress_other_prints(false);
}

