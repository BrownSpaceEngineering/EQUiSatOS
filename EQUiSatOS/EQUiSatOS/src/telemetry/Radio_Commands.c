#include "Radio_Commands.h"

char ground_callsign_buf[] = {'K', '1', 'A', 'D'};
char echo_buf[] = {'E', 'C'};
char kill_3days_buf[] = {'K', '1'};
char kill_week_buf[] = {'K', '2'};
char kill_forever_buf[] = {'K', '3'};
char flash_buf[] = {'F', 'L'};
char reboot_buf[] = {'R', 'E'};

char dealer_response[4] = {1, 196, 0, 59};
char txFreq_response[4] = {1, 183, 0, 72};
char rxFreq_response[4] = {1, 185, 0, 70};
char channel_response[4] = {0x01, 0x83, 0x00, 0x7c};
char warmReset_response[4] = {0x01, 0x9d, 0x00, 0x62};

int working = 1;

void radio_init(void) {
	// USART_init() should be called as well (above)
	setup_pin(true, P_RAD_PWR_RUN); //3v6 enable
	setup_pin(true, P_RAD_SHDN); //init shutdown pin
	setup_pin(true, P_TX_EN); //init send enable pin
	setup_pin(true, P_RX_EN); //init receive enable pin
}

bool check_checksum(char* data, uint8_t dataLen, uint8_t actualChecksum) {
	uint8_t checksum = 0;
	for (uint8_t i = 0; i<dataLen; i++) {
		checksum = (checksum + data[i]) & 0xFF;
	}
	checksum = ~checksum;
	return (checksum == actualChecksum);
}

void set_command_mode(bool delay) {
	if (delay) delay_ms(SET_CMD_MODE_WAIT_BEFORE_MS);
	usart_send_string((uint8_t*) "+++");
	if (delay) delay_ms(SET_CMD_MODE_WAIT_AFTER_MS);
}

bool check_if_rx_matches(char* buf, uint8_t len, uint8_t rx_buf_index) {
	for (int i = 0; i < len; i++) {
		if (buf[i] != radio_receive_buffer[rx_buf_index % LEN_RECEIVEBUFFER]) {
			return false;
		}		
		rx_buf_index++;
	}
	return true;
}

// checks if an RX command was received, and signals the transmit
// task if so. MUST be called from an interrupt while doing so.
rx_cmd_type_t check_rx_received(void) {
	//checking for callsign
	rx_cmd_type_t command = CMD_NONE;
	uint8_t i = 0;	
	while (i < LEN_RECEIVEBUFFER) {
		bool callsign_valid = true;
		for (int j = 0; j < LEN_GROUND_CALLSIGN; j++) {
			if (radio_receive_buffer[(i+j) % LEN_RECEIVEBUFFER] != ground_callsign_buf[j]) {
				callsign_valid = false;
				break;
			}
		}
		if (callsign_valid) {
			uint8_t rxbuf_cmd_start_index = (i+4) % LEN_RECEIVEBUFFER;
			// if valid command, send to transmit task to handle when ready			
			if (check_if_rx_matches(echo_buf, LEN_UPLINK_BUF, rxbuf_cmd_start_index)) {
				//ECHO
				command = CMD_ECHO;
				//xQueueSendFromISR(rx_command_queue, &command, NULL);								
			} else if (check_if_rx_matches(flash_buf, LEN_UPLINK_BUF, rxbuf_cmd_start_index)) {
				//FLASH
				command = CMD_FLASH;
				//xQueueSendFromISR(rx_command_queue, &command, NULL);				
			} else if (check_if_rx_matches(reboot_buf, LEN_UPLINK_BUF, rxbuf_cmd_start_index)) {
				//REBOOT
				command = CMD_REBOOT;
				//xQueueSendFromISR(rx_command_queue, &command, NULL);				
			}  else if (check_if_rx_matches(kill_3days_buf, LEN_UPLINK_BUF, rxbuf_cmd_start_index)) {
				//KILL FOR 3 DAYS
				command = CMD_KILL_3DAYS;
				//xQueueSendFromISR(rx_command_queue, &command, NULL);					
			} else if (check_if_rx_matches(kill_week_buf, LEN_UPLINK_BUF, rxbuf_cmd_start_index)) {
				//KILL FOR 1 WEEK
				command = CMD_KILL_WEEK;
				//xQueueSendFromISR(rx_command_queue, &command, NULL);				
			} else if (check_if_rx_matches(kill_forever_buf, LEN_UPLINK_BUF, rxbuf_cmd_start_index)) {
				//KILL FOREVER :'(
				command = CMD_KILL_FOREVER;
				//xQueueSendFromISR(rx_command_queue, &command, NULL);				
			}			
		}
		i++;
	}
	return command;
}

