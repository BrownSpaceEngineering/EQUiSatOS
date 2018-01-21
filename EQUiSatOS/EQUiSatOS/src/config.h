/*
 * config.h
 *
 * Created: 11/17/2017 8:02:34 PM
 *  Author: Tyler
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_

#include "FreeRTOSConfig.h"

/*************************************************************************/
/*							CONFIG OPTIONS                               */
/*************************************************************************/

//Choose 1 Config: Either FLIGHT, FLATSAT or XPLAINED
#define FLIGHT

/** 
 * flags to control dangerous / inconvenient functions of the satellite 
 * MAKE SURE to RE-build solution to be double-sure they were disabled
 */
//#define FLASH_ACTIVE // enable flashing
//#define ANTENNA_DEPLOY_ACTIVE // enable antenna deployment
//#define WATCHDOG_RESET_ACTIVE // enable watchdog actually rebooting satellite

/*** TESTING / DEBUG ***/
// define this flag to disable normal frequencies and read data faster
// (ex: don't read data distributed around an orbit)
#define TESTING_SPEEDUP

// whether to start up testing tasks
//#define RUN_TESTING_TASKS

//0 to suppress prints, 1 to print over USART
#define PRINT_DEBUG 1

/****************************************************************************************/
/*  DO NOT EDIT BELOW THIS LINE UNLESS YOU ARE INTENTIONALLY MODIFYING CONFIGURATION    */
/****************************************************************************************/

#ifdef FLIGHT
	//Sets processor pinouts
	#define	CTRL_BRD_V3
	
	//Flight IR sensors
	#define	IR_FLASH	MLX90614_FLASHPANEL_V6_2_2
	#define	IR_SIDE1	MLX90614_SIDEPANEL_V5_2
	#define	IR_SIDE2	MLX90614_SIDEPANEL_V5_5
	#define	IR_RBF		MLX90614_RBFPANEL_V1
	#define IR_ACCESS	MLX90614_ACCESSPANEL_V4_7
	#define IR_TOP1		MLX90614_TOPPANEL_V5_1
	
#endif
#ifdef FLATSAT
	//Sets processor pinouts
	#define	CTRL_BRD_V3

	/*Flatsat IR sensors*/
	#define	IR_FLASH	MLX90614_FLASHPANEL_V6_2_1	
	#define	IR_SIDE1	MLX90614_SIDEPANEL_V4_2	
	#define	IR_SIDE2	MLX90614_SIDEPANEL_V5_2
	#define	IR_RBF		MLX90614_RBFPANEL_V1
	#define IR_ACCESS	MLX90614_ACCESSPANEL_V4_6
	#define IR_TOP1		MLX90614_TOPPANEL_V4_1
	
#endif	
#ifdef XPLAINED	
	//Sets processor pinouts
	#define	CTRL_BRD_V3

	/*Flatsat IR sensors (because we have nothing else to do) */
	#define	IR_FLASH	MLX90614_FLASHPANEL_V6_2_1
	#define	IR_SIDE1	MLX90614_SIDEPANEL_V4_2
	#define	IR_SIDE2	MLX90614_SIDEPANEL_V5_2
	#define	IR_RBF		MLX90614_RBFPANEL_V1
	#define IR_ACCESS	MLX90614_ACCESSPANEL_V4_6
	#define IR_TOP1		MLX90614_TOPPANEL_V4_1
#endif

#if configUSE_TRACE_FACILITY == 1
	traceString global_trace_channel;
#endif


/****************************************************************************************/
/*					 SENSOR ADDRESSES BELOW THIS LINE - DO NOT EDIT					    */
/****************************************************************************************/

//Remote ADCs
#define AD7991_BATBRD 0b0101000 //0x28 battery board
#define AD7991_CTRLBRD 0b0101001 //0x29 control board

/*IR CONFIGURATION*/
//Test Sensors
#define MLX90614_TBOARD_IR2			0x5B
#define MLX90614_DEFAULT_I2CADDR	0x5A
#define MLX90614_SIDEPANEL_V4_2		0x5D // probably
#define MLX90614_ACCESSPANEL_V4_6   0x5C
#define MLX90614_TOPPANEL_V4_1		0x6A
#define MLX90614_FLASHPANEL_V6_2_1	0x6C
//Flight Sensors
#define MLX90614_FLASHPANEL_V6_2_2	0x6A
#define MLX90614_ACCESSPANEL_V4_7	0x6B
#define MLX90614_SIDEPANEL_V5_5		0x5E
#define MLX90614_SIDEPANEL_V5_2		0x6E
#define MLX90614_RBFPANEL_V1		0x5F
#define MLX90614_TOPPANEL_V5_1		0x7A

#endif /* CONFIG_H_ */