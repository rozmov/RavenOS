/*! \file trace.c
    \brief This file defines tracing interfaces and tables
	  \details Provides tracing infrastructure for the RTOS
*/

#include "stdio.h"
#include <string.h>
#include <stdint.h>
#include "trace.h"
#include "CU_TM4C123.h"

//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------

#define MIN_TRACE_ARR_LEN (0)  ///< Minimum size of the message buffer
//
//      <o> Maximum size of the buffer
//              <20=> 20
//              <30=> 30
//              <40=> 40
//              <50=> 50
//              <75=> 75
//              <100=> 100
//          <i> Specifies the maximum number of messages to be traced until trace is dumped.
//
#define MAX_TRACE_ARR_LEN (50)  ///< Maximum size of the message buffer

typedef struct {
	char trace[MAX_STR_LEN];
} trace_t;

trace_t trace_table[MAX_TRACE_ARR_LEN]; ///< The message table.
uint32_t trace_counter = MIN_TRACE_ARR_LEN;  ///< The message table counter.

/*! 
    \brief Getter function for the current trace counter 
		\return Returns current trace counter.
*/
uint32_t getTraceCounter(void)
{
	return trace_counter;
}

/*! 
    \brief Increment trace counter if trace table not full
		\return Returns \ref TRACE_OK if successful and \ref TRACE_ERROR  otherwise.
*/
uint32_t incrementTraceCounter(void)
{
	if (trace_counter + 1 == MAX_STR_LEN)
	{
		return TRACE_ERROR;
	}
	
	trace_counter++;
	return TRACE_OK;
}

/*! 
    \brief Decrement trace counter if trace table not empty
		\return Returns \ref TRACE_OK if successful and \ref TRACE_ERROR  otherwise.
*/
uint32_t decrementTraceCounter(void)
{
	if (trace_counter == MIN_TRACE_ARR_LEN)
	{
		return TRACE_ERROR;
	}
	
	trace_counter--;
	return TRACE_OK;
}

/*! 
    \brief Add message to the trace table 
		\param message Message to be added to the trace table, up to \ref MAX_STR_LEN characters
		\return Returns \ref TRACE_OK if successful and \ref TRACE_ERROR otherwise.
*/
uint32_t addTrace(const char * message, uint32_t length)
{
	if (strlen(message) > length)
	{
		return TRACE_ERROR;
	}
	
	/// The length of a message needs to be small enough to leave room for 
	/// a null string termination and LF/CR
	if ( length > (MAX_STR_LEN - 3) )
	{
		return TRACE_ERROR;
	}
	
	memset(trace_table[trace_counter].trace, 0, sizeof(trace_table[trace_counter].trace));
	strcpy(trace_table[trace_counter].trace, message);
	
	if (incrementTraceCounter() != TRACE_OK)
	{
		return TRACE_ERROR;
	}	
	
	return TRACE_OK;
}

/*!
    \brief Dump messages from the trace table to output 
*/
void dumpTrace(void)
{
	uint32_t idx = 0;
	
	while (decrementTraceCounter() == TRACE_OK)
	{
		printf("%s\n\r",trace_table[idx].trace);
		memset(trace_table[idx].trace, 0, sizeof(trace_table[idx]));
    idx++;		
	}	
}
