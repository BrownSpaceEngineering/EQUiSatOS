/*
 * Radio_Commands.c
 *
 * Created: 9/20/2016 9:49:53 PM
 *  Author: Tyler
 */ 
#include "Radio_Commands.h"


void setCommandMode() {
	usart_send_string("+++");
	delay_ms(300);
}

void setDealerMode() {
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

void setRxFreq() {
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

void setChannel() {
	//index 2 is byte to set channel
	sendbuffer[0] = 0x01;
	sendbuffer[1] = 0x03;
	sendbuffer[2] = 0x01;
	sendbuffer[3] = 0xFB;
	sendbuffer[4] = '\0';
	usart_send_string(sendbuffer);
}

void warmReset(){
	sendbuffer[0] = 0x01;
	sendbuffer[1] = 0x1d;
	sendbuffer[2] = 0x01; //warm
	sendbuffer[3] = ~0x1E;
	sendbuffer[4] = '\0';
	usart_send_string(sendbuffer);
}

void setModulationFormat(){
	sendbuffer[0] = 0x01;
	sendbuffer[1] = 0x2B;
	sendbuffer[2] = 0x01;
	sendbuffer[3] = ~0x2C;
	sendbuffer[4] = '\0';
	usart_send_string(sendbuffer);
}

void setLinkSpeed(){
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

void initializeRadio() {
	setCommandMode();
	delay_ms(100); //remember to put delay, because the MCU is faster than the USART
	
	setDealerMode();
	delay_ms(100);
	working = responseCheck(dealer_response);
	print(receivebuffer);
	
	
	setTxFreq();
	delay_ms(100);
	working = responseCheck(txFreq_response);
	print(receivebuffer);
	
	setRxFreq();
	delay_ms(100);
	working = responseCheck(rxFreq_response);
	print(receivebuffer);
	
	setModulationFormat();
	delay_ms(400);
	print(receivebuffer);
	
	setLinkSpeed();
	delay_ms(100);
	print(receivebuffer);
	
	setChannel();
	delay_ms(500); //longer delay because radio is from the 90s and needs a smoke break
	working = responseCheck(channel_response);
	print(receivebuffer);
	
	warmReset();
	delay_ms(500); //longer delay because radio is from the 90s and needs a smoke break
	working = responseCheck(warmReset_response);
	print(receivebuffer);
	
}