#include "Radio_Commands.h"
#include "../processor_drivers/Direct_Pin_Commands.h"

char dealer_response[4] = {1, 196, 0, 59};
char txFreq_response[4] = {1, 183, 0, 72};
char rxFreq_response[4] = {1, 185, 0, 70};
char channel_response[4] = {0x01, 0x83, 0x00, 0x7c};
char warmReset_response[4] = {0x01, 0x9d, 0x00, 0x62};

int working = 1;

void setCommandMode(void) {
	usart_send_string("+++");
	delay_ms(300);
}

void setDealerMode(void) {
	sendbuffer[0] = 0x01;
	sendbuffer[1] = 0x44;
	sendbuffer[2] = 0x01;
	sendbuffer[3] = ~0x45;
	sendbuffer[4] = '\0';
	usart_send_string(sendbuffer);
}

void setTxFreq() {
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

void setRxFreq(void) {
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

void setChannel(void) {
	//index 2 is byte to set channel
	sendbuffer[0] = 0x01;
	sendbuffer[1] = 0x03;
	sendbuffer[2] = 0x01;
	sendbuffer[3] = 0xFB;
	sendbuffer[4] = '\0';
	usart_send_string(sendbuffer);
}

void warmReset(void){
	sendbuffer[0] = 0x01;
	sendbuffer[1] = 0x1d;
	sendbuffer[2] = 0x01; //warm
	sendbuffer[3] = ~0x1E;
	sendbuffer[4] = '\0';
	usart_send_string(sendbuffer);
}

void setModulationFormat(void){
	sendbuffer[0] = 0x01;
	sendbuffer[1] = 0x2B;
	sendbuffer[2] = 0x01;
	sendbuffer[3] = ~0x2C;
	sendbuffer[4] = '\0';
	usart_send_string(sendbuffer);
}

void setLinkSpeed(void){
	sendbuffer[0] = 0x01;
	sendbuffer[1] = 0x05;
	sendbuffer[2] = 0x03;
	sendbuffer[3] = ~0x08;
	sendbuffer[4] = '\0';
	usart_send_string(sendbuffer);
}

unsigned char calculateChecksum(char* data, int dataLen) {
	unsigned char checksum = 0;
	for (int i = 0; i<dataLen; i++) checksum = (checksum + data[i]) & 0xFF;
	return ~checksum;
}

int responseCheck(char arr[]){
	if(working==0) return 0; //not gonna work anymore pal

	for(int i=0;i<sizeof(arr)/sizeof(arr[0]);i++){
		if(receivebuffer[i]!=arr[i]) return 0;
	}
	return 1;
}

void setSendEnable(bool level) {
	set_output(level, PIN_PA02);
}

void setReceiveEnable(bool level) {
	set_output(level, PIN_PA03);
}

void initializeRadio() {
	setup_pin(true,PIN_PB02); //init send enable pin
	setup_pin(true,PIN_PB03); //init receive enable pin
	
	setCommandMode();
	delay_ms(100); //remember to put delay, because the MCU is faster than the USART
	
	setDealerMode();
	delay_ms(100);
	working = responseCheck(dealer_response);	
	
	setTxFreq();
	delay_ms(100);
	working = responseCheck(txFreq_response);	
	
	setRxFreq();
	delay_ms(100);
	working = responseCheck(rxFreq_response);	
	
	setModulationFormat();
	delay_ms(400);	
	
	setLinkSpeed();
	delay_ms(100);	
	
	setChannel();
	delay_ms(500); //longer delay because radio is from the 90s and needs a smoke break
	working = responseCheck(channel_response);	
	
	warmReset();
	delay_ms(500); //longer delay because radio is from the 90s and needs a smoke break
	working = responseCheck(warmReset_response);	
}