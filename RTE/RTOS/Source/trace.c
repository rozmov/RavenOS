/*! \file trace.c
    \brief This file defines tracing interfaces and tables
	  \details Provides tracing infrastructure for the RTOS
*/

#include "stdio.h"
#include <string.h>
#include <stdint.h>
#include "trace.h"

#define MAX_STR_LEN       64  ///< Maximum length of a trace message
#define MIN_TRACE_ARR_LEN  0  ///< Minimum size of the message buffer
#define MEX_TRACE_ARR_LEN 50  ///< Maximum size of the message buffer

char trace_table[MEX_TRACE_ARR_LEN][MAX_STR_LEN]; ///< The message table.
uint32_t trace_counter = MIN_TRACE_ARR_LEN;  ///< The message table counter.

/*! \fn uint32_t incrementTraceCounter()
    \brief Increment trace counter if trace table not full
		\return Returns \ref TRACE_OK if successful and \ref TRACE_ERROR  otherwise.
*/
uint32_t incrementTraceCounter()
{
	if (trace_counter + 1 == MAX_STR_LEN)
	{
		return TRACE_ERROR;
	}
	
	trace_counter++;
	return TRACE_OK;
}

/*! \fn uint32_t decrementTraceCounter()
    \brief Decrement trace counter if trace table not empty
		\return Returns \ref TRACE_OK if successful and \ref TRACE_ERROR  otherwise.
*/
uint32_t decrementTraceCounter()
{
	if (trace_counter == MIN_TRACE_ARR_LEN)
	{
		return TRACE_ERROR;
	}
	
	trace_counter--;
	return TRACE_OK;
}


/*! \fn uint32_t addTrace(char * message)
    \brief Add message to the trace table 
		\param message Message to be added to the trace table, up to \ref MAX_STR_LEN characters
		\return Returns \ref TRACE_OK if successful and \ref TRACE_ERROR otherwise.
*/
uint32_t addTrace(char * message)
{
	/// The length of a message needs to be small enough to leave room for a null string termination 
	if ( strlen(message) > (MAX_STR_LEN - 1) )
	{
		return TRACE_ERROR;
	}
	
	strcpy(trace_table[trace_counter], message);
	
	if (incrementTraceCounter() != TRACE_OK)
	{
		return TRACE_ERROR;
	}	
	
	return TRACE_OK;
}


/*! \fn
    \brief Dump messages from the trace table to output 
*/
void dumpTrace(void)
{
	uint32_t idx = 0;
	while (decrementTraceCounter() == TRACE_OK)
	{
		printf("%s\n\r",trace_table[idx]);
		memset(trace_table[idx],0,sizeof(trace_table[idx]));
    idx++;		
	}	
}

