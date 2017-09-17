/*
 * errors.h
 *
 * Created: 9/10/2017 2:48:18 PM
 *  Author: jleiken
 * 
 * Errors should be constructed by combining the location and the code
 * i.e. if TEMP_5 was observing a dangerous temperature, the code would be: 16,27
 * where each number is one byte. It would then be transmitted as 00010000 00011011
 */ 


#ifndef ERRORS_H_
#define ERRORS_H_

/******************** LOCATIONS ********************/
#define ELOC_IR_1						0
#define ELOC_IR_2						1
#define ELOC_IR_3						2
#define ELOC_IR_4						3
#define ELOC_IR_5						4
#define ELOC_IR_6						5

#define ELOC_PD_1						6
#define ELOC_PD_2						7
#define ELOC_PD_3						8
#define ELOC_PD_4						9
#define ELOC_PD_5						10
#define ELOC_PD_6						11

#define ELOC_TEMP_1						12
#define ELOC_TEMP_2						13
#define ELOC_TEMP_3						14
#define ELOC_TEMP_4						15
#define ELOC_TEMP_5						16
#define ELOC_TEMP_6						17
#define ELOC_TEMP_7						18
#define ELOC_TEMP_8						19
#define ELOC_RADIO_TEMP					20

#define ELOC_IMU_ACC					21
#define ELOC_IMU_GYRO					22
#define ELOC_IMU_MAG					23

#define ELOC_LED1SNS					24
#define ELOC_LED2SNS					25
#define ELOC_LED3SNS					26
#define ELOC_LED4SNS					27
#define ELOC_LFB1OSNS					28
#define ELOC_LFB1SNS					29
#define ELOC_LFB2OSNS					30
#define ELOC_LFB2SNS					31
#define ELOC_LF1REF						32
#define ELOC_LF2REF						33
#define ELOC_LF3REF						34
#define ELOC_LF4REF						35
#define ELOC_L1_REF						36
#define ELOC_L2_REF						37
#define ELOC_DET_RTN					38

/******************** PROBLEM CODES ********************/

/**** ATMEL DEFINED ****/
#define ECODE_OK						0
#define ECODE_VALID_DATA				1
#define ECODE_NO_CHANGE					2
#define ECODE_ABORTED					3
#define ECODE_BUSY						4
#define ECODE_SUSPEND					5
#define ECODE_IO						6
#define ECODE_REQ_FLUSHED				7
#define ECODE_TIMEOUT					8
#define ECODE_BAD_DATA					9
#define ECODE_NOT_FOUND					10
#define ECODE_UNSUPPORTED_DEV			11
#define ECODE_NO_MEMORY					12
#define ECODE_INVALID_ARG				13
#define ECODE_BAD_ADDRESS				14
#define ECODE_BAD_FORMAT				15
#define ECODE_BAD_FRQ					16
#define ECODE_DENIED					17
#define ECODE_ALREADY_INITIALIZED		18
#define ECODE_OVERFLOW					19
#define ECODE_NOT_INITIALIZED			20
#define ECODE_SAMPLERATE_UNAVAILABLE	21
#define ECODE_RESOLUTION_UNAVAILABLE	22
#define ECODE_BAUDRATE_UNAVAILABLE		23
#define ECODE_PACKET_COLLISION			24
#define ECODE_PROTOCOL					25
#define ECODE_PIN_MUX_INVALID			26

/**** CUSTOM ****/
#define ECODE_TEMP_HIGH					27
#define ECODE_TEMP_LOW					28
#define ECODE_SIGNAL_LOST				29

#endif /* ERRORS_H_ */
