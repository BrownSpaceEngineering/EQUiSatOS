/*
 * bat_testing.c
 *
 * Created: 10/26/2017 3:26:26 PM
 *  Author: Ryan Izant
 */ 
#include "bat_testing.h"

void bat_testing_init(){
	delay_init();
	configure_i2c_standard(SERCOM4);
	
	LTC1380_init();
	AD7991_init();
	return_struct_16 tca9535_rs;
	TCA9535_init(tca9535_rs);
	
	setup_pin(true,P_5V_EN);
	set_output(true, P_5V_EN); //init gpio pin for 5V regulator enable
	
}

//Reads all sensors from Battery Board remote ADC and returns in human readable voltage
void readRemoteADC_0(float* batReadings){
	uint16_t remoteADC[4];
	AD7991_read_all(remoteADC, AD7991_ADDR_0);
	batReadings[0] = (((float) remoteADC[0])/4096*3.3-1)*2000;// mA
	batReadings[1] = (((float) remoteADC[1])/4096*3.3-1)*2000;// mA
	batReadings[2]= ((float)  remoteADC[2])/4096*3.3*2.717;//V
	batReadings[3] = ((float) remoteADC[3])/4096*3.3*5.58;//V
}



void bat_testing_run(){
	
	for (int i=0; i<100; i++){
		for (int j=4 )
	}
}