/*bool send_command(int numBytesReceiving) {
	clear_USART_rx_buffer();
	waitingForData = true;
	receiveDataReady = false;
	expectedReceiveDataLen = numBytesReceiving;
	usart_send_string(sendbuffer);
	int i = 0;
	while (!receiveDataReady && i < 20) {
		delay_ms(20);
		i++;
	}
	return receiveDataReady;
}

void set_dealer_mode(void) {
	sendbuffer[0] = 0x01;
	sendbuffer[1] = 0x44;
	sendbuffer[2] = 0x01;
	sendbuffer[3] = ~0x45;
	sendbuffer[4] = '\0';
	usart_send_string(sendbuffer);
}

void set_tx_freq(void) {
	//index 3-6 is 4 byte frequency in Hz
	sendbuffer[0] = 0x01;
	sendbuffer[1] = 0x37;
	sendbuffer[2] = 0x01; //channel 1
	sendbuffer[3] = 0x1A;
	sendbuffer[4] = 0x0C;
	sendbuffer[5] = 0x17;
	sendbuffer[6] = 0x40;
	sendbuffer[7] = ~0xB5;
	sendbuffer[8] = '\0';
	usart_send_string(sendbuffer);
}

void set_rx_freq(void) {
	//index 3-6 is 4 byte frequency in Hz
	sendbuffer[0] = 0x01;
	sendbuffer[1] = 0x39;
	sendbuffer[2] = 0x01; //channel 1
	sendbuffer[3] = 0x1A;
	sendbuffer[4] = 0x0C;
	sendbuffer[5] = 0x17;
	sendbuffer[6] = 0x40;
	sendbuffer[7] = ~0xB7;
	sendbuffer[8] = '\0';
	usart_send_string(sendbuffer);
}

void set_channel(void) {
	//index 2 is byte to set channel
	sendbuffer[0] = 0x01;
	sendbuffer[1] = 0x03;
	sendbuffer[2] = 0x01;
	sendbuffer[3] = 0xFB;
	sendbuffer[4] = '\0';
	usart_send_string(sendbuffer);
}

void cold_reset(void){
	sendbuffer[0] = 0x01;
	sendbuffer[1] = 0x1d;
	sendbuffer[2] = 0x00; //cold
	sendbuffer[3] = ~0x1d;
	sendbuffer[4] = '\0';
	usart_send_string(sendbuffer);
	delay_ms(200);
}

void set_modulation_format(void){
	sendbuffer[0] = 0x01;
	sendbuffer[1] = 0x2B;
	sendbuffer[2] = 0x01;
	sendbuffer[3] = ~0x2C;
	sendbuffer[4] = '\0';
	usart_send_string(sendbuffer);
}

void set_link_speed(void){
	sendbuffer[0] = 0x01;
	sendbuffer[1] = 0x05;
	sendbuffer[2] = 0x03;
	sendbuffer[3] = ~0x08;
	sendbuffer[4] = '\0';
	usart_send_string(sendbuffer);
}


int response_check(char arr[]){
	if(working==0) return 0; //not gonna work anymore pal

	for(int i=0;i<sizeof(arr)/sizeof(arr[0]);i++){
		if(receivebuffer[i]!=arr[i]) return 0;
	}
	return 1;
}*/

/*Returns 16 bit temp reading in 1/10 degree Celsius)*/
void XDL_prepare_get_temp(void) {
	radio_send_buffer[0] = 0x01;
	radio_send_buffer[1] = 0x50;
	radio_send_buffer[2] = ~0x50;
	radio_send_buffer[3] = '\0';
}

void warm_reset(void){
	radio_send_buffer[0] = 0x01;
	radio_send_buffer[1] = 0x1d;
	radio_send_buffer[2] = 0x01; //warm
	radio_send_buffer[3] = ~0x1E;
	radio_send_buffer[4] = '\0';
}

/*Controls TX buffer between proc TX and radio.*/
void setTXEnable(bool enable) {
	//Invert because active low to open
	set_output(!enable, P_TX_EN);
}

/*Controls TX buffer between proc TX and radio.*/
void setRXEnable(bool enable) {
	//Invert because active low to open
	set_output(!enable, P_RX_EN);
}

/************************************************************************/
/* RTOS-Integrated Helpers                                              */
/************************************************************************/

/* transmits the buffer of given size over the radio USART,
	then waits the expected transmit time to emulate an atomic operation */
void transmit_buf_wait(const uint8_t* buf, size_t size) {
	#if defined(SAFE_PRINT) && (PRINT_DEBUG == 1 || PRINT_DEBUG == 3)
		// take this for a shorter time than normal to not mess up RTOS much
		xSemaphoreTakeRecursive(print_mutex, 200 / portTICK_PERIOD_MS);
	#endif
	
	#ifdef DONT_PRINT_RAW_TRANSMISSIONS
		print("Transmitted %d bytes\n", size);
	#endif
	
	// note the time of transmission so we can make sure we only delay 
	// for the actual transmission time
	TickType_t transmission_start_ticks;
	
	// enable IR power and wait if necessary (it'll be safe (consistent) 
	// if we have the mutex, otherwise transmission is so crucial we don't care)
	// we're also taking the I2C mutex here so we're less likely to 
	// get delayed later when verifying radio currents
	enable_ir_pow_if_necessary_unsafe();
 	bool got_i2c_irpow_mutex = true;
	if (!xSemaphoreTake(i2c_irpow_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS)) {
		log_error(ELOC_RADIO, ECODE_I2C_IRPOW_MUTEX_TIMEOUT, true);
		got_i2c_irpow_mutex = false;
	}
	{	
		// now that we may have the mutex, try and enable IR power again
		// in case it got shut off in the period between us first turning it
		// on and getting the mutex (it should really be on)
		enable_ir_pow_if_necessary_unsafe();
		
		// suspend the scheduler to make sure the whole buf is sent atomically
		// (so the radio gets the full buf at once and doesn't cut out in the middle)	
		pet_watchdog(); // in case this takes a bit and we're close to reset	
		vTaskSuspendAll();
		{
			#if defined(TRANSMIT_ACTIVE)
				setTXEnable(true);
			#endif
			#if defined(TRANSMIT_ACTIVE) || !defined(DONT_PRINT_RAW_TRANSMISSIONS)
				transmission_start_ticks = xTaskGetTickCount();
				trace_print("transmitting...");
				usart_send_buf(buf, size);
			#endif
			#if PRINT_DEBUG == 1 || PRINT_DEBUG == 3
				delay_ms(10); // TODO
				setTXEnable(false);
			#endif
		}
		xTaskResumeAll();
		
		// we don't care too much here if the mutex times out; it's just for confirmations
		bool got_hw_state_mutex = true;
		if (!hardware_state_mutex_take()) {
			log_error(ELOC_RADIO, ECODE_HW_STATE_MUTEX_TIMEOUT, true);
			got_hw_state_mutex = false;
		}
		
		// wait duration for transmission to start and current to rise before verifying
		// NOTE this increments transmission_start_ticks so it equals the time after this returns (for later)
		vTaskDelayUntil(&transmission_start_ticks, TRANSMIT_CURRENT_RISE_TIME_MS / portTICK_PERIOD_MS);
		
		// only set as transmitting after enought time that we expect the current should be up
		get_hw_states()->radio_state = RADIO_TRANSMITTING;
		// we only give the hw mutex here because we can't inside the scheduler suspended zone
		// (also verify regulators needs it)
		if (got_hw_state_mutex) hardware_state_mutex_give();
		
		if (got_i2c_irpow_mutex) {
			// if we got the mutex we can safely do a fast call to the unsafe method
			verify_regulators_unsafe();
			trace_print("verified regulators");
		
			// if we got the mutex, we can safely turn off IR power. Otherwise, 
			// the best we can do is see if we can turn it off now.
			// we don't care if we turned on IR power (this is mainly used in sensor read functions
			// so external callers can avoid waits)
			disable_ir_pow_if_necessary_unsafe(true);
			xSemaphoreGive(i2c_irpow_mutex);
		} else {
			// if we didn't get the mutex, still try and verify the regulators (the slower way)
			verify_regulators();
			trace_print("verified regulators");
		
			// if we didn't get the IR power mutex, the best we can do 
			// is see if we can turn it off now
			disable_ir_pow_if_should_be_off(true); // expected on bcs. we turned it on
		}
	}
	
	// delay for a total time of the transmission duration, STARTING at when we verified
	// regulators (see above)
	vTaskDelayUntil(&transmission_start_ticks, TRANSMIT_TIME_MS(size) / portTICK_PERIOD_MS);	

	// note hardware state change, ignoring mutex timeout
	if (hardware_state_mutex_take()) {
		get_hw_states()->radio_state = RADIO_IDLE;
		hardware_state_mutex_give();
	} else {
		get_hw_states()->radio_state = RADIO_IDLE;
		log_error(ELOC_RADIO, ECODE_HW_STATE_MUTEX_TIMEOUT, true);
	}
	
	#if defined(SAFE_PRINT) && (PRINT_DEBUG == 1 || PRINT_DEBUG == 3)
		xSemaphoreGiveRecursive(print_mutex);
	#endif
}

/* high-level function to bring radio systems online and check their state */
void setRadioState(bool enable, bool confirm) {
	// keep hardware state mutex while state is indeterminate
	hardware_state_mutex_take();
	{
		// enable / disable 3V6 regulator and radio power at the same time
		set_output(enable, P_RAD_PWR_RUN);
		set_output(enable, P_RAD_SHDN);
		#if PRINT_DEBUG == 0
			setTXEnable(enable);
			setRXEnable(enable);
		#endif

		if (confirm) {
			activate_ir_pow(); // avoid IR power on delay for speed
			vTaskDelay(max(REGULATOR_ENABLE_WAIT_AFTER_MS, IR_WAKE_DELAY_MS) / portTICK_PERIOD_MS);	
		} else {
			vTaskDelay(REGULATOR_ENABLE_WAIT_AFTER_MS / portTICK_PERIOD_MS);
		}
		get_hw_states()->radio_state = enable ? RADIO_IDLE : RADIO_OFF;
	}
	hardware_state_mutex_give();
	if (confirm) {
		verify_regulators(); // will log error if regulator not valid
		disable_ir_pow_if_should_be_off(true);
	}
}
