/*! \file thread0.c
    \brief A thread that toggles the RED LED.
		\details Initialize and implement thread
*/

#include <string.h>
#include "peripherals.h" 
#include "osObjects.h"
#include "trace.h"

osThreadDef (thread0, osPriorityBelowNormal, 2, 100);  ///< thread definition

osThreadId tid_thread0;          ///< thread0 id
void task0(void);

/*! \fn int Init_thread0 (void)
    \brief Initializing thread0
*/
int Init_thread0 (void) 
{
  tid_thread0 = osThreadCreate (osThread(thread0), NULL);
	
  if (tid_thread0 == NULL) 
	{
		return(-1);
  }
	
	ADD_TRACE("thread0 init");
	
  return(0);
}

/*! \fn int Terminate_thread0 (void) 
    \brief Terminating thread0
		\return 0=successful; -1=failure
*/
int Terminate_thread0 (void) 
{	
	ADD_TRACE_PROTECTED("thread0 terminate attempt") ;
	
	if (osThreadTerminate(tid_thread0) != osOK)
	{
		ADD_TRACE_PROTECTED("could not terminate thread0") ;
		return(-1);
	}

  return(0);
}

/*! \fn void thread0 (void const *argument)
    \brief Thread definition for thread 0.
    \param argument A pointer to the list of arguments.
*/
void thread0 (void const *argument) 
{
	ADD_TRACE_PROTECTED("thread0 start run") ;
	
  while (1) 
	{	
		ADD_TRACE_PROTECTED("thread0 take sem0 attempt") ;
    if ( osSemaphoreWait (sid_Semaphore0, 0) != -1 ) // no wait
		{      
			ADD_TRACE_PROTECTED("thread0 take sem0 success") ;
			
			task0(); // thread code 
			count1Sec();
			task0();
			
			ADD_TRACE_PROTECTED("thread0 set priority to osPriorityLow") ;
			if (osThreadSetPriority(osThreadGetId(), osPriorityLow) != osOK)
			{
				ADD_TRACE_PROTECTED("thread0 could not set priority") ;
			}
							
			// suspend thread
			ADD_TRACE_PROTECTED("thread0 yields") ;
			if (osThreadYield() != osOK)
			{
				ADD_TRACE_PROTECTED("thread0 could not yield") ;
			}
				
			ADD_TRACE_PROTECTED("thread0 release sem0 attempt") ;
			if (osSemaphoreRelease (sid_Semaphore0) != osOK)
			{
				ADD_TRACE_PROTECTED("thread0 release sem0 fail") ;
			}
			else
			{
				ADD_TRACE_PROTECTED("thread0 release sem0 success") ;
			}			
		}
		else
		{
			ADD_TRACE_PROTECTED("thread0 take sem0 fail") ;
		}
		
		ADD_TRACE_PROTECTED("thread0 set priority to osPriorityLow") ;
		if (osThreadSetPriority(osThreadGetId(), osPriorityLow) != osOK)
		{
			ADD_TRACE_PROTECTED("thread0 could not set priority") ;
		}
		
		// suspend thread
		ADD_TRACE_PROTECTED("thread0 yields") ;
    if (osThreadYield() != osOK)
		{
			ADD_TRACE_PROTECTED("thread0 could not yield") ;
		}
		
		ADD_TRACE_PROTECTED("thread0 back from yield") ;
		
		ADD_TRACE_PROTECTED("thread0 delete sem0") ;
		if (Delete_Semaphore0() != 0)
		{
			stop_cpu;
		}			
				
    // This should terminate the current thread0 thread		
		if (Terminate_thread0() != 0)
		{
			stop_cpu;		
		}		
  }
}

// -------------------------------------------------------------------------
/*! \fn void task0(void)
    \brief Toggle LED #0
*/
void task0(void)
{ 
  LED_blink(LED0);
	ADD_TRACE_PROTECTED("thread0 flips LED") ;
}
