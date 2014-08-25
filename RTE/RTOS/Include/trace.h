/*! \file trace.h
    \brief This header file defines visible tracing infrastructures
	  \details Provides tracing infrastructure for the RTOS
*/

#ifndef _TRACE_H
#define _TRACE_H

#include <stdint.h>

#define TRACE_OK 0    ///< Trace return code - success
#define TRACE_ERROR 1 ///< Trace return code - fail

uint32_t addTraceProtected(char * message);
void dumpTraceProtected(void);

uint32_t addTrace(char * message);
void dumpTrace(void);
uint32_t getTraceCounter(void);

#endif // _TRACE_H
