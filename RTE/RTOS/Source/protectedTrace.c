/*! \file protectedTrace.c
    \brief This file defines thread-safe tracing interfaces 
	  \details Provides thread-safe tracing infrastructure for the RTOS based on regular tracing
*/

#include "stdio.h"
#include <string.h>
#include <stdint.h>
#include "trace.h"
#include "osObjects.h"

//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
//--------------------- Protected Trace Configuration ----------------------------------
//
//  <e> Trace Configuration
//          <i> Uncheck this box to skip the trace printing.
//
# define TRACE_FLAG 1   ///< trace printing flag: 1 = print traces; 0 = do no print traces

/*! 
    \brief Thread-safe add message to the trace table 
		\param message Message to be added to the trace table, up to \ref MAX_STR_LEN characters
		\return Returns \ref TRACE_OK if successful and \ref TRACE_ERROR otherwise.
*/
uint32_t addTraceProtected(char * message)
{
	if (TRACE_FLAG != 1)
	{
		return TRACE_OK;
	}
	
	if ( osSemaphoreWait (sid_Semaphore1, osWaitForever) != -1 ) // wait forever
	{					 
		
		if (addTrace(message) != TRACE_OK)
		{
			stop_cpu;
		}	
		
		if (osSemaphoreRelease (sid_Semaphore1) != osOK)
		{
			if (addTrace(" release sem1 fail") != TRACE_OK)
			{
				stop_cpu;
			}						
		}				
	}
	else
	{
			stop_cpu;				
	}
	
	return TRACE_OK;
}


/*!
    \brief Dump messages from the trace table to output 
*/
void dumpTraceProtected(void)
{
	if (TRACE_FLAG != 1)
	{
		return ;
	}	
	
	if ( osSemaphoreWait (sid_Semaphore1, osWaitForever) != -1 ) // wait forever
	{					 
		dumpTrace();
		
		if (osSemaphoreRelease (sid_Semaphore1) != osOK)
		{
			if (addTrace(" release sem1 fail") != TRACE_OK)
			{
				stop_cpu;
			}						
		}				
	}
	else
	{
		stop_cpu;					
	}
	
	return;
}


