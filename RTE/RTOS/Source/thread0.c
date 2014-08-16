/*! \file thread0.c
    \brief A thread that toggles the RED LED.
		\details Initialize and implement thread
*/

#include "cmsis_os.h"                                           // CMSIS RTOS header file
#include "peripherals.h" 
#include "osObjects.h"
#include "trace.h"

osThreadDef (thread0, osPriorityBelowNormal, 1, 100);

osThreadId tid_thread0;                                          ///< thread id
void task0(void);

/*! \fn int Init_thread0 (void)
    \brief Initializing thread0
*/
int Init_thread0 (void) 
{
  tid_thread0 = osThreadCreate (osThread(thread0), NULL);
  if(!tid_thread0) return(-1);
  
	if (addTrace("thread0 init") != TRACE_OK)
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
    
		task0(); // thread code 
		
		if (addTrace("thread0 yields") != TRACE_OK)
		{
			stop_cpu;
		}
		
    osThreadYield();  // suspend thread
		
		if (addTrace("thread0 back from yield") != TRACE_OK)
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
  if (osKernelSysTick() & 0x80) {LED_blink(LED0);} // Set   LED 0
  else                      {LED_blink(LED0);} // Clear LED 0
	
	count1Sec();
}
