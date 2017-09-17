/*
 * scratch_testing.c
 *
 * Created: 9/18/2016 4:04:37 PM
 *  Author: Daniel
 */ 

#include "run.h"
#include "scratch_testing.h"

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
	
	initializeRadio();	
	char toSend[] = {0x01, 0x00, 0x08, 0x00, 0x03, 0x30, 0x31, 0x32, 0x61};				
	while (true) {		
		usart_send_string("EQUiSat\n");
		delay_ms(1500);		
	}
	/*while (true) {
		delay_ms(500);
		print(receivebuffer);	
	}*/
			
}