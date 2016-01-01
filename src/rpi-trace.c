#include "rpi-trace.h"


#include <stdio.h>
#include <stdarg.h>


//
// Trace implementation
//
void trace(const char* fmt, ...)
{
	// Get system time. Wraps after about a day, so should include time_hi
	uint32_t time_lo = rpi_sys_timer->clo;

	// Calculate time parts
	time_lo /= 1000;
	uint32_t time_ms = time_lo % 1000;
	time_lo /= 1000;
	uint32_t time_se = time_lo % 60;
	time_lo /= 60;
	uint32_t time_mi = time_lo % 60;
	time_lo /= 60;
	uint32_t time_hr = time_lo % 24;

	// Write format string into buffer
	// TODO add time string
	char format_buf[1024];
	sprintf(format_buf, "%02u:%02u:%02u:%03u: %s\n", time_hr, time_mi, time_se, time_ms, fmt);

	// Format arguments into buffer
	char trace_buf[2048];
	va_list args;
	va_start(args, fmt);
	vsprintf(trace_buf, format_buf, args);
	va_end(args);

	// Write buffer to the UART
	printf(trace_buf);
}
