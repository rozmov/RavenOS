/*! \file thread2.c
    \brief A thread that toggles the BLUE LED.
		\details Initialize and implement thread
*/

#include "peripherals.h" 
#include "osObjects.h"
#include "trace.h"

osThreadDef (thread2, osPriorityBelowNormal, 1, 100);  ///< thread definition

osThreadId tid_thread2;                                ///< thread id
void task2(void);

/*! \fn int Init_thread2 (void)
    \brief Initializing thread2
*/
int Init_thread2 (void) 
{
	if (addTrace("thread2 terminate attempt") != TRACE_OK)
	{
		stop_cpu;
	}	
	
  tid_thread2 = osThreadCreate (osThread(thread2), NULL);
  if(!tid_thread2) return(-1);
  
  return(0);
}

/*! \fn int Terminate_thread2 (void) 
    \brief Terminating thread2
*/
int Terminate_thread2 (void) 
{	
	if (osThreadTerminate(tid_thread2) != osOK)
	{
		if (addTrace("could not terminate thread2") != TRACE_OK)
		{
			dumpTrace();
			addTrace("could not terminate thread2") ;
		}			
		return(-1);
	}

	if (addTrace("terminated thread2") != TRACE_OK)
	{
		dumpTrace();
		addTrace("terminated thread2") ;
	}		
  return(0);
}

/*! \fn void thread2 (void const *argument)
    \brief Thread definition for thread 2.
    \param argument A pointer to the list of arguments.
*/
void thread2 (void const *argument) 
{
  if (addTrace("thread2 start run") != TRACE_OK)
	{
		stop_cpu;
	}
	
  while (1) 
	{		
		if (addTrace("thread2 take sem0 attempt") != TRACE_OK)
		{
			stop_cpu;
		}			
    if ( osSemaphoreWait (sid_Semaphore0, osWaitForever) != -1 ) // wait forever
		{					 
			if (addTrace("thread2 take sem0 success; now releasing") != TRACE_OK)
			{
				stop_cpu;
			}	
			
			task2(); // thread code
			count1Sec();
			task2();
			
			if (addTrace("thread2 release sem0 attempt") != TRACE_OK)
			{
				stop_cpu;
			}				
			if (osSemaphoreRelease (sid_Semaphore0) != osOK)
			{
				if (addTrace("thread2 release sem0 fail") != TRACE_OK)
				{
					stop_cpu;
				}						
			}
		}
		else
		{
			if (addTrace("thread2 take sem0 fail") != TRACE_OK)
			{
				stop_cpu;
			}				
		}

		if (addTrace("thread2 set priority to osPriorityBelowNormal") != TRACE_OK)
		{
			stop_cpu;
		}		
		osThreadSetPriority(osThreadGetId(), osPriorityBelowNormal);
		
		if (addTrace("thread2 yields") != TRACE_OK)
		{
			stop_cpu;
		}		
    osThreadYield();                                            // suspend thread
		
		if (addTrace("thread2 back from yield") != TRACE_OK)
		{
			stop_cpu;
		}
		
		// This should terminate the current thread2 thread		
		if (Terminate_thread2() != 0)
		{
			stop_cpu;		
		}
		
  }
}

// -------------------------------------------------------------------------
/*! \fn void task2(void)
    \brief Toggle LED #2
*/
void task2(void)
{ 
  if (osKernelSysTick() & 0x200) {LED_blink(LED2);} // Set   LED 2
  else                       {LED_blink(LED2);} // Clear LED 2
	
	if (addTrace("thread2 flips LED") != TRACE_OK)
	{
		stop_cpu;
	}
}
