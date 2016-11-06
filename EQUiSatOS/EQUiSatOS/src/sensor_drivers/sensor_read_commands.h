/*
 * sensor_read_commands.h
 *
 * Created: 11/1/2016 8:17:44 PM
 *  Author: mckenna
 */ 

#ifndef SENSOR_READ_COMMANDS_H
#define	SENSOR_READ_COMMANDS_H

#include "../radio/Stacks/Sensor_Structs.h"

ir_batch read_ir_batch();
temp_batch read_temp_batch();

#endif