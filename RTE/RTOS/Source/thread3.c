/*! \file thread3.c
    \brief A thread that prints trace information to UART.
		\details Initialize and implement thread
*/

#include "trace.h"
#include "osObjects.h"

osThreadDef (thread3, osPriorityLow, 1, 100);  ///< thread definition

osThreadId tid_thread3;   ///< thread3 id
void task3(void);

/*! 
    \brief Initializing thread 3
		\return 0=successful; -1=failure
*/
int Init_thread3 (void) 
{
	if (addTrace("thread3 init") != TRACE_OK)
	{
		stop_cpu;
	}	
	
  tid_thread3 = osThreadCreate (osThread(thread3), NULL);
  if(!tid_thread3) return(-1);

  return(0);
}

/*!  
    \brief Terminating thread3
		\return 0=successful; -1=failure
*/
int Terminate_thread3 (void) 
{	
	dumpTraceProtected();
	
	if (osThreadTerminate(tid_thread3) != osOK)
	{		
		return(-1);
	}		
  return(0);
}

/*! 
    \brief Thread definition for thread 3.
    \param argument A pointer to the list of arguments.
*/
void thread3 (void const *argument) 
{
	if (addTraceProtected("thread3 start run") != TRACE_OK)
	{
		stop_cpu;
	}
	
  while (1) 
	{
    task3(); // thread code 
		
		// give a chance to the other tasks to run now
		if (addTraceProtected("thread3 set priority to osPriorityLow") != TRACE_OK)
		{
			stop_cpu;
		}		
		osThreadSetPriority(osThreadGetId(), osPriorityLow);
		
		if (addTraceProtected("thread3 yields") != TRACE_OK)
		{
			stop_cpu;
		}
		osThreadYield(); 
		
		// throttle mechanism in place to stop this thread from running if no other traces added but its own
		while (getTraceCounter() <= 4)
		{
			osThreadYield();             // suspend thread
		}
		
		if (addTraceProtected("thread3 back") != TRACE_OK)
		{
			stop_cpu;
		}
		
  }
}

// -------------------------------------------------------------------------
/*! 
    \brief Print trace information to UART
*/
void task3(void)
{ 
	/// Print trace information
	dumpTraceProtected();
	if (addTraceProtected("thread3 dumped trace") != TRACE_OK)
	{
		stop_cpu;
	}
}
