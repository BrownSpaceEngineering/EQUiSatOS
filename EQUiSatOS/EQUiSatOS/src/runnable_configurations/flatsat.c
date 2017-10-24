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
	///setup_switching();
	//TCA9535_init();
}

 void readMuxs(uint8_t* tempBuffer, uint8_t* pdBuffer){
	 
	 for (int i=0; i<4; i++){
		 //struct adc_module temp_instance;
		 configure_adc(&temp_instance,P_AI_TEMP_OUT); // Changed for proc test board testing
		 return_struct_8 rs;
		 LTC1380_channel_select(0x48, i+4, rs);
		 //tempBuffer[i] = readVoltagemV(temp_instance);
		 
		uint16_t sum =0;
		
		for (int j=0; j<num_samples; j++){
			adc_enable(&temp_instance);
			sum = sum +read_adc(temp_instance);
		}
		tempBuffer[i] = sum/num_samples;
	 }
	 
	 for (int i=0; i<1; i++){
		//struct adc_module adc_instance;
		configure_adc(&pd_instance,P_AI_PD_OUT);
		return_struct_8 rs;
		LTC1380_channel_select(0x4a, i+3, rs); 
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
 
 
 void readOtherADC(uint8_t* buffer){
	 for (int i=0; i<LEN_ADC; i++){
		 //buffer[i] = readVoltagemV(modules[i]);
		 buffer[i] = readFromADC(adc_pins[i]);
		 //delay_ms(10);
	 }
 }
 
 uint8_t readFromADC(enum adc_positive_input pin){
	struct adc_module adc_instance;
	configure_adc(&adc_instance,pin);
	read_adc(adc_instance);
	adc_enable(&adc_instance);
	uint16_t sum =0;
	
	for (int i=0; i<num_samples; i++){
		adc_enable(&adc_instance);
		sum = sum +read_adc(adc_instance);
	}
	return sum/num_samples;
}

void read_IR(uint16_t* buffer) {
	for (int i = 0; i < LEN_IR; i++)
	{
		return_struct_16 rs;
		MLX90614_read2ByteValue(irs[i], MLX90614_TOBJ1, rs);
		buffer[i] = rs.return_value;
	}
}

MPU9250Reading read_IMU(void) {
	MPU9250Reading data = MPU9250_read();
	return data;
}

void sendToArduino(uint8_t* data, uint8_t length) {
	writeDataToAddress(data, length, ARDUINO_ADDR, true);
}

float convertToVoltage(uint8_t reading){
	return (((float) (reading))/255*3.300/1.48); //converts from 8 bit to V
}

void readAnalog(float* temperatures, float* photodiodes, float* analogs){
	uint8_t buffer[LEN_ADC];
	uint8_t temp_buffer[4];
	uint8_t pd_buffer[1];
	
	readOtherADC(buffer);
	for (int i=0; i<LEN_ADC; i++){
		analogs[i] = convertToVoltage(buffer[i]);
	}
	readMuxs(temp_buffer, pd_buffer);
	for (int i=0; i<4; i++){
		float current = ((float) convertToVoltage(temp_buffer[i]))/2197 -0.0001002; //converts from V to A
		temperatures[i] = (current)*1000000-273;// T = 454*V in C
	}
	for (int i=0; i<1; i++){
		photodiodes[i] = convertToVoltage(pd_buffer[i]);
	}
}

void flatsat_run(void) {	
	flatsat_init();	
	
	float pre_temp[4];
	float pre_pd[1];
	float pre_analog[LEN_ADC];
	
	float dur1_temp[4];
	float dur1_pd[1];
	float dur1_analog[LEN_ADC];
	
	float dur2_temp[4];
	float dur2_pd[1];
	float dur2_analog[LEN_ADC];
	


	int count =0; //Run only 5 times in case we get out of debug mode.
	while (count<5){
		
		
		
		readAnalog(pre_temp,pre_pd, pre_analog);
		
		
 		set_output(false,P_LED_CMD); //Turns on LEDs for 100ms (hardware timed). Any breakpoints placed during flashing are not guaranteed to be accurate
		readAnalog(dur1_temp,dur1_pd, dur1_analog);
		set_output(true,P_LED_CMD);
		readAnalog(dur2_temp,dur2_pd, dur2_analog);
		
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