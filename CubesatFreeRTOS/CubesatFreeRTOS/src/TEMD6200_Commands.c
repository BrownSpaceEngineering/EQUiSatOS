/*
 * TEMD21_Commands.c
 *
 * Created: 2/3/2016 8:29:56 PM
 *  Author: Tyler
 */ 

#include "TEMD6200_commands.h"

int isBright(struct adc_module adc_reader){
	float voltage = readVoltagemV(adc_reader);
	if(voltage > TEMD6200_VOLTAGE_LIMIT_MV){
		return true;
	}else{
		return false;
	}
}