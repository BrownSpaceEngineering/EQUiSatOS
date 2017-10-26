/*
 * bat_testing.c
 *
 * Created: 10/26/2017 3:26:26 PM
 *  Author: Ryan Izant
 */ 

void bat_testing_init(){
	delay_init();
	configure_i2c_standard(SERCOM4);
	
	LTC1380_init();
	AD7991_init();
	TCA9535_init();
	
	setup_pin(true,P_5V_EN);
	set_output(true, P_5V_EN); //init gpio pin for 5V regulator enable
	
}

void readRemoteADC_0(float* batReadings){
	uint16_t remoteADC[4];
	AD7991_read_all(remoteADC, AD7991_ADDR_0);
	batReadings[0] = ((float) remoteADC[0])/4096*3.3;//TBD
	batReadings[1] = ((float) remoteADC[1])/4096*3.3;//TBD
	batReadings[2]= ((float)  remoteADC[2])/4096*3.3;//TBD
	batReadings[3] = ((float) remoteADC[3])/4096*3.3;//TBD
}



void bat_testing_run(){
	
}