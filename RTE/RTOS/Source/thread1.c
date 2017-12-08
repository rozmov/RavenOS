/*! \file thread1.c
    \brief A thread that toggles the GREEN LED.
		\details Initialize and implement thread
*/

#include <string.h>
#include "stdio.h"
#include "peripherals.h" 
#include "osObjects.h"
#include "trace.h"

osThreadDef (thread1, osPriorityBelowNormal, 1, 100);  ///< thread definition

osThreadId tid_thread1;     ///< thread id
void task1(void);

/*! 
    \brief Initializing thread1
*/
int Init_thread1 (void) 
{
	ADD_TRACE("thread1 init");
	
  tid_thread1 = osThreadCreate (osThread(thread1), NULL);
  if(!tid_thread1) return(-1);
	
  return(0);
}

/*!  
    \brief Terminating thread1
*/
int Terminate_thread1 (void) 
{	
	ADD_TRACE_PROTECTED("thread1 terminate attempt") ;
	
	if (osThreadTerminate(tid_thread1) != osOK)
	{
		ADD_TRACE_PROTECTED("could not terminate thread1");
		
		return(-1);
	}
		
  return(0);
}

/*! 
    \brief Thread definition for thread 1.
    \param argument A pointer to the list of arguments.
*/
void thread1 (void const *argument) 
{	
	osStatus  os_rc;

	ADD_TRACE_PROTECTED("thread1 start run");
	
  while (1) 
	{	
		ADD_TRACE_PROTECTED("thread1 take sem0 attempt");
		
    if ( osSemaphoreWait (sid_Semaphore0, 1100) != -1 ) // wait mSec
		{					
			ADD_TRACE_PROTECTED("thread1 take sem0 success");
			
			task1();     // thread code 
  		count1Sec();			
			task1();
			
			ADD_TRACE_PROTECTED("thread1 release sem0 attempt");
			
			os_rc = osSemaphoreRelease (sid_Semaphore0);
			
			if (os_rc != osOK)
			{
				ADD_TRACE_PROTECTED("thread1 release sem0 fail");
			}
		}
		else
		{
			ADD_TRACE_PROTECTED("thread1 release sem0 fail");			
		}	

		ADD_TRACE_PROTECTED("thread1 set thread0 priority to osPriorityNormal");	
		
		os_rc = osThreadSetPriority(tid_thread0, osPriorityNormal);
				
		ADD_TRACE_PROTECTED("thread1 back from set thread0 priority to osPriorityNormal");
		
		ADD_TRACE_PROTECTED("thread1 yields");
		
    os_rc = osThreadYield();  // suspend thread

		ADD_TRACE_PROTECTED("thread1 back from yield");			
		
		ADD_TRACE_PROTECTED("thread1 finish");
		
		// This should terminate the current thread1 thread		
		if (Terminate_thread1() != 0)
		{
			ADD_TRACE_PROTECTED("thread1 finish fail");
			
			while(1)
			{		
					// Should not be here
			}			
		}
  }
}

// -------------------------------------------------------------------------
/*! 
    \brief Toggle LED #1
*/
void task1(void)
{
  LED_blink(LED1);
	
	ADD_TRACE_PROTECTED("thread1 flips LED");
}
