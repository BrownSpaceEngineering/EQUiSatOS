#include "switching_commands.h"

//Target side: true for side 1, false for side 2
void pick_side(bool targetSide){
	if(targetSide){
		set_output(false,SIDE_1_ENABLE);
		set_output(true,SIDE_2_ENABLE);
	}else{
		set_output(false,SIDE_2_ENABLE);
		set_output(true,SIDE_1_ENABLE);
	}
}

void pick_input(uint8_t target){
	if(target <= 3){
		bool bit_low = target & 0x01;
		
		bool bit_high = target & 0x02;
	
		set_output(bit_low, BIT_0_CONTROL);
		set_output(bit_high, BIT_1_CONTROL);
	}
}

void setup_switching(void){
	setup_pin(true,SIDE_1_ENABLE);
	setup_pin(true,SIDE_2_ENABLE);
	setup_pin(true,BIT_0_CONTROL);
	setup_pin(true,BIT_1_CONTROL);
}