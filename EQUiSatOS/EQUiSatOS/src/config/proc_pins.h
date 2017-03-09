/*
 * proc_pins.h
 *
 * Created: 3/8/2017 8:10:30 PM
 *  Author: Tyler
 */ 


#ifndef PROC_PINS_H_
#define PROC_PINS_H_

#define CNTRL_BRD_V3

#ifdef CNTRL_BRD_V3
	//Batteries
	#define L1_RUN_CHG		PIN_PA01
	#define L2_RUN_CHG		PIN_PB04
	#define LF_B1_RUNCHG	PIN_PB05
	#define LF_B2_RUNCHG	PIN_PB10
	#define L1_DISG			PIN_PB31 //active low
	#define L2_DISG			PIN_PA00 //active low
	#define LF_B1_OUTEN		PIN_PB06
	#define LFB1OSNS		PIN_PA11
	#define LFB1SNS			PIN_PA10
	#define LF_B2_OUTEN		PIN_PB07
	#define LFB2OSNS		PIN_PA09
	#define LFB2SNS			PIN_PA08
	#define LF1REF			PIN_PA06
	#define LF2REF			PIN_PA07
	#define LF3REF			PIN_PA08
	#define LF4REF			PIN_PA09
	#define L1_REF			PIN_PA03
	#define L2_REF			PIN_PA07
	
	//I2C
	#define P_SDA			PIN_PB12
	#define P_SCL			PIN_PB13
	
	//Radio
	#define P_TX_EN			PIN_PA23
	#define P_TX_DATA		PIN_PA24
	#define P_RX_EN			PIN_PA22
	#define P_RX_DATA		PIN_PA25
	#define P_RAD_SHDN		PIN_PA21 //active low (to shutdown)
	#define P_RAD_PWR_RUN	PIN_PA20
	
	//Sensors
	#define P_IR_PWR_CMD	PIN_PB17
	#define P_TEMP_OUT		PIN_PB08
	#define P_PD_OUT		
	
	//MRAM
	#define P_SO			PIN_PA16
	#define P_CS			PIN_PA17 //active low
	#define P_SI			PIN_PA18
	#define P_SCK			PIN_PA19
	#define P_CS2			PIN_PA16 //active low
	
	
	//Antenna Deploy
	#define P_ANT_DRV1		PIN_PA13
	#define P_ANT_DRV2		PIN_PA14
	#define P_ANT_DRV3		PIN_PA15
	#define P_5V_EN			PIN_PB11
	#define P_DET_RTN		PIN_PB14
	
	//LED
	#define P_LED_CMD		PIN_PA12 //active low
	#define P_LED1SNS		PIN_PB03
	#define P_LED2SNS		PIN_PB02
	#define P_LED3SNS		PIN_PB01
	#define P_LED4SNS		PIN_PB00
			
	

#endif




#endif /* PROC_PINS_H_ */