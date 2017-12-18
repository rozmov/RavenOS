/*! \file trace.c
    \brief This file defines tracing interfaces and tables
    \details Provides tracing infrastructure for the RTOS
*/

#include "stdio.h"
#include <string.h>
#include <stdint.h>
#include "trace.h"
#include "CU_TM4C123.h"
#include "osObjects.h"

//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
//
//      <o> Maximum size of the buffer
//              <20=> 20
//              <30=> 30
//              <40=> 40
//              <50=> 50
//              <75=> 75
//              <100=> 100
//          <i> Specifies the maximum number of messages to be traced until trace is dumped.
//
#define MAX_TRACE_ARR_LEN (50)  ///< Maximum size of the message buffer
//
//      <s.16> Trace Stop Marker
//          <i> Marker in the trace array to indicate stop of tracing.
//              Used by the thread doing the trace dump to know when
//              to stop after own dump if no other thread uses the trace.
//
#define TRACE_MARKER_STOP ("TRACE STOP")
// <<< end of configuration section >>>

#define MIN_TRACE_ARR_LEN (0)  ///< Minimum size of the message buffer

typedef struct {
  char trace[MAX_STR_LEN];
} trace_t;

trace_t trace_table[MAX_TRACE_ARR_LEN]; ///< The message table.
uint32_t trace_counter = MIN_TRACE_ARR_LEN;  ///< The message table counter.

uint32_t removeTrace(void);

/*!
    \brief Getter function for the current trace counter
    \return Returns current trace counter.
*/
uint32_t getTraceCounter(void)
{
  return trace_counter;
}

/*!
    \brief Increment trace counter if trace table not full
    \return Returns \ref TRACE_OK if successful
                    \ref TRACE_ERROR if error
                    \ref TRACE_FULL if no room in trace array
*/
uint32_t incrementTraceCounter(void)
{
  trace_counter++;

  /* Reached the end of the trace array */
  if (trace_counter == MAX_TRACE_ARR_LEN)
  {
    return TRACE_FULL;
  }

  return TRACE_OK;
}

/*!
    \brief Decrement trace counter if trace table not empty
    \return Returns \ref TRACE_OK if successful and \ref TRACE_ERROR  otherwise.
*/
uint32_t decrementTraceCounter(void)
{
  if (trace_counter == MIN_TRACE_ARR_LEN)
  {
    return TRACE_ERROR;
  }

  trace_counter--;
  return TRACE_OK;
}

/*!
    \brief Clear top of stack (aka last) trace in trace table
    \return Returns \ref TRACE_OK if successful and \ref TRACE_ERROR otherwise.
*/
uint32_t clearTrace(uint32_t trace)
{
  //if (trace < MIN_TRACE_ARR_LEN) return TRACE_ERROR; // trace out of bounds
  if (trace >= MAX_TRACE_ARR_LEN) return TRACE_ERROR; // trace out of bounds

  memset(trace_table[trace].trace, 0, sizeof(trace_table[trace].trace));

  return TRACE_OK;
}

/*!
    \brief Add message to the trace table
    \param message Message to be added to the trace table, up to \ref MAX_STR_LEN characters
    \param length  The size of the message (including the null character)
    \return Returns \ref TRACE_OK if successful and \ref TRACE_ERROR otherwise.
*/
uint32_t addTrace(const char * message, uint32_t length)
{
  uint32_t trace_counter;

  if (length > strlen(message))
  {
    length = strlen(message) + 1;
  }

  /// If we get here, we just add the trace,
  /// clean up any stop markers left for trace worker threads
  if (isTraceMarked() != 0) removeTrace();

  trace_counter = getTraceCounter();

  /// The length of a message needs to be small enough to leave room for
  /// a null string termination (truncating if needed)
  if ( length >= sizeof(trace_table[trace_counter].trace) )
  {
    length = (sizeof(trace_table[trace_counter].trace)-1);
  }

  /* Just in case */
  clearTrace(trace_counter);

  strncpy(trace_table[trace_counter].trace, message, length);
  trace_table[trace_counter].trace[length] = '\0';

  if (incrementTraceCounter() != TRACE_OK)
  {
    return TRACE_ERROR;
  }

  return TRACE_OK;
}

/*!
    \brief Remove message to the trace table
    \param message Message to be added to the trace table, up to \ref MAX_STR_LEN characters
    \return Returns \ref TRACE_OK if successful and \ref TRACE_ERROR otherwise.
*/
uint32_t removeTrace()
{
  uint32_t trace_counter;

  trace_counter = getTraceCounter();
  if (trace_counter == MIN_TRACE_ARR_LEN) return TRACE_OK; // nothing to remove, trace array is empty

  clearTrace(trace_counter-1);

  if (decrementTraceCounter() != TRACE_OK)
  {
    return TRACE_ERROR;
  }

  return TRACE_OK;
}

/*!
    \brief Dump messages from the trace table to output
*/
void dumpTrace(void)
{
  uint32_t idx = 0;

  while (decrementTraceCounter() == TRACE_OK)
  {
    printf("%s\n\r", trace_table[idx].trace);
    clearTrace(idx);
    idx++;
  }
}

/*!
    \brief Check whether the tracing is on or not
    \return Returns 1 if last trace represents the stop marker and 0 otherwise.
*/
uint8_t isTraceMarked(void)
{
  uint32_t trace_counter;

  trace_counter = getTraceCounter();

  if (trace_counter == MIN_TRACE_ARR_LEN) return 0;

  if (strcmp(trace_table[trace_counter-1].trace, TRACE_MARKER_STOP) == 0)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

/*!
    \brief Set whether the tracing is on or not
    \param set Set the marker to stop tracing or remove
*/
void markTrace(uint8_t set)
{
  if (set == 0)
  {
    if (isTraceMarked() != 0)
      if (removeTrace() != TRACE_OK)
      {
        return;
      }
  }
  else
  {
    if (isTraceMarked() == 0) ADD_TRACE(TRACE_MARKER_STOP);
  }
}
