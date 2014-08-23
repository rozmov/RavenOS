/*! \file thread1.c
    \brief A thread that toggles the GREEN LED.
		\details Initialize and implement thread
*/

#include "peripherals.h" 
#include "osObjects.h"
#include "trace.h"

osThreadDef (thread1, osPriorityBelowNormal, 1, 100);

osThreadId tid_thread1;     ///< thread id
void task1(void);

/*! \fn int Init_thread1 (void)
    \brief Initializing thread1
*/
int Init_thread1 (void) 
{
  tid_thread1 = osThreadCreate (osThread(thread1), NULL);
  if(!tid_thread1) return(-1);
  
	if (addTrace("thread1 init") != TRACE_OK)
	{
		stop_cpu;
	}
	
  return(0);
}

/*! \fn int Terminate_thread1 (void) 
    \brief Terminating thread1
*/
int Terminate_thread1 (void) 
{	
	if (osThreadTerminate(tid_thread1) != osOK)
	{
		if (addTrace("could not terminate thread1") != TRACE_OK)
		{
			dumpTrace();
			addTrace("could not terminate thread1") ;
		}			
		return(-1);
	}

	if (addTrace("terminated thread1") != TRACE_OK)
	{
		dumpTrace();
		addTrace("terminated thread1") ;
	}		
  return(0);
}

/*! \fn void thread1 (void const *argument)
    \brief Thread definition for thread 1.
    \param argument A pointer to the list of arguments.
*/
void thread1 (void const *argument) 
{	
	if (addTrace("thread1 start run") != TRACE_OK)
	{
		stop_cpu;
	}
	
  while (1) 
	{	
		if (addTrace("thread1 take sem0 attempt") != TRACE_OK)
		{
			stop_cpu;
		}			
    if ( osSemaphoreWait (sid_Semaphore0, 100) != -1 ) // wait mSec
		{		
			task1(); // thread code 
			if (addTrace("thread1 take sem0 success; now releasing") != TRACE_OK)
			{
				stop_cpu;
			}	
			if (osSemaphoreRelease (sid_Semaphore0) != osOK)
			{
				if (addTrace("thread1 release sem0 fail") != TRACE_OK)
				{
					stop_cpu;
				}						
			}
		}
		else
		{
			if (addTrace("thread1 take sem0 fail") != TRACE_OK)
			{
				stop_cpu;
			}				
		}

//		count1Sec();
		
		
		if (addTrace("thread1 set thread0 priority to osPriorityNormal") != TRACE_OK)
		{
			stop_cpu;
		}			
		osThreadSetPriority(tid_thread2, osPriorityNormal);
				
		if (addTrace("thread1 yields") != TRACE_OK)
		{
			stop_cpu;
		}
    osThreadYield();                                            // suspend thread
		
		if (addTrace("thread1 back from yield") != TRACE_OK)
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
/*! \fn void task1(void)
    \brief Toggle LED #1
*/
void task1(void)
{
  if (osKernelSysTick() & 0x100) {LED_blink(LED1);} // Set   LED 1
  else                       {LED_blink(LED1);} // Clear LED 1
	if (addTrace("thread1 flips LED") != TRACE_OK)
	{
		stop_cpu;
	}	
}
