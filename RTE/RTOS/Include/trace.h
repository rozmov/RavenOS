/*! \file trace.h
    \brief This header file defines visible tracing infrastructures
    \details Provides tracing infrastructure for the RTOS
*/

#ifndef _TRACE_H
#define _TRACE_H

#include <stdint.h>

//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
// <h> --------------------- Trace Configuration ----------------------------------
//
//      <o0> Maximum Supported String Length for the Trace Message
//              <16=> 16
//              <32=> 32
//              <64=> 64
//              <128=> 128
//              <256=> 256
//              <512=> 512
//          <i> Specifies the maximum length of the string that can be handled by the tracing mechanism.
//
//      <o1> Maximum Trace Throttle for the Trace Message Dump
//              <0-49>
//          <i> Specifies the minimum amount of traces before the trace is dumped to UART
//
// </h>
//
// <h> --------------------- Protected Trace Configuration ------------------------
//
//       <q2> Trace Enable
//          <i>  Uncheck this box to skip the trace printing.
//
// </h>
#define MAX_STR_LEN (32)  ///< Maximum length of a trace message

#define TRACE_TROTTLE (5) ///< throttle mechanism in place to stop thread from running
                          /// if no other traces added but its own

# define TRACE_FLAG (1)   ///< trace printing flag: 1 = print traces; 0 = do no print traces
// <<< end of configuration section >>>

#define TRACE_OK    (0) ///< Trace return code - success
#define TRACE_ERROR (1) ///< Trace return code - fail
#define TRACE_FULL  (2) ///< Trace return code - trace array is full (would need to dump or clear)

#define ADD_TRACE_PROTECTED(content) \
{ \
  char      message[MAX_STR_LEN]; \
  char     *message_ptr; \
  \
  memset(message, 0, sizeof(message)); \
  message_ptr = strncpy(message, content, sizeof(message)); \
  \
  if (message_ptr == NULL) \
  { \
    /* nothing to do */  \
  } \
  else if (addTraceProtected(message, strlen(message)) != TRACE_OK) \
  { \
    stop_cpu; \
  } \
}

#define ADD_TRACE(content) \
{ \
  const char init[] = content; \
  \
  if (addTrace(init, strlen(init)) != TRACE_OK) \
  { \
    stop_cpu; \
  }  \
}

uint32_t addTraceProtected(const char * message, uint32_t length);
void dumpTraceProtected(void);
uint8_t isTraceMarkedProtected(void);
void markTraceProtected(uint8_t set);

uint32_t addTrace(const char * message, uint32_t length);
void dumpTrace(void);
uint32_t getTraceCounter(void);
uint8_t isTraceMarked(void);
void markTrace(uint8_t set);

#endif // _TRACE_H
