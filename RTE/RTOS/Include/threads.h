#ifndef _THREADS_H
#define _THREADS_H

#include <stdint.h>

/// \def MAX_THREADS Maximum number for threads supported
#define MAX_THREADS 10
#define DEFAULT_STACK_SIZE 128 ///< Default Stack Size foa given Thread

/// Thread Status : Running, Blocked or Asleep.
typedef enum os_thread_status 
{
	TH_RUNNING,
	TH_READY,
	TH_BLOCKED,
	TH_ASLEEP
} osThreadStatus;

extern uint32_t rtr_q_h;
extern uint32_t rtr_q_idx;

#endif // _THREADS_H

