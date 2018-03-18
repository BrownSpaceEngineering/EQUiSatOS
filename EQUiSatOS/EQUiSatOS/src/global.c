/*
 * global.c
 *
 * Created: 11/19/2017 3:53:48 PM
 *  Author: jleiken
 */ 

#include "global.h"

#if PRINT_DEBUG > 0 // if using debug print
	#define DEBUG_BUF_SIZE		128
	char debug_buf[DEBUG_BUF_SIZE];

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
	setup_pin(true,P_L1_RUN_CHG);
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
	set_output(false, P_LF_B1_RUNCHG);
	set_output(false, P_LF_B2_RUNCHG);
	set_output(true, P_LED_CMD);
	
}

/* status codes from initialization to be logged once RTOS starts */
status_code_genare_t mpu9250_init_sc;
status_code_genare_t gyro_init_sc;
status_code_genare_t accel_init_sc;
status_code_genare_t mag_init_sc;

void global_init(void) {
	// Initialize the SAM system
	system_init();
	
	// start watchdog hardware ASAP
	#ifdef WATCHDOG_RESET_ACTIVE // note bootloader may have enabled it
		init_watchdog_clock();
	#endif

	// MUST be before anything RTOS-related
	// (most notably, those creating mutexes)
	init_tracelyzer();	

	pin_init();
 	init_rtc();
	USART_init();
	radio_control_init();
	configure_i2c_master(SERCOM4);
	MLX90614_init();
	mpu9250_init_sc = MPU9250_init();
	gyro_init_sc = gyro_init();
	accel_init_sc = accel_init();
	mag_init_sc = mag_init();
	
	HMC5883L_init();
	delay_init();
	
	#if PRINT_DEBUG != 0 && defined(SAFE_PRINT)
		print_mutex = xSemaphoreCreateRecursiveMutexStatic(&_print_mutex_d);
	#endif
	
	init_sensor_read_commands();
	init_persistent_storage();
	init_errors();
	
	#ifdef USE_REED_SOLOMON
		pet_watchdog(); // the following takes a while so pet before
		initialize_ecc(); // for reed-solomon lookup tables, etc.
	#endif
	#ifdef EQUISIM_SIMULATE_BATTERIES
		equisim_init();
	#endif
}

// initialization that can only be done with RTOS started
void global_init_post_rtos(void) {
	// initialize watchdog in RTOS because it's callback requires RTOS
	init_watchdog_task();
	// now that errors are initialized, try to init AD7991 and log potential errors
	log_if_error(ELOC_AD7991_BBRD, AD7991_init(AD7991_BATBRD), true);
	log_if_error(ELOC_AD7991_CBRD, AD7991_init(AD7991_CTRLBRD), true);
	log_if_error(ELOC_IMU_INIT, mpu9250_init_sc, true);
	log_if_error(ELOC_IMU_GYRO_INIT, gyro_init_sc, true);
	log_if_error(ELOC_IMU_ACCEL_INIT, accel_init_sc, true);
	log_if_error(ELOC_IMU_MAG_INIT, mag_init_sc, true);
}

// call this function to take the print mutex (suppress other task's printing)
// in order to print without being disturbed, and then CALL IT AGAIN to deactivate 
// and allow other tasks to print again
void suppress_other_prints(bool on) {
	#if PRINT_DEBUG > 0 && defined(SAFE_PRINT) // if debug mode
		if (on) {
			xSemaphoreTakeRecursive(print_mutex, PRINT_MUTEX_WAIT_TIME_TICKS);
			} else {
			xSemaphoreGiveRecursive(print_mutex);
		}
	#endif
}

// use in debug mode (set in header file)
// input: format string and arbitrary number of args to be passed to sprintf
// call to sprintf stores result in char *debug_buf
void print(const char *format, ...)
{
	#if PRINT_DEBUG > 0 // if debug mode
		#ifdef SAFE_PRINT
			bool got_mutex = false;
			if (rtos_ready) {
				got_mutex = xSemaphoreTakeRecursive(print_mutex, PRINT_MUTEX_WAIT_TIME_TICKS);
			}
		#endif
		
		va_list arg;
		va_start (arg, format);
		vsprintf(debug_buf, format, arg);
		va_end (arg);
		
		// add \r for each \n
		size_t len = strlen(debug_buf);
		if (len+1 <= DEBUG_BUF_SIZE && debug_buf[len-1] == '\n') {
			// replace \0 with carriage return
			debug_buf[len] = '\r';
			// add back \0
			debug_buf[len+1] = '\0';
		}
		
		#if configUSE_TRACE_FACILITY == 1
			#if PRINT_DEBUG == 2 || PRINT_DEBUG == 3
				vTracePrint(global_trace_channel, debug_buf);
			#endif
		#endif
		
		#if PRINT_DEBUG == 1 || PRINT_DEBUG == 3
			usart_send_string((uint8_t*) debug_buf);
		#endif
	
		#ifdef SAFE_PRINT
			if (rtos_ready) {
				if (got_mutex) xSemaphoreGiveRecursive(print_mutex);
			}
		#endif
	#endif
}
