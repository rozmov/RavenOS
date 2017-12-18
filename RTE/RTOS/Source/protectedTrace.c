/*! \file protectedTrace.c
    \brief This file defines thread-safe tracing interfaces
    \details Provides thread-safe tracing infrastructure for the RTOS based on regular tracing
*/

#include "stdio.h"
#include <string.h>
#include <stdint.h>
#include "trace.h"
#include "osObjects.h"

/*!
    \brief Thread-safe add message to the trace table
    \param message Message to be added to the trace table, up to \ref MAX_STR_LEN characters
    \return Returns \ref TRACE_OK if successful and \ref TRACE_ERROR otherwise.
*/
uint32_t addTraceProtected(const char * message, uint32_t length)
{
  const char mess[] = "release sem1 fail";

  if (TRACE_FLAG != 1)
  {
    return TRACE_OK;
  }

  if ( osSemaphoreWait (sid_Semaphore1, osWaitForever) != -1 ) // wait forever
  {
    if (addTrace(message, length) != TRACE_OK)
    {
      stop_cpu;
    }

    if (osSemaphoreRelease (sid_Semaphore1) != osOK)
    {
      if (addTrace(mess, strlen(mess)+1) != TRACE_OK)
      {
        stop_cpu;
      }
    }
  }
  else
  {
      stop_cpu;
  }

  return TRACE_OK;
}

/*!
    \brief Dump messages from the trace table to output
*/
void dumpTraceProtected(void)
{
  const char mess[] = "release sem1 fail";

  if (TRACE_FLAG != 1)
  {
    return ;
  }

  if ( osSemaphoreWait (sid_Semaphore1, osWaitForever) != -1 ) // wait forever
  {
    dumpTrace();

    if (osSemaphoreRelease (sid_Semaphore1) != osOK)
    {
      if (addTrace(mess, strlen(mess)+1) != TRACE_OK)
      {
        stop_cpu;
      }
    }
  }
  else
  {
    stop_cpu;
  }

  return;
}

uint8_t isTraceMarkedProtected(void)
{
  const char mess[] = "release sem1 fail";
  uint8_t rc;

  if (TRACE_FLAG != 1)
  {
    return 0;
  }

  if ( osSemaphoreWait (sid_Semaphore1, osWaitForever) != -1 ) // wait forever
  {
    rc = isTraceMarked();

    if (osSemaphoreRelease (sid_Semaphore1) != osOK)
    {
      if (addTrace(mess, strlen(mess)+1) != TRACE_OK)
      {
        stop_cpu;
      }
    }
  }
  else
  {
    stop_cpu;
  }

  return rc;
}

void markTraceProtected(uint8_t set)
{
  const char mess[] = "release sem1 fail";

  if (TRACE_FLAG != 1)
  {
    return ;
  }

  if ( osSemaphoreWait (sid_Semaphore1, osWaitForever) != -1 ) // wait forever
  {
    markTrace(set);

    if (osSemaphoreRelease (sid_Semaphore1) != osOK)
    {
      if (addTrace(mess, strlen(mess)+1) != TRACE_OK)
      {
        stop_cpu;
      }
    }
  }
  else
  {
    stop_cpu;
  }

  return;
}
