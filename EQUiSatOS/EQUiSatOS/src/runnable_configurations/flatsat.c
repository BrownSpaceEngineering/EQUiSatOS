#include "flatsat.h"

struct adc_module temp_instance;
struct adc_module pd_instance;
uint8_t magReadBuff[6] = {0, 0, 0, 0, 0, 0};
int16_t xyzBuff[3] = {0, 0, 0};
	
enum adc_positive_input adc_pins[14] = {
ADC_POSITIVE_INPUT_PIN11, //LED1SNS
ADC_POSITIVE_INPUT_PIN10, //LED2SNS
ADC_POSITIVE_INPUT_PIN9,  //LED3SNS
ADC_POSITIVE_INPUT_PIN8,  //LED4SNS
ADC_POSITIVE_INPUT_PIN19, //LFB1OSNS
ADC_POSITIVE_INPUT_PIN18, //LFB1SNS
ADC_POSITIVE_INPUT_PIN16, //LFB2OSNS
ADC_POSITIVE_INPUT_PIN17, //LFB2SNS
ADC_POSITIVE_INPUT_PIN6,  //LF1REF
ADC_POSITIVE_INPUT_PIN5,  //LF2REF
ADC_POSITIVE_INPUT_PIN3,  //LF3REF
ADC_POSITIVE_INPUT_PIN4,  //LF4REF
ADC_POSITIVE_INPUT_PIN1,  //L1_REF
ADC_POSITIVE_INPUT_PIN7,  //L2_REF
};


struct adc_module modules[14];

//TODO: FILL WITH SENSOR LIST, make LEN_IR the list length
uint8_t irs[LEN_IR] = {MLX90614_DEFAULT_I2CADDR};

	
void flatsat_init(void) {

	configure_i2c_standard(SERCOM4);
	//temperature
	configure_adc(&temp_instance,ADC_POSITIVE_INPUT_PIN2);
	//pd
	configure_adc(&pd_instance, ADC_POSITIVE_INPUT_PIN0);
	
	//other adc
	for (int i=0; i<14; i++){
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


 void readMuxs(float* buffer1, float* buffer2){
	 for (int i=0; i<8; i++){
		 LTC1380_channel_select(0x48, i);
		 buffer1[i] = readVoltagemV(temp_instance);
	 }
	 
	 for (int i=0; i<6; i++){
		LTC1380_channel_select(0x49, i); 
		buffer2[i] = readVoltagemV(pd_instance);
	 }
 }
 
 
 void readOtherADC(float* buffer){
	 for (int i=0; i<14; i++){
		buffer[i] = readVoltagemV(modules[i]) ;
	 }
 }


void read_IR(uint16_t* buffer) {
	for (int i = 0; i < LEN_IR; i++)
	{
		buffer[i] = MLX90614_read2ByteValue(irs[i], MLX90614_TOBJ1);
	}
}

MPU9250Reading read_IMU(void) {
	MPU9250Reading data = MPU9250_read();
	return data;
}



void flatsat_run(void) {	
	flatsat_init();
<<<<<<< HEAD
	/*uint16_t a = 124;
	while (true) {
		writeDataToAddress(&a, 1, 0x08, true);
	}*/
	//set_output(true, PIN_PB11);
	//set_output(true, PIN_PB17);

	//while(true){
		//led_flash();
		//uint16_t x = AD7991_read(0x0);
		//int y = 2;
	//}
	
	
	/*
	pick_side(true);
	pick_input(0x1);
	struct adc_module temp_instance;
	configure_adc(&temp_instance,ADC_POSITIVE_INPUT_PIN8);
=======
>>>>>>> origin/master
	
	/************************************************************************/
	/* EDIT THIS LOOP                                                       */
	/************************************************************************/
	while(true){
		float mux1Buffer[8];
		float mux2Buffer[6];
		float otherADCBuffer[14];
		uint16_t irBuffer[LEN_IR];
		MPU9250Reading mpuReading;
		
		readMuxs(mux1Buffer, mux2Buffer);
		readOtherADC(otherADCBuffer);
		read_IR(irBuffer);
		mpuReading = read_IMU();
	}
}