/*! \file thread0.c
    \brief A thread that toggles the RED LED.
		\details Initialize and implement thread
*/

#include "peripherals.h" 
#include "osObjects.h"
#include "trace.h"

osThreadDef (thread0, osPriorityBelowNormal, 2, 100);

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
	
	if (addTrace("thread0 init") != TRACE_OK)
	{
		stop_cpu;
	}
	
  return(0);
}

/*! \fn int Terminate_thread0 (void) 
    \brief Terminating thread0
		\return 0=successful; -1=failure
*/
int Terminate_thread0 (void) 
{	
	if (osThreadTerminate(tid_thread0) != osOK)
	{
		if (addTrace("could not terminate thread0") != TRACE_OK)
		{
			stop_cpu;
		}			
		return(-1);
	}

	if (addTrace("terminated thread0") != TRACE_OK)
	{
		stop_cpu;
	}		
  return(0);
}

/*! \fn void thread0 (void const *argument)
    \brief Thread definition for thread 0.
    \param argument A pointer to the list of arguments.
*/
void thread0 (void const *argument) 
{
	
	if (addTrace("thread0 start run") != TRACE_OK)
	{
		stop_cpu;
	}
	
  while (1) 
	{	
		
		if (addTrace("thread0 take sem0 attempt") != TRACE_OK)
		{
			stop_cpu;
		}			
    if ( osSemaphoreWait (sid_Semaphore0, 0) != -1 ) // wait 0 mSec
		{
      task0(); // thread code 
			if (addTrace("thread0 take sem0 success") != TRACE_OK)
			{
				stop_cpu;
			}	

//			count1Sec();
			
			if (addTrace("thread0 set priority to osPriorityLow") != TRACE_OK)
			{
				stop_cpu;
			}			
			osThreadSetPriority(osThreadGetId(), osPriorityLow);
							
			
			if (addTrace("thread0 yields") != TRACE_OK)
			{
				stop_cpu;
			}	
			osThreadYield();  // suspend thread			
				
			if (addTrace("thread0 release sem0 attempt") != TRACE_OK)
			{
				stop_cpu;
			}				
			if (osSemaphoreRelease (sid_Semaphore0) != osOK)
			{
				if (addTrace("thread0 release sem0 fail") != TRACE_OK)
				{
					stop_cpu;
				}						
			}
			else
			{
				if (addTrace("thread0 release sem0 success") != TRACE_OK)
				{
					stop_cpu;
				}					
			}
			
		}
		else
		{
			if (addTrace("thread0 take sem0 fail") != TRACE_OK)
			{
				stop_cpu;
			}				
		}

//		count1Sec();
		
		if (addTrace("thread0 set priority to osPriorityLow") != TRACE_OK)
		{
			stop_cpu;
		}			
		osThreadSetPriority(osThreadGetId(), osPriorityLow);
		    
		
		if (addTrace("thread0 yields") != TRACE_OK)
		{
			stop_cpu;
		}	
    osThreadYield();  // suspend thread
		
		if (addTrace("thread0 back from yield") != TRACE_OK)
		{
			stop_cpu;
		}		
		
		if (addTrace("thread0 delete sem0") != TRACE_OK)
		{
			stop_cpu;
		}				
		if (Delete_Semaphore0() != 0)
		{
			stop_cpu;
		}			
				
    // This should terminate the current thread0 thread		
		if (Terminate_thread0() != 0)
		{
			while(1)
			{		
					// Should not be here
			}			
		}
		
  }
}

// -------------------------------------------------------------------------
/*! \fn void task0(void)
    \brief Toggle LED #0
*/
void task0(void)
{ 
  if (osKernelSysTick() & 0x80) {LED_blink(LED0);} // Set   LED 0
  else                      {LED_blink(LED0);} // Clear LED 0
	if (addTrace("thread0 flips LED") != TRACE_OK)
	{
		stop_cpu;
	}		
}
