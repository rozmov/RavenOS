/*! \file thread2.c
    \brief A thread that toggles the BLUE LED.
    \details Initialize and implement thread
*/

#include <string.h>
#include "peripherals.h"
#include "osObjects.h"
#include "trace.h"

osThreadDef (thread2, osPriorityBelowNormal, 1, 100);  ///< thread definition

osThreadId tid_thread2;                                ///< thread id
void task2(void);

/*!
    \brief Initializing thread2
*/
int Init_thread2 (void)
{
  ADD_TRACE("thread2 init") ;

  tid_thread2 = osThreadCreate (osThread(thread2), NULL);
  if(!tid_thread2) return(-1);

  return(0);
}

/*!
    \brief Terminating thread2
*/
int Terminate_thread2 (void)
{
  ADD_TRACE_PROTECTED("thread2 terminate attempt ") ;

  if (osThreadTerminate(tid_thread2) != osOK)
  {
    ADD_TRACE_PROTECTED("could not terminate thread2") ;
    return(-1);
  }

  return(0);
}

/*!
    \brief Thread definition for thread 2.
    \param argument A pointer to the list of arguments.
*/
void thread2 (void const *argument)
{
  ADD_TRACE_PROTECTED("thread2 start run") ;

  while (1)
  {
    ADD_TRACE_PROTECTED("thread2 take sem0 attempt") ;

    if ( osSemaphoreWait (sid_Semaphore0, osWaitForever) != -1 ) // wait forever
    {
      ADD_TRACE_PROTECTED("thread2 take sem0 success") ;

      task2(); // thread code
      count1Sec();
      task2();

      ADD_TRACE_PROTECTED("thread2 release sem0 attempt") ;

      if (osSemaphoreRelease (sid_Semaphore0) != osOK)
      {
        ADD_TRACE_PROTECTED("thread2 release sem0 fail") ;
      }
    }
    else
    {
      ADD_TRACE_PROTECTED("thread2 take sem0 fail") ;
    }

    ADD_TRACE_PROTECTED("thread2 set priority to osPriorityBelowNormal") ;
    if (osThreadSetPriority(osThreadGetId(), osPriorityBelowNormal) != osOK)
    {
      ADD_TRACE_PROTECTED("thread2 set priority fail") ;
    }

    // suspend thread
    ADD_TRACE_PROTECTED("thread2 yields") ;
    if (osThreadYield() != osOK)
    {
      ADD_TRACE_PROTECTED("thread2 yields fail") ;
    }

    ADD_TRACE_PROTECTED("thread2 back from yield") ;

    // This should terminate the current thread2 thread
    if (Terminate_thread2() != 0)
    {
      stop_cpu;
    }
  }
}

// -------------------------------------------------------------------------
/*!
    \brief Toggle LED #2
*/
void task2(void)
{
  ADD_TRACE_PROTECTED("thread2 flips LED") ;
  LED_blink(LED2);
}
