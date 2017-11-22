/*
 * global.c
 *
 * Created: 11/19/2017 3:53:48 PM
 *  Author: jleiken
 */ 

#include "global.h"

void init_tracelyzer(void) {
	#if configUSE_TRACE_FACILITY == 1
		#if TRC_CFG_RECORDER_MODE == TRC_RECORDER_MODE_STREAMING
			vTraceEnable(TRC_INIT); // start streaming data when requested instead of waiting here
		#elif TRC_CFG_RECORDER_MODE == TRC_RECORDER_MODE_SNAPSHOT
			vTraceEnable(TRC_START);
		#endif
	
		global_trace_channel = xTraceRegisterString("Global Events");
	#endif
}

void trace_print(const char *format)
{
	// NOTE: va_args won't work; no function that takes that :(
	#if configUSE_TRACE_FACILITY == 1
		vTracePrintF(global_trace_channel, format);
	#endif
}

static void pin_init(void) {
	// set write pins
	setup_pin(true,P_LF_B1_OUTEN);
	setup_pin(true,P_LF_B2_OUTEN);
	setup_pin(true,P_LF_B1_RUNCHG);
	setup_pin(true,P_LF_B2_RUNCHG);
	setup_pin(true,P_L1_RUN_CHG); //TODO consider if we need these here
	setup_pin(true,P_L2_RUN_CHG);
	setup_pin(true,P_L1_DISG);
	setup_pin(true,P_L2_DISG);
	setup_pin(true,P_LED_CMD);
	setup_pin(true,P_RAD_PWR_RUN); //3v6 enable
	setup_pin(true,P_RAD_SHDN); //init shutdown pin
	setup_pin(true,P_TX_EN); //init send enable pin
	setup_pin(true,P_RX_EN); //init receive enable pin
	setup_pin(true,P_IR_PWR_CMD); //init low power ir pin
	setup_pin(true,P_5V_EN);
	
	// set read pins
	setup_pin(false, P_DET_RTN);
	
	// initial writes
	set_output(false, P_LF_B1_OUTEN);
	set_output(false, P_LF_B2_OUTEN);
	set_output(false, P_L1_RUN_CHG);
	set_output(false, P_L2_RUN_CHG);
	
}

void global_init(void) {
	// Initialize the SAM system
	system_init();

	pin_init();
	init_rtc();
	USART_init();
	configure_i2c_master(SERCOM4);
	MLX90614_init();
	MPU9250_init();
	delay_init();
	
	init_tracelyzer();	// MUST be before anything RTOS-related! (Equistacks in init_errors!)
	
	init_errors();
	
	// now that errors are initialized, try to init AD7991 and log potential errors
	log_if_error(ELOC_AD7991_0, AD7991_init(AD7991_BATBRD), true);
	log_if_error(ELOC_AD7991_1, AD7991_init(AD7991_CTRLBRD), true);
}