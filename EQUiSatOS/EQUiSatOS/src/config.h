/*
 * config.h
 *
 * Created: 11/17/2017 8:02:34 PM
 *  Author: Tyler
 */


#ifndef CONFIG_H_
#define CONFIG_H_

/*************************************************************************/
/*							CONFIG OPTIONS                               */
/*************************************************************************/

//Choose 1 Config: Either FLIGHT, FLATSAT or XPLAINED
#define FLIGHT

/**
 * flags to control dangerous / inconvenient functions of the satellite
 * MAKE SURE to RE-BUILD solution to be double-sure they were disabled
 */
#define FLASH_ACTIVE // enable flashing
#define ANTENNA_DEPLOY_ACTIVE // enable antenna deployment
#define TRANSMIT_ACTIVE // enable radio transmission (note PRINT_DEBUG must be 0)
#define BAT_CHARGING_ACTIVE // enable battery charging logic ACTUALLY changing battery actions
#define WATCHDOG_RESET_ACTIVE //  enable watchdog actually rebooting satellite; WARNING bootloader may still start watchdog even if this is disabled

/*** TESTING ***/
// used for final release settings (disables debug helpers, etc.)
#define RELEASE

// define this flag to disable normal frequencies and read data faster,
// ***AT THE EXPENSE OF COMPLETELY NORMAL FUNCTIONALITY***
// ENUMERATION of changes when ON:
// - don't read data distributed around an orbit
//#define TESTING_SPEEDUP
//#define TESTING_SPEEDUP_LONG_OPS

// reduces the stack size of some tasks to a workable but risky level
//#define RISKY_STACK_SIZES

// use this flag to disable reed-solomon tables (frees up memory)
#define USE_REED_SOLOMON

// if defined, explicitly sets the initial SAT state (must set both)
//#define OVERRIDE_INIT_SAT_STATE		IDLE_FLASH
//#define OVERRIDE_INIT_TASK_STATES	IDLE_FLASH_TASK_STATES
//													WDOG,  STATE,	(ant),	BAT,	TRANS,	FLASH,	IDLE,  	LOWP,	ATTI,	PERSIST
//#define OVERRIDE_INIT_TASK_STATES	((task_states){{true,	true,	false,	true,	false,	false,	false,	false,	true,   true}})
#define OVERRIDE_STATE_HOLD_INIT	0 // whether to hold initial state (stop auto state changes)

// whether to start up misc. testing tasks (doing both at same time will likely run out of mem)
//#define ONLY_RUN_TESTING_TASKS	// doesn't run normal EQUISAT tasks
//#define RUN_RTOS_SYSTEM_TEST	// ~800 bytes of memory
	// less data
	//#define RTOS_SYSTEM_TEST_SUMMARY
	// whether to print all data every test or only one element per stack (with exception of error stack)
	//#define RTOS_SYSTEM_TEST_ONLY_RECENT
//	#define SYSTEM_TEST_TASK_FREQ		(60*1000) // ms
//#define RUN_TESTING_TASK		// ~1024 bytes of memory
//#define RUN_TASK_STACK_TESTS

// whether to rewrite MRAM with "zero" values (done before launch)
//#define WRITE_DEFAULT_MRAM_VALS

/** System Tests/Simulations **/
//#define EQUISIM_SIMULATE_BATTERIES // see config in equisim_simulated_data.h
//#define EQUISIM_SIMULATE_DIRECT_STATE_CHANGES // must disable OVERRIDE_STATE_HOLD_INIT!
//#define EQUISIM_IN_STATE_TIME_MS			(60*1000)
//#define EQUISIM_WATCHDOG_RESET_TEST

/** Debug **/
// whether to include Tracelyzer tracing library
#define USE_TRACELYZER				0

// whether to configASSERTs (and other asserts) should hang
//#define USE_ASSERTIONS

// add strict asserts to notify of events that we want to be rare
//#define USE_STRICT_ASSERTIONS

// debug print control; set to:
// 0 to suppress prints and allow radio transmission
// 1 to print over USART
// 2 to print over Tracelyzer trace_print
// 3 to print over both
#define PRINT_DEBUG					0
// whether to let normal transmissions print while print enabled
//#define DONT_PRINT_RAW_TRANSMISSIONS
// whether to print hex version of transmissions + data summaries with each transmit
//#define PRINT_HEX_TRANSMISSIONS
// whether to use a mutex to avoid collisions with radio
// define for radio testing but turn off for timing/task testing
// (it slows down and alters the OS timings)
//#define SAFE_PRINT

