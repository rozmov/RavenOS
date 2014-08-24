/*! \file thread3.c
    \brief A thread that prints trace information to UART.
		\details Initialize and implement thread
*/

#include "trace.h"
#include "osObjects.h"

osThreadDef (thread3, osPriorityLow, 1, 100);

osThreadId tid_thread3;   ///< thread3 id
void task3(void);

/*! \fn int Init_thread3 (void)
    \brief Initializing thread 3
		\return 0=successful; -1=failure
*/
int Init_thread3 (void) 
{
  tid_thread3 = osThreadCreate (osThread(thread3), NULL);
  if(!tid_thread3) return(-1);
	
  if (addTrace("thread3 init") != TRACE_OK)
	{
		stop_cpu;
	}
  return(0);
}

/*! \fn int Terminate_thread3 (void) 
    \brief Terminating thread3
		\return 0=successful; -1=failure
*/
int Terminate_thread3 (void) 
{	
	dumpTrace();
	
	if (osThreadTerminate(tid_thread3) != osOK)
	{		
		return(-1);
	}		
  return(0);
}

/*! \fn void thread3 (void const *argument)
    \brief Thread definition for thread 3.
    \param argument A pointer to the list of arguments.
*/
void thread3 (void const *argument) 
{
	if (addTrace("thread3 start run") != TRACE_OK)
	{
		stop_cpu;
	}
	
  while (1) /// \todo may need a throttle mechanism in place to stop this thread from running if no other thread (but Idle) present
	{
    task3(); // thread code 
		
		// give a chance to the other tasks to run now
		if (addTrace("thread3 set priority to osPriorityLow") != TRACE_OK)
		{
			stop_cpu;
		}		
		osThreadSetPriority(osThreadGetId(), osPriorityLow);
		
		if (addTrace("thread3 set priority of thread0 to osPriorityNormal") != TRACE_OK)
		{
			stop_cpu;
		}
		osThreadSetPriority(tid_thread0, osPriorityNormal);
		
		
		if (addTrace("thread3 yields") != TRACE_OK)
		{
			stop_cpu;
		}		
    osThreadYield();                                            // suspend thread
		
		if (addTrace("thread3 back") != TRACE_OK)
		{
			stop_cpu;
		}
  }
}

// -------------------------------------------------------------------------
/*! \fn void task3(void)
    \brief Print trace information to UART
*/
void task3(void)
{ 
	/// Print trace information
	dumpTrace();
	if (addTrace("thread3 dumped trace") != TRACE_OK)
	{
		stop_cpu;
	}
}
