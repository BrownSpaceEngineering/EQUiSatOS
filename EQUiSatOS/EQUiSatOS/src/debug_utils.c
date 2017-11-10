/*
 * debug_utils.c
 *
 * Created: 11/2/2017 20:41:56
 *  Author: mcken
 */ 

#include "debug_utils.h"

/* Start and init tracalyzer tracing */
void init_tracelyzer(void) {
	#if configUSE_TRACE_FACILITY == 1
		vTraceEnable(TRC_START);
		global_trace_channel = xTraceRegisterString("Global Events");
	#endif
}

void trace_print(const char *format)
{
	// NOTE: va_args won't work; no function that takes that :(
	#if configUSE_TRACE_FACILITY == 1
		vTracePrintF(global_trace_channel, format);
	#endif
}