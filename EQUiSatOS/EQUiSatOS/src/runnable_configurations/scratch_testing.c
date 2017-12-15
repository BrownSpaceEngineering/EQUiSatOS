/*
 * scratch_testing.c
 *
 * Created: 9/18/2016 4:04:37 PM
 *  Author: Daniel
 */ 

#include "global.h"
#include "scratch_testing.h"

#include "../telemetry/rscode-1.3/ecc.h"
#include "../processor_drivers/Flash_Commands.h"

void runit(void){	
	configure_i2c_standard(SERCOM4);
	MLX90614_init();
	//float data = MLX90614_readTempC(MLX90614_TBOARD_IR2,false);
	//int a = 3;
	
	/*
	uint8_t addr = 0x01;
	
	while(addr != 0x00){
		float data = MLX90614_readTempC(MLX90614_DEFAULT_I2CADDR,false);
		float data2 = MLX90614_readTempC(0x5B,false);
		int a = 2;
		int b = a+1;
		if(data2 > -273){
			int a=1;
		}
		addr = addr + 1;
	}
	**/
	/**
	//READ FROM MAGNETOMETER
	HMC5883L_init(*i2c_write_command, *i2c_read_command);
	for (int j = 0; j < 15; j++) {
		uint8_t readBuff[6] = {0, 0, 0, 0, 0, 0};
		HMC5883L_read(readBuff);
		int16_t xyzBuff[3] = {0, 0, 0};
		HMC5883L_getXYZ(readBuff, xyzBuff);
		int i;
		for (i = 0; i < 6; i++) {		
			printf("%x ", readBuff[i]);					
		}
		printf("\n\r");
		printf("%d %d %d\n\r", xyzBuff[0], xyzBuff[1], xyzBuff[2]);	
		float heading = HMC5883L_computeCompassDir(xyzBuff[0], xyzBuff[1], xyzBuff[2]);	
		int a = 0;
	}
	**/

	/*
	uint16_t val1 = MLX90614_getAddress(MLX90614_DEFAULT_I2CADDR);
	MLX90614_setAddress(MLX90614_DEFAULT_I2CADDR,MLX90614_DEFAULT_I2CADDR);
	uint16_t val2 = MLX90614_getAddress(MLX90614_DEFAULT_I2CADDR);
	
	while(true){
		float x = MLX90614_readTempC(MLX90614_DEFAULT_I2CADDR,false);
		int y = 1;
	}
	*/
	setup_switching();
 	pick_side(true);
	//bool x =get_input(SIDE_1_ENABLE);
	pick_input(0x00);
	pick_input(0x01);
	pick_input(0x02);
	pick_input(0x03);
	pick_side(false);
	pick_input(0x00);
	pick_input(0x01);
	pick_input(0x02);
	pick_input(0x03);
	pick_side(true);
	
	struct adc_module temp_instance;
	configure_adc(&temp_instance,ADC_POSITIVE_INPUT_PIN8);

	/*int i = 0;
	int cum = 0;
	while(i<50){
		int x = readVoltagemV(temp_instance);
		cum += x;
		printf("%d\n\r",x);
		i++;
	}
	printf("AVG: %d\n\r", cum/i);*/
    //printf("Temperature in F: %f\r\n", MLX90614_read_temperature());

	// motor controller test that says bad address
	/*uint8_t write_buffer[3] = {
		0xaa, 0x0a, 0x00
	};
	
	struct i2c_master_packet write_packet = {
		.address     = 0x0f,
		.data_length = 3,
		.data        = write_buffer,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};
	i2c_write_command(&write_packet);
	
	// Create task to monitor processor activity
	if (xTaskCreate(task_monitor, "Monitor", TASK_MONITOR_STACK_SIZE, NULL,
			TASK_MONITOR_STACK_PRIORITY, NULL) != pdPASS) {
		//printf("Failed to create Monitor task\r\n");
	}

	// Create task to make led blink
	if (xTaskCreate(task_led, "Led", TASK_LED_STACK_SIZE, NULL,
			TASK_LED_STACK_PRIORITY, NULL) != pdPASS) {
		//printf("Failed to create test led task\r\n");
	}
	
	// Create task to make led blink
	if (xTaskCreate(task_adc_read, "ADC", TASK_ADC_STACK_SIZE, NULL,
	TASK_ADC_STACK_PRIORITY, NULL) != pdPASS) {
		//printf("Failed to create test led task\r\n");
	}

	// Start the scheduler.
	vTaskStartScheduler();

	// Will only get here if there was insufficient memory to create the idle task.
	*/
}

void radioTest(void) {	
	USART_init();
	setup_pin(true, P_RAD_PWR_RUN); //3v6 enable
	setup_pin(true,P_RAD_SHDN); //init shutdown pin
	setup_pin(true,P_TX_EN); //init send enable pin
	setup_pin(true,P_RX_EN); //init receive enable pin
	
	set_output(true, P_RAD_PWR_RUN);
	set_output(true, P_RAD_SHDN);
	set_output(false, P_TX_EN);
	set_output(false, P_RX_EN);
		
	//char toSend[] = {0x01, 0x00, 0x08, 0x00, 0x03, 0x30, 0x31, 0x32, 0x61};				
	while (true) {		
		usart_send_string((uint8_t *)"EQUiSat\n");
		delay_ms(1500);		
	}
	/*while (true) {
		delay_ms(500);
		print(receivebuffer);	
	}*/
			
}

static void byte_err (int err, int loc, unsigned char *dst)
{
  //printf("Adding Error at loc %d, data %#x\n", loc, dst[loc-1]);
  dst[loc-1] ^= err;
}

//Pass in location of error (first byte position is labeled starting at 1, not 0), and the codeword.
static void byte_erasure (int loc, unsigned char dst[], int cwsize, int erasures[]) 
{
  //printf("Erasure at loc %d, data %#x\n", loc, dst[loc-1]);
  dst[loc-1] = 0;
}

void rsTest(void) {
	unsigned char msg[] = "The quick brown fox jumped over the lazy dog.";
	unsigned char codeword[256];
	 
	uint8_t erasures[16];
	int nerasures = 0;
	 
	// Initialization the ECC library
	initialize_ecc();
	
	encode_data(msg, sizeof(msg), codeword);
	
	#define ML (sizeof (msg) + NPAR)
	
	//introduce errors and erasures	
	byte_err(0x35, 3, codeword);

	byte_err(0x23, 17, codeword);
	byte_err(0x34, 19, codeword);
	
	//We need to indicate the position of the erasures.  Eraseure
    //positions are indexed (1 based) from the end of the message... 
	erasures[nerasures++] = ML-17;
	erasures[nerasures++] = ML-19;
	
	decode_data(codeword, ML);
	
	 //check if syndrome is all zeros
	 if (check_syndrome () != 0) {
		 correct_errors_erasures (codeword,
		 ML,
		 nerasures,
		 erasures);		 		 
	 }	   
}

void simpleADCTest(void) {
	struct adc_module adc_instance;
	configure_adc(&adc_instance, PIN_PB00); // must be an analog pin

	uint16_t value;
	enum status_code sc = read_adc(adc_instance, &value);
	float voltage = convertToVoltage(value);
}

void flashBurstTest(void) {
	uint16_t two_buf[2];
	uint16_t three_buf[3];
	uint16_t four_buf[4];
	
	
	//print("\n# LED Temps #\n");
	read_led_temps_batch(four_buf);
	//print("%d %d %d %d\n", four_buf[0], four_buf[1], four_buf[2], four_buf[3]);
	
	//print("\n# LiFePO4 Temps #\n");
	read_lifepo_temps_batch(two_buf);
	//print("%d %d\n", two_buf[0], two_buf[1]);
	
	//print("# LiFePO4 CURRENT #\n");
	read_lifepo_current_batch(four_buf);
	//print("%d %d %d %d\n", four_buf[0], four_buf[1], four_buf[2], four_buf[3]);
	
	//print("\n# LiFePO4 VOLTS #\n");
	read_lifepo_volts_batch(four_buf);
	//print("%d %d %d %d\n", four_buf[0], four_buf[1], four_buf[2], four_buf[3]);
	
	//print("# LED CURRENT #\n");
	read_led_current_batch(four_buf);
	//print("%d %d %d %d\n", four_buf[0], four_buf[1], four_buf[2], four_buf[3]);
	
	//print("\n# GYRO #\n");
	read_gyro_batch(three_buf);
	//print("x: %d y: %d z: %d\n", three_buf[0], three_buf[1], three_buf[2]);		
}


void abbreviatedFlashBurstTest(lifepo_volts_batch volts, lifepo_current_batch current, led_current_batch led_current) {
	read_lifepo_volts_batch(volts);
	read_lifepo_current_batch(current);
	//read_led_current_batch(led_current);
}

void actuallyFlashingFlashBurstTest(void) {
	//#define HUMAN
	
	int num = 30;
	lifepo_volts_batch volts[num];
	lifepo_current_batch current[num];
	led_current_batch led_current[num];
	
	int i = 0;
	for (; i < 5; i++) {
		abbreviatedFlashBurstTest(volts[i], current[i], led_current[i]);
	}
	
	set_lifepo_output_enable(true);
 	reset_flash_pin();
 	flash_leds();
	
	for (; i < num; i++) {
		abbreviatedFlashBurstTest(volts[i], current[i], led_current[i]);
	}
	set_lifepo_output_enable(false);
	
	delay_ms(500);
	
	#ifdef HUMAN
	
	// REGEX to parse this: (\w+)\s+(\d+)\s+(\d+)\s+\w+$ -> $1, $2, $3 , \=+.*\=+ -> ""
	print("==============FLASH Test==============\n");	
	for (i = 0; i < num; i++) {
		print("============== %s READING %d \t ==============\n", (i < 5) ? "PRE-FLASH" : "", i);	
		print(" %s \t %d \t %d mV\n", "P_AI_LF1REF", volts[i][0], (uint16_t)(convertToVoltage(volts[i][0])*1000));
		print(" %s \t %d \t %d mV\n", "P_AI_LF2REF", volts[i][1], (uint16_t)(convertToVoltage(volts[i][1])*1000));
		print(" %s \t %d \t %d mV\n", "P_AI_LF3REF", volts[i][2], (uint16_t)(convertToVoltage(volts[i][2])*1000));
		print(" %s \t %d \t %d mV\n", "P_AI_LF4REF", volts[i][3], (uint16_t)(convertToVoltage(volts[i][3])*1000));
			
		print(" %s \t %d \t %d mV\n", "P_AI_LFB1SNS",  current[i][0], (uint16_t)(convertToVoltage(current[i][0])*1000));
		print(" %s \t %d \t %d mV\n", "P_AI_LFB1OSNS", current[i][1], (uint16_t)(convertToVoltage(current[i][1])*1000));
		print(" %s \t %d \t %d mV\n", "P_AI_LFB2SNS",  current[i][2], (uint16_t)(convertToVoltage(current[i][2])*1000));
		print(" %s \t %d \t %d mV\n", "P_AI_LFB2OSNS", current[i][3], (uint16_t)(convertToVoltage(current[i][3])*1000));
	}
	
	#else 
	
	// REGEX to parse this: (\w+)\s+(\d+)\s+(\d+)\s+\w+$ -> $1, $2, $3 , \=+.*\=+ -> ""
	print("==============FLASH Test==============\n");
	print("%s, \t%s, \t%s, \t%s, \t%s, \t%s, \t%s, \t%s, \t%s, \t%s, \t%s, \t%s, \t%s, \t%s, \t%s, \t%s\n", 
		"P_AI_LF1REF", "P_AI_LF2REF", "P_AI_LF3REF", "P_AI_LF4REF", "P_AI_LFB1SNS", "P_AI_LFB1OSNS", "P_AI_LFB2SNS", "P_AI_LFB2OSNS",
		"P_AI_LF1REF(mv)", "P_AI_LF2REF(mv)", "P_AI_LF3REF(mv)", "P_AI_LF4REF(mv)", "P_AI_LFB1SNS(mv)", "P_AI_LFB1OSNS(mv)", "P_AI_LFB2SNS(mv)", "P_AI_LFB2OSNS(mv)");
	for (i = 0; i < num; i++) {
		print("%d, \t%d, \t%d, \t%d, \t%d, \t%d, \t%d, \t%d, \t%d, \t%d, \t%d, \t%d, \t%d, \t%d, \t%d, \t%d\n", 
			volts[i][0], volts[i][1], volts[i][2], volts[i][3], current[i][0], current[i][1],  current[i][2], current[i][3],
			(uint16_t)(convertToVoltage(volts[i][0])*1000), (uint16_t)(convertToVoltage(volts[i][1])*1000), (uint16_t)(convertToVoltage(volts[i][2])*1000), (uint16_t)(convertToVoltage(volts[i][3])*1000),
			(uint16_t)(convertToVoltage(current[i][0])*1000), (uint16_t)(convertToVoltage(current[i][1])*1000), (uint16_t)(convertToVoltage(current[i][2])*1000), (uint16_t)(convertToVoltage(current[i][3])*1000));
	}
	
	#endif
	delay_ms(500);
}