// define to print errors as the come in
//#define PRINT_NEW_ERRORS // note: probably will result in many stack overflows

/****************************************************************************************/
/*  DO NOT EDIT BELOW THIS LINE UNLESS YOU ARE INTENTIONALLY MODIFYING CONFIGURATION    */
/****************************************************************************************/

#ifdef FLIGHT
	//Sets processor pinouts
	#define	CTRL_BRD_V3

	//Flight IR sensors
	#define	IR_POS_Y	MLX90614_FLASHPANEL_V6_2_2	//+Y
	#define	IR_NEG_X	MLX90614_SIDEPANEL_V5_5B	//-X
	#define	IR_NEG_Y	MLX90614_SIDEPANEL_V5_5		//-Y
	#define	IR_POS_X		MLX90614_RBFPANEL_V1_1		//+X
	#define IR_NEG_Z	MLX90614_ACCESSPANEL_V4_7	//-Z
	#define IR_POS_Z		MLX90614_TOPPANEL_V5_1		//+Z

#endif
#ifdef FLATSAT
	//Sets processor pinouts
	#define	CTRL_BRD_V3

	/*Flatsat IR sensors*/
	#define	IR_POS_Y	MLX90614_FLASHPANEL_V6_2_1
	#define	IR_NEG_X	MLX90614_SIDEPANEL_V5_3
	#define	IR_NEG_Y	MLX90614_SIDEPANEL_V5_2
	#define	IR_POS_X		MLX90614_RBFPANEL_V1_4
	#define IR_NEG_Z	MLX90614_ACCESSPANEL_V4_6
	#define IR_POS_Z		MLX90614_TOPPANEL_V5_2

#endif
#ifdef XPLAINED
	//Sets processor pinouts
	#define	CTRL_BRD_V3

	/*Flatsat IR sensors (because we have nothing else to do) */
	#define	IR_POS_Y	MLX90614_FLASHPANEL_V6_2_1
	#define	IR_NEG_X	MLX90614_SIDEPANEL_V5_5
	#define	IR_NEG_Y	MLX90614_SIDEPANEL_V5_2
	#define	IR_POS_X		MLX90614_RBFPANEL_V1_4
	#define IR_NEG_Z	MLX90614_ACCESSPANEL_V4_6
	#define IR_POS_Z		MLX90614_TOPPANEL_V4_1

	#define ADC_RD_VAL_XPLAINED		0x0300
#endif

/****************************************************************************************/
/*					 SENSOR ADDRESSES BELOW THIS LINE - DO NOT EDIT					    */
/****************************************************************************************/

//Remote ADCs
#define AD7991_BATBRD 0b0101000 //0x28 battery board
#define AD7991_CTRLBRD 0b0101001 //0x29 control board

/*IR CONFIGURATION*/
//Flight Sensors
#define MLX90614_FLASHPANEL_V6_2_2	0x6A
#define MLX90614_ACCESSPANEL_V4_7	0x6B
#define MLX90614_SIDEPANEL_V5_5		0x5E
#define MLX90614_SIDEPANEL_V5_5B	0x5A
#define MLX90614_RBFPANEL_V1_1		0x6F
#define MLX90614_TOPPANEL_V5_1		0x7A

//Test Sensors
#define MLX90614_TBOARD_IR2			0x5B
#define MLX90614_DEFAULT_I2CADDR	0x5A
#define MLX90614_ACCESSPANEL_V4_6   0x5C
#define MLX90614_TOPPANEL_V4_1		0x6A
#define MLX90614_FLASHPANEL_V6_2_1	0x6C
#define MLX90614_RBFPANEL_V1_4		0x5F
#define MLX90614_SIDEPANEL_V5_2		0x6E
#define MLX90614_SIDEPANEL_V5_3		0x6D
#define MLX90614_TOPPANEL_V5_2		0x5D

// config for watchdog drivers
#define WATCHDOG_USE_EARLY_WARNING

//NEW SIDE PANEL: 0x5D
//NEW TOP PANEL:  0x5A

#endif /* CONFIG_H_ */
