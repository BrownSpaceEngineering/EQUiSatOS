#include "Radio_Commands.h"

char dealer_response[4] = {1, 196, 0, 59};
char txFreq_response[4] = {1, 183, 0, 72};
char rxFreq_response[4] = {1, 185, 0, 70};
char channel_response[4] = {0x01, 0x83, 0x00, 0x7c};
char warmReset_response[4] = {0x01, 0x9d, 0x00, 0x62};

int working = 1;

void set_command_mode(void) {
	delay_ms(150);
	usart_send_string((uint8_t*) "+++");
	delay_ms(150);
}

/*void set_dealer_mode(void) {
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
}*/

void warm_reset(void){
	sendbuffer[0] = 0x01;
	sendbuffer[1] = 0x1d;
	sendbuffer[2] = 0x01; //warm
	sendbuffer[3] = ~0x1E;
	sendbuffer[4] = '\0';
	usart_send_string(sendbuffer);
}

/*void set_modulation_format(void){
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
}*/


int response_check(char arr[]){
	if(working==0) return 0; //not gonna work anymore pal

	for(int i=0;i<sizeof(arr)/sizeof(arr[0]);i++){
		if(receivebuffer[i]!=arr[i]) return 0;
	}
	return 1;
}

void get_temperature(void){
	sendbuffer[0] = 0x01;
	sendbuffer[1] = 0x50;
	sendbuffer[2] = ~0x50;
	sendbuffer[3] = '\0';
	usart_send_string(sendbuffer);
}

unsigned char calculate_checksum(char* data, int dataLen) {
	unsigned char checksum = 0;
	for (int i = 0; i<dataLen; i++) checksum = (checksum + data[i]) & 0xFF;
	return ~checksum;
}

/*Returns 16 bit temp reading in 1/10 degree Celsius)*/
uint16_t XDL_get_temperature() {
	set_command_mode();
	get_temperature();
	delay_ms(200);
	//TODO: Check packet validity and extract data
	if (calculate_checksum(receivebuffer+1, 2) == receivebuffer[3]) {
		uint16_t radioTemp = (receivebuffer[1] << 8) | receivebuffer[2];
		clear_USART_rx_buffer();
		warm_reset();
		delay_ms(500);
		return radioTemp;
	} else {
		//maybe wait a little longer? or reset radio?
		//TODO: Log error: couldn't get temp from radio
	}	
}

void setTXEnable(bool enable) {
	set_output(enable, P_TX_EN);
}

void setRXEnable(bool enable) {
	set_output(enable, P_RX_EN);
}

void set3V6Power(bool on) {
	set_output(on, P_RAD_PWR_RUN);
}

void setRadioPower(bool on) {
	set_output(on, P_RAD_SHDN);
}