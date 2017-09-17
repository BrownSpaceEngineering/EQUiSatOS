#include "flatsat.h"

struct adc_module temp_instance;
struct adc_module pd_instance;
uint8_t magReadBuff[6] = {0, 0, 0, 0, 0, 0};
int16_t xyzBuff[3] = {0, 0, 0};

struct adc_module modules[LEN_ADC];

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

	configure_i2c_standard(SERCOM4);
	//temperature
	configure_adc(&temp_instance,P_AI_TEMP_OUT);
	//pd
	configure_adc(&pd_instance, P_AI_PD_OUT);
	
	//other adc
	for (int i=0; i<LEN_ADC; i++){
		configure_adc(&modules[i], adc_pins[i]);
	}
	
	MLX90614_init();
	
	MPU9250_init();
	
	//setup_pin(true,PIN_PB17);
	//setup_pin(true,PIN_PA12);
	//setup_pin(true,PIN_PB11);
	//set_output(true, PIN_PA12); //init gpio pin for flashing
	//configure_watchdog();
	//LTC1380_init();
	///setup_switching();
	//TCA9535_init();
}

 void readMuxs(float* tempBuffer, float* pdBuffer){
	 for (int i=0; i<8; i++){
		 LTC1380_channel_select(0x48, i);
		 tempBuffer[i] = readVoltagemV(temp_instance);
	 }
	 
	 for (int i=0; i<6; i++){
		LTC1380_channel_select(0x49, i); 
		pdBuffer[i] = readVoltagemV(pd_instance);
	 }
 }
 
 
 void readOtherADC(float* buffer){
	 for (int i=0; i<LEN_ADC; i++){
		buffer[i] = readVoltagemV(modules[i]) ;
	 }
 }

void read_IR(uint16_t* buffer) {
	for (int i = 0; i < LEN_IR; i++)
	{
		buffer[i] = MLX90614_read2ByteValue(irs[i], MLX90614_TOBJ1).return_value;
	}
}

MPU9250Reading read_IMU(void) {
	MPU9250Reading data = MPU9250_read();
	return data;
}

void sendToArduino(uint8_t* data, uint8_t length) {
	writeDataToAddress(data, length, ARDUINO_ADDR, true);
}

void flatsat_run(void) {	
	flatsat_init();	
	
	/************************************************************************/
	/* EDIT THIS LOOP                                                       */
	/************************************************************************/
	uint8_t arduinoBuffer[70];
	
	while(true){
		float tempBuffer[8];
		float pdBuffer[6];
		float otherADCBuffer[LEN_ADC];
		uint16_t irBuffer[LEN_IR];
		MPU9250Reading mpuReading;
		
		readMuxs(tempBuffer, pdBuffer);
		readOtherADC(otherADCBuffer);
		read_IR(irBuffer);
		mpuReading = read_IMU();		
	}
}