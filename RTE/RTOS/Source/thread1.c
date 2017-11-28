/*! \file thread1.c
    \brief A thread that toggles the GREEN LED.
		\details Initialize and implement thread
*/

#include <string.h>
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
	const char init[] = "thread1 init";
	
	if (addTrace(init, strlen(init)) != TRACE_OK)
	{
		stop_cpu;
	}	
	
  tid_thread1 = osThreadCreate (osThread(thread1), NULL);
  if(!tid_thread1) return(-1);
	
  return(0);
}

/*!  
    \brief Terminating thread1
*/
int Terminate_thread1 (void) 
{	
	const char terminate_attempt[] = "thread1 terminate attempt";
	const char terminate_attempt_fail[] = "could not terminate thread1";
	
	if (addTraceProtected(terminate_attempt, strlen(terminate_attempt)) != TRACE_OK)
	{
		stop_cpu;
	}	
	
	if (osThreadTerminate(tid_thread1) != osOK)
	{
		if (addTraceProtected(terminate_attempt_fail, strlen(terminate_attempt_fail)) != TRACE_OK)
		{
			stop_cpu;
		}			
		
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
	char      message[MAX_STR_LEN];
	char     *message_ptr;
	
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
				memset(message, 0, sizeof(message));			
				snprintf(message, sizeof(message), "thread1 release sem0 fail rc=%d", os_rc);							
											
				if (addTraceProtected(message, strlen(message)) != TRACE_OK)
				{
					stop_cpu;
				}						
			}
		}
		else
		{
			ADD_TRACE_PROTECTED("thread1 release sem0 fail");			
		}	

		ADD_TRACE_PROTECTED("thread1 set thread0 priority to osPriorityNormal");	
		
		os_rc = osThreadSetPriority(tid_thread0, osPriorityNormal);
				
		memset(message, 0, sizeof(message));			
		snprintf(message, sizeof(message), "thread1 yields", os_rc);			
		
		if (addTraceProtected(message, strlen(message)) != TRACE_OK)
		{
			stop_cpu;
		}
		
    os_rc = osThreadYield();  // suspend thread

		memset(message, 0, sizeof(message));			
		snprintf(message, sizeof(message), "thread1 back from yield", os_rc);			
		
		if (addTraceProtected(message, strlen(message)) != TRACE_OK)
		{
			stop_cpu;
		}
		
		// This should terminate the current thread1 thread		
		if (Terminate_thread1() != 0)
		{
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
	const char task_work[] = "thread1 flips LED";
	
  LED_blink(LED1);
	if (addTraceProtected(task_work, strlen(task_work)) != TRACE_OK)
	{
		stop_cpu;
	}	
}
