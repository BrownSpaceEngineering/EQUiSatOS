/*
 * trace_utils.h
 *
 * Created: 11/2/2017 20:38:41
 *  Author: mcken
 */ 

#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

#include "FreeRTOSConfig.h"

/************************************************************************/
/* DEBUG CONFIGURATION                                                  */
/************************************************************************/

// BOARD TYPE - ALSO LOOK AT SERCOM_PINOUT.H, it has to have it's own version changed too!!!
#define CNTRL_BRD_V3 // to use print, etc. on real control board
//#define XPLAINED_PRO  // to use print, etc. on devboard

#ifdef XPLAINED_PRO
	#define PRINT_DEBUG 1  // change to 1 if print debug on DEV BOARD is desired
#endif
#ifdef CNTRL_BRD_V3
	#define PRINT_DEBUG 1  // change to 1 if print debug on REAL CONTROL BOARD is desired
#endif


/************************************************************************/
/* HELPER METHODS                                                       */
/************************************************************************/

#if configUSE_TRACE_FACILITY == 1
	traceString global_trace_channel;
#endif

void init_tracelyzer(void);
void trace_print(const char *format);

#endif /* DEBUG_UTILS_H */