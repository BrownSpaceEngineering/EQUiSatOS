#include "flatsat.h"

struct adc_module temp_instance;
struct adc_module pd_instance;
uint8_t magReadBuff[6] = {0, 0, 0, 0, 0, 0};
int16_t xyzBuff[3] = {0, 0, 0};

struct adc_module modules[LEN_ADC];

struct adc_module adc_instance;

int num_samples = 10;

enum adc_positive_input adc_pins[LEN_ADC] = {
P_AI_LED1SNS,
P_AI_LED2SNS,
P_AI_LED3SNS,
P_AI_LED4SNS,
P_AI_LFB1OSNS,
P_AI_LFB1SNS,
P_AI_LFB2OSNS,
P_AI_LFB2SNS,
P_AI_LF1REF,
P_AI_LF2REF,
P_AI_LF3REF,
P_AI_LF4REF,
P_AI_L1_REF,
P_AI_L2_REF,
};

//TODO: FILL WITH SENSOR LIST, make LEN_IR the list length
uint8_t irs[LEN_IR] = {
	MLX90614_FLASHPANEL_V6_2_1,
	MLX90614_TOPPANEL_V5_1,
	MLX90614_ACCESSPANEL_V4_7,
	MLX90614_SIDEPANEL_V5_2,
	MLX90614_SIDEPANEL_V5_5,
	MLX90614_RBFPANEL_V1_4
};
	
void flatsat_init(void) {
	
	delay_init();
	
	//adc_regular_ain_channel(adc_pins,LEN_ADC);
	
	configure_i2c_standard(SERCOM4);
	//delay_ms(100); // delay after setting up i2c cuz ryan is suspicious 
	
	//configure_adc(&adc_instance,adc_pins[0]);
	
	//temperature
	//configure_adc(&temp_instance,P_AI_TEMP_OUT);
	//pd
	//configure_adc(&pd_instance, P_AI_PD_OUT);
	
	//other adc
	
	//for (int i=0; i<LEN_ADC; i++){
		//configure_adc(&modules[i], adc_pins[i]);
	//}
	
	MLX90614_init();
	
	//MPU9250_init();
	
	setup_pin(true,P_LED_CMD);
	set_output(true, P_LED_CMD); //init gpio pin for flashing
	
	setup_pin(true,P_5V_EN);
	set_output(true, P_5V_EN); //init gpio pin for 5V regulator enable
	
	//configure_watchdog();
	LTC1380_init();
	AD7991_init(AD7991_BATBRD);
	AD7991_init(AD7991_CTRLBRD);
	///setup_switching();
	//TCA9535_init();
}

//Cycles through temperatures sensor and photodiode external muxes and does 10x software averaging on each channel
static void readMuxs(uint16_t* tempBuffer, uint16_t* pdBuffer){
	 
	 for (int i=0; i<8; i++){
		 
		 uint8_t rs;
		 LTC1380_channel_select(0x48, i, &rs);
		 
		 uint16_t buf;
		 readFromADC(P_AI_TEMP_OUT,num_samples,&buf);
		 tempBuffer[i] = buf;
	 }
	 
	 for (int i=0; i<6; i++){
		uint8_t rs;
		LTC1380_channel_select(0x4a, i, &rs); 

		uint16_t buf;
		readFromADC(P_AI_PD_OUT,num_samples,&buf);
		pdBuffer[i] = buf;
	 }
 }
 
 //Cycles through all of the ADC channels that are not externally mux'd
static void readOtherADC(uint16_t* buffer){
	 for (int i=0; i<LEN_ADC; i++){
		 uint16_t buf;
		 readFromADC(adc_pins[i], num_samples,&buf);
		 buffer[i] = buf;
	 }
 }
 


void read_IR(uint16_t* buffer) {
	for (int i = 0; i < LEN_IR; i++)
	{
		uint16_t rs;
		MLX90614_read2ByteValue(irs[i], MLX90614_TOBJ1, &rs);
		buffer[i] = rs;
	}
}

void sendToArduino(uint8_t* data, uint8_t length) {
	writeDataToAddress(data, length, ARDUINO_ADDR, true);
}



//Read all channels of processor ADC - only for testing. For flight we'll want to break this up and 
static void readAnalog(float* temperatures, float* photodiodes, float* analogs){
	uint16_t buffer[LEN_ADC];
	uint16_t temp_buffer[8];
	uint16_t pd_buffer[6];
	
	readOtherADC(buffer);
	for (int i=0; i<LEN_ADC; i++){
		analogs[i] = convertToVoltage(buffer[i]);
	}
	
	readMuxs(temp_buffer, pd_buffer);
	for (int i=0; i<8; i++){
		float current = ((float) convertToVoltage(temp_buffer[i]))/2197-0.0002142; //converts from V to A
		temperatures[i] = (current)*1000000;// T = 454*V in K
	}
	for (int i=0; i<6; i++){
		photodiodes[i] = convertToVoltage(pd_buffer[i]);
	}
}

//Reads all sensors from Control Board remote ADC and returns in human readable voltage
void readRemoteADC_1(float* cntrlReadings){
	uint16_t remoteADC[4];
	AD7991_read_all(remoteADC, AD7991_CTRLBRD);
	cntrlReadings[0] = ((float) remoteADC[0])/4096*3.3*2.01;//3V6Rf
	cntrlReadings[1] = ((float) remoteADC[1])/4096*3.3;//3V6SNS
	cntrlReadings[2]= ((float)  remoteADC[2])/4096*3.3*3.381;//5VREF
	cntrlReadings[3] = ((float) remoteADC[3])/4096*3.3*2.01;//3V3REf
}



void flatsat_run(void) {	
	flatsat_init();	
	
	float pre_temp[8];
	float pre_pd[6];
	float pre_analog[LEN_ADC];
	float cntrlReadings[4];
	//float batReadings[4];
	
	
	
	
	setup_pin(true, P_RAD_PWR_RUN);
	set_output(true, P_RAD_PWR_RUN);

	
	int count =0; //Run only 15 times in case we get out of debug mode.
	while (count<1000){	
		
		//int curtime = get_count(); //gets the current time since build in seconds
		//readAnalog(pre_temp,pre_pd, pre_analog);
		
		//readRemoteADC_1(cntrlReadings);
		
		
		uint8_t value = 19 +count;
		uint8_t rtn;
		EEPROM_addr_struct addr;
		addr.LSB_8=1;
		addr.MSB_8=0;
		addr.MSB_2=0;
		enum status_code statc1 = writeByteToM24M01(addr,value);
		enum status_code statc2 = readByteFromM24M01(addr,&rtn);
		
		int len = 6;
		uint8_t values[len];
		for (int i=0; i<len; i++){
			values[i] = 0x40+count+i;
		}
		uint8_t rtn2[len];
		EEPROM_addr_struct addr2;
		addr2.LSB_8=0;
		addr2.MSB_8=1;
		addr2.MSB_2=0;
		enum status_code statc3 = writeBytesToM24M01(addr2,values,len);
		
		enum status_code statc4 = readBytesFromM24M01(addr2,rtn2,len);
		
		
		
		//set_output(true, P_RAD_PWR_RUN);
 		//set_output(false,P_LED_CMD); //Turns on LEDs for 100ms (hardware timed). Any breakpoints placed during flashing are not guaranteed to be accurate
		//readAnalog(dur1_temp,dur1_pd, dur1_analog);
		//set_output(true,P_LED_CMD);
		
		count = count +1;
	}
	
		
	
	
	/************************************************************************/
	/* EDIT THIS LOOP                                                       */
	/************************************************************************/
	//uint8_t arduinoBuffer[70];
	/*
	int count =0;
	
	while(count<5){
		float tempBuffer[8];
		float pdBuffer[6];
		float otherADCBuffer[LEN_ADC];
		uint16_t irBuffer[LEN_IR];
		MPU9250Reading mpuReading;
		
		readMuxs(tempBuffer, pdBuffer);
		readOtherADC(otherADCBuffer);
		read_IR(irBuffer);
		mpuReading = read_IMU();	
		count ++;	
	}*/
}