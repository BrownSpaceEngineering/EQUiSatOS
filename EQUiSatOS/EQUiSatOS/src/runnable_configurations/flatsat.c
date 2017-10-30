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
	MLX90614_TOPPANEL_V4_2, 
	MLX90614_ACCESSPANEL_V3_1,
	MLX90614_SIDEPANEL_V4_2,
	MLX90614_SIDEPANEL_V4_3,
	MLX90614_SIDEPANEL_V4_4 };
	
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
	AD7991_init();
	///setup_switching();
	//TCA9535_init();
}

//Cycles through temperatures sensor and photodiode external muxes and does 10x software averaging on each channel
 void readMuxs(uint16_t* tempBuffer, uint16_t* pdBuffer){
	 
	 for (int i=0; i<4; i++){
		 configure_adc(&temp_instance,P_AI_TEMP_OUT);
		 return_struct_8 rs;
		 LTC1380_channel_select(0x48, i, rs);
		 
		uint16_t sum =0;
		
		for (int j=0; j<num_samples; j++){
			adc_enable(&temp_instance);
			sum = sum +read_adc(temp_instance);
		}
		tempBuffer[i] = sum/num_samples;
	 }
	 
	 for (int i=0; i<1; i++){
		configure_adc(&pd_instance,P_AI_PD_OUT);
		return_struct_8 rs;
		LTC1380_channel_select(0x4a, i, rs); 
		//pdBuffer[i] =(readVoltagemV(pd_instance));//-6.5105)/0.3708; // I = exp((V-6.5105/0.3708)) in uA
		
		uint16_t sum =0;
		
		for (int j=0; j<num_samples; j++){
			adc_enable(&pd_instance);
			sum = sum +read_adc(pd_instance);
		}
		uint16_t value = sum/num_samples;
		pdBuffer[i] = value;
	 }
 }
 
 //Cycles through all of the ADC channels that are not externally mux'd
 void readOtherADC(uint16_t* buffer){
	 for (int i=0; i<LEN_ADC; i++){
		 //buffer[i] = readVoltagemV(modules[i]);
		 //buffer[i] = readFromADC(adc_pins[i], num_samples);
		 //delay_ms(10);
	 }
 }
 


void read_IR(uint16_t* buffer) {
	for (int i = 0; i < LEN_IR; i++)
	{
		return_struct_16 rs;
		MLX90614_read2ByteValue(irs[i], MLX90614_TOBJ1, rs);
		buffer[i] = rs.return_value;
	}
}

void sendToArduino(uint8_t* data, uint8_t length) {
	writeDataToAddress(data, length, ARDUINO_ADDR, true);
}



//Read all channels of processor ADC - only for testing. For flight we'll want to break this up and 
void readAnalog(float* temperatures, float* photodiodes, float* analogs){
	uint16_t buffer[LEN_ADC];
	uint16_t temp_buffer[8];
	uint16_t pd_buffer[6];
	
	readOtherADC(buffer);
	for (int i=0; i<LEN_ADC; i++){
		analogs[i] = convertToVoltage(buffer[i]);
	}
	
	readMuxs(temp_buffer, pd_buffer);
	for (int i=0; i<8; i++){
		float current = ((float) convertToVoltage(temp_buffer[i]))/2197 -0.0001001; //converts from V to A
		temperatures[i] = (current)*1000000-273;// T = 454*V in C
	}
	for (int i=0; i<6; i++){
		photodiodes[i] = convertToVoltage(pd_buffer[i]);
	}
}

//Reads all sensors from Control Board remote ADC and returns in human readable voltage
void readRemoteADC_1(float* cntrlReadings){
	uint16_t remoteADC[4];
	AD7991_read_all(remoteADC, AD7991_ADDR_1);
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
	usart_send_string("Begin\n");
	char temp[50];
	while (count<1000){	
		
		//int curtime = get_count(); //gets the current time since build in seconds
		readAnalog(pre_temp,pre_pd, pre_analog);
		
		readRemoteADC_1(cntrlReadings);
		
		usart_send_string("\n___________________\n");
		sprintf(temp,"Sending new batch of data. Counts Since Boot = %d.\n\n",count );
		usart_send_string(temp);

		sprintf(temp,"3V3 Regulator Voltage Sense\t=%dmV\n",((uint16_t)(cntrlReadings[3]*1000)));
		usart_send_string(temp);
		
		sprintf(temp,"5V Regulator Voltage Sense\t=%dmV\n", ((uint16_t)(cntrlReadings[2]*1000)));
		usart_send_string(temp);
		
		sprintf(temp,"3V6 Regulator Voltage Sense\t=%dmV\n", ((uint16_t)(cntrlReadings[0]*1000)));
		usart_send_string(temp);
		
		sprintf(temp,"3V6 Regulator Current Sense\t=%dmA\n", ((uint16_t)(cntrlReadings[1]*1000)));
		usart_send_string(temp);
		
		if (((uint16_t)(cntrlReadings[0]*1000))<3000){
			usart_send_string("3V6 rail looking funky\n\n\n\n\n\n\n\n*****************");
		}
	
		usart_send_string("--------------------\n");
		//set_output(true, P_RAD_PWR_RUN);
 		//set_output(false,P_LED_CMD); //Turns on LEDs for 100ms (hardware timed). Any breakpoints placed during flashing are not guaranteed to be accurate
		//readAnalog(dur1_temp,dur1_pd, dur1_analog);
		//set_output(true,P_LED_CMD);
		delay_ms(10);
		
		count = count +1;
	}
	usart_send_string("end\n");
	
		
	
	
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