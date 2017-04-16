/*
 * global.h
 *
 * Created: 4/12/2017 9:35:47 PM
 *  Author: jleiken
 */ 


#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <asf.h>

typedef struct return_struct_float {
	enum status_code return_status;
	float return_value;
} return_struct_float;

typedef struct return_struct_16 {
	enum status_code return_status;
	uint16_t return_value;
} return_struct_16;
	
typedef struct return_struct_8 {
	enum status_code return_status;
	uint8_t return_value;
} return_struct_8;
	
typedef struct return_struct_0 {
	enum status_code return_status;
} return_struct_0;


#endif /* GLOBAL_H_ */