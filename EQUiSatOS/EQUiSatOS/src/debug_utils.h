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




/************************************************************************/
/* HELPER METHODS                                                       */
/************************************************************************/

#if configUSE_TRACE_FACILITY == 1
	traceString global_trace_channel;
#endif

void init_tracelyzer(void);
void trace_print(const char *format);

#endif /* DEBUG_UTILS_H */