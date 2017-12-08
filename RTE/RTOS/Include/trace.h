/*! \file trace.h
    \brief This header file defines visible tracing infrastructures
	  \details Provides tracing infrastructure for the RTOS
*/

#ifndef _TRACE_H
#define _TRACE_H

#include <stdint.h>

//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
//--------------------- Trace Configuration ----------------------------------
//
//      <o> Maximum Supported String Length for the Trace Message
//              <16=> 16
//              <32=> 32
//              <64=> 64
//              <128=> 128
//              <256=> 256
//              <512=> 512
//          <i> Specifies the maximum length of the string that can be handled by the tracing mechanism.
//
#define MAX_STR_LEN       (64)  ///< Maximum length of a trace message

#define TRACE_OK    (0) ///< Trace return code - success
#define TRACE_ERROR (1) ///< Trace return code - fail

#define TRACE_TROTTLE (4) ///< throttle mechanism in place to stop thread from running 
                          /// if no other traces added but its own

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
	  /* nothing to do */	\
	} \
	else if (addTraceProtected(message, strlen(message)) != TRACE_OK) \
	{ \
		stop_cpu; \
	} \
}

//#define ADD_TRACE_PROTECTED2(content, ...) \
//{ \
//	char      message[MAX_STR_LEN]; \
//	int       print_rc; \
//	\
//	memset(message, 0, sizeof(message));			\
//	print_rc = snprintf(message, sizeof(message), content, __VA_ARGS__); \
//							\
//	if (print_rc > 0) \
//	{				\
//		if (print_rc < sizeof(message)) \
//		{ \
//			message[MAX_STR_LEN - 1] = '\0'; \
//		} \
//		\
//		if (addTraceProtected(message, strlen(message)) != TRACE_OK) \
//		{ \
//			stop_cpu; \
//		}		\
//	}	\
//}

#define ADD_TRACE(content) \
{ \
	const char init[] = content; \
	\
	if (addTrace(init, strlen(init)) != TRACE_OK) \
	{ \
		stop_cpu; \
	}	\
}

uint32_t addTraceProtected(const char * message, uint32_t length);
void dumpTraceProtected(void);

uint32_t addTrace(const char * message, uint32_t length);
void dumpTrace(void);
uint32_t getTraceCounter(void);

#endif // _TRACE_H
