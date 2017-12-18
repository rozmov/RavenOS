/*! \file thread3.c
    \brief A thread that prints trace information to UART.
    \details Initialize and implement thread
*/

#include <string.h>
#include "stdio.h"
#include "trace.h"
#include "osObjects.h"

osThreadDef (thread3, osPriorityLow, 1, 100);  ///< thread definition

osThreadId tid_thread3;   ///< thread3 id
void task3(void);

/*!
    \brief Initializing thread 3
    \return 0=successful; -1=failure
*/
int Init_thread3 (void)
{
  ADD_TRACE("thread3 init");

  tid_thread3 = osThreadCreate (osThread(thread3), NULL);
  if(!tid_thread3) return(-1);

  return(0);
}

/*!
    \brief Terminating thread3
    \return 0=successful; -1=failure
*/
int Terminate_thread3 (void)
{
  dumpTraceProtected();

  if (osThreadTerminate(tid_thread3) != osOK)
  {
    return(-1);
  }
  return(0);
}

/*!
    \brief Thread definition for thread 3.
    \param argument A pointer to the list of arguments.
*/
void thread3 (void const *argument)
{
  ADD_TRACE_PROTECTED("thread3 start run");

  while (1)
  {
    task3(); // thread code

    // give a chance to the other tasks to run now
    if (isTraceMarkedProtected() == 0) ADD_TRACE_PROTECTED("thread3 set priority to osPriorityLow");

    if (osThreadSetPriority(osThreadGetId(), osPriorityLow) != osOK)
    {
      if (isTraceMarkedProtected() == 0) ADD_TRACE_PROTECTED("thread3 set priority to osPriorityLow fail");
    }
    else
    {
      if (isTraceMarkedProtected() == 0) ADD_TRACE_PROTECTED("thread3 back from set priority to osPriorityLow");
    }

    if (isTraceMarkedProtected() == 0) ADD_TRACE_PROTECTED("thread3 yields");

    if ( osThreadYield() != osOK)
    {
      if (isTraceMarkedProtected() == 0) ADD_TRACE_PROTECTED("thread3 yields fail");
    }
    else
    {
      if (isTraceMarkedProtected() == 0) ADD_TRACE_PROTECTED("thread3 back from yield");
    }

    if (isTraceMarkedProtected() == 0)
    {
      markTraceProtected(1);
    }

//    while (getTraceCounter() <= TRACE_TROTTLE)
//    {
//      ADD_TRACE_PROTECTED("thread3 yields");
//
//      if ( osThreadYield() != osOK)
//      {
//        ADD_TRACE_PROTECTED("thread3 yields fail");
//      }
//      else
//      {
//        ADD_TRACE_PROTECTED("thread3 back from yield");
//      }
//    }
  }
}

// -------------------------------------------------------------------------
/*!
    \brief Print trace information to UART
*/
void task3(void)
{
  if (isTraceMarkedProtected() == 0)
  {
    /// Print trace information
    ADD_TRACE_PROTECTED("thread3 dump trace");
    dumpTraceProtected();
  }
}
