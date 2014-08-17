/*! \file thread3.c
    \brief A thread that prints trace information to UART.
		\details Initialize and implement thread
*/

#include "cmsis_os.h"                                           // CMSIS RTOS header file
#include "trace.h"
#include "osObjects.h"

osThreadDef (thread3, osPriorityBelowNormal, 1, 100);

osThreadId tid_thread3;                                          // thread id
void task3(void);

/*! \fn int Init_thread3 (void)
    \brief Initializing thread 3
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
	
  while (1) {
    task3(); // thread code 
		
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
}
