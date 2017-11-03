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

void trace_print(const char *format, ...)
{
	#if configUSE_TRACE_FACILITY == 1
		va_list arg;
		va_start (arg, format);
		vTracePrintF(global_trace_channel, format, arg);
		va_end (arg);
	#endif
}