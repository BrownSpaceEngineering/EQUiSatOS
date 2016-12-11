/*
 * package_stack.h
 *
 * Created: 12/6/2016 8:47:29 PM
 *  Author: jleiken
 */ 


#ifndef PACKAGE_STACK_H_
#define PACKAGE_STACK_H_

#include <asf.h>
#include "equistacks.h"

uint8_t to_char_arr(void *input, int sizeof_input, int state, char *buffer);
#define CHECKSUM 1


#endif /* PACKAGE_STACK_H_ */