/*! \file threadIdle.c
    \brief Idle thread.
    \details Initialize and implement idle thread. Which should be scheduled even if other threads are running.
*/

#include "cmsis_os.h"                                           // CMSIS RTOS header file
#include "trace.h"
#include "threadIdle.h"

osThreadDef (threadIdle, osPriorityIdle, 1, 100);
osThreadId tid_threadIdle;        // thread id

/*! \fn int Init_threadIdle (void)
    \brief Initializing Idle thread 
*/
int Init_threadIdle (void) 
{
  tid_threadIdle = osThreadCreate (osThread(threadIdle), NULL);
  if(!tid_threadIdle) return(-1);

	if (addTrace("threadIdle init") != TRACE_OK)
	{
		while (1) 
		{
			
		}
	}	
	
  return(0);
}

/*! \fn void threadIdle (void const *argument)
    \brief Thread definition for the idle thread.
    \param argument A pointer to the list of arguments.
*/
void threadIdle (void const *argument) 
{
	if (addTrace("threadIdle start run") != TRACE_OK)
	{
		while (1) 
		{
			
		}
	}
	
  while (1) 
	{
		
		if (addTrace("threadIdle yields") != TRACE_OK)
		{
			while (1) 
			{
				
			}
		}		
    osThreadYield();                                            // suspend thread
		
		if (addTrace("threadIdle back from yield") != TRACE_OK)
		{
			while (1) 
			{
				
			}
		}
		
  }
}
