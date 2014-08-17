/*! \file thread1.c
    \brief A thread that toggles the GREEN LED.
		\details Initialize and implement thread
*/

#include "cmsis_os.h"                                          // CMSIS RTOS header file
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
	
  while (1) {
    task1(); // thread code 
		
		if (addTrace("thread1 yields") != TRACE_OK)
		{
			stop_cpu;
		}
		
		osThreadSetPriority(tid_thread2, osPriorityNormal);
		
    osThreadYield();                                            // suspend thread
		
		if (addTrace("thread1 back from yield") != TRACE_OK)
		{
			stop_cpu;
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
	
	count1Sec();
}
