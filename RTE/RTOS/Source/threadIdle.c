/*! \file threadIdle.c
    \brief Idle thread.
    \details Initialize and implement idle thread. Which should be scheduled even if other threads are running.
             The Idle Thread does not support termination. It is created by the kernel at initialization and will run as long as the kernel does.
*/

#include <string.h>
#include "osObjects.h"
#include "trace.h"
#include "threadIdle.h"

osThreadDef (threadIdle, osPriorityIdle, 1, 100);  ///< thread definition
osThreadId tid_threadIdle;                         ///< thread id

void work(void);

/*! \fn int Init_threadIdle (void)
    \brief Initializing Idle thread
*/
int Init_threadIdle (void)
{
  ADD_TRACE("threadIdle init") ;

  tid_threadIdle = osThreadCreate (osThread(threadIdle), NULL);
  if(!tid_threadIdle) return(-1);

  return(0);
}

/*! \fn void threadIdle (void const *argument)
    \brief Thread definition for the idle thread.
    \param argument A pointer to the list of arguments.
*/
void threadIdle (void const *argument)
{
  if (isTraceMarkedProtected() == 0) ADD_TRACE_PROTECTED("threadIdle start run") ;

  while (1)
  {
    work();

    // suspend thread
    if (isTraceMarkedProtected() == 0) ADD_TRACE_PROTECTED("threadIdle yields");
    if (osThreadYield() != osOK)
    {
      if (isTraceMarkedProtected() == 0) ADD_TRACE_PROTECTED("threadIdle yield fail") ;
    }

//    // throttle mechanism in place to stop this thread from running if no other traces added but its own
//    // if other work scheduled, this mechanism can be removed or adapted as necessary
//    while (getTraceCounter() <= TRACE_TROTTLE)
//    {
//      if (osThreadYield()!= osOK)
//      {
//        ADD_TRACE_PROTECTED("threadIdle yield while fail") ;
//      }
//    }

    if (isTraceMarkedProtected() == 0) ADD_TRACE_PROTECTED("threadIdle back from yield") ;

    if (isTraceMarkedProtected() == 0)
    {
      markTraceProtected(1);
    }
  }
}

/*!
    \brief Perform any work scheduled for the background
*/
void work(void)
{
  /// Print trace information
  if (isTraceMarkedProtected() == 0) ADD_TRACE_PROTECTED("threadIdle dump trace") ;
  dumpTraceProtected();
}
