/*
 * global.c
 *
 * Created: 11/19/2017 3:53:48 PM
 *  Author: jleiken
 */ 

#include "global.h"

#if PRINT_DEBUG > 0 // if using debug print
	char debug_buf[128];

	StaticSemaphore_t _print_mutex_d;
#endif

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

	// MUST be before anything RTOS-related
	// (most notably, those creating mutexes)
	init_tracelyzer();	

	pin_init();
 	init_rtc();
	USART_init();
	radio_init();
	configure_i2c_master(SERCOM4);
	MLX90614_init();
	MPU9250_init();
	HMC5883L_init();
	delay_init();
	
	#if PRINT_DEBUG != 0
		print_mutex = xSemaphoreCreateMutexStatic(&_print_mutex_d);
	#endif
	
	init_sensor_read_commands();
	init_persistent_storage();
	init_errors();
	watchdog_init();
	#ifndef TESTING_SPEEDUP
		initialize_ecc(); // for reed-solomon lookup tables, etc.
	#endif
}

// initialization that can only be done with RTOS started
void global_init_post_rtos(void) {
	// now that errors are initialized, try to init AD7991 and log potential errors
	// TODO: we can't do this here because we haven't yet populated the error equistacks
	// from MRAM and these errors here may be overwritten once we do
	log_if_error(ELOC_AD7991_BBRD, AD7991_init(AD7991_BATBRD), true);
	log_if_error(ELOC_AD7991_CBRD, AD7991_init(AD7991_CTRLBRD), true);
}

// use in debug mode (set in header file)
// input: format string and arbitrary number of args to be passed to sprintf
// call to sprintf stores result in char *debug_buf
void print(const char *format, ...)
{
	#if PRINT_DEBUG > 0 // if debug mode
		if (rtos_started) {
			xSemaphoreTake(print_mutex, PRINT_MUTEX_WAIT_TIME_TICKS);
		}
		
		va_list arg;
		va_start (arg, format);
		vsprintf(debug_buf, format, arg);
		va_end (arg);
		
		#if configUSE_TRACE_FACILITY == 1
			#if PRINT_DEBUG == 2 || PRINT_DEBUG == 3
				vTracePrint(global_trace_channel, debug_buf);
			#endif
		#endif
		
		#if PRINT_DEBUG == 1 || PRINT_DEBUG == 3
			usart_send_string((uint8_t*) debug_buf);
		#endif
	
		if (rtos_started) {
			xSemaphoreGive(print_mutex);
		}
	#endif
}
