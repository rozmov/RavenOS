/*! \file threads.h
    \brief This header file defines thread related data
		\details Defines the maximum supported threads, default stack size, thread FSM states 
*/

#ifndef _THREADS_H
#define _THREADS_H

#include <stdint.h>

#define MAX_THREADS 10         ///< Maximum number of threads supported
#define DEFAULT_STACK_SIZE 128 ///< Default Stack Size for a given Thread

/// \enum os_thread_status Thread Status : Running, Blocked or Asleep.
typedef enum os_thread_status 
{
	TH_RUNNING,         ///< Thread "Runnning" state, this is the currently running process
	TH_READY,           ///< Thread "Ready" state, the process is in the Ready-to-Run queue
	TH_BLOCKED,         ///< Thread "Blocked" state, the process is blocked on a semaphore
	TH_ASLEEP,          ///< Thread "Asleep" state, the process has finished processing this period and is currently dormant
	TH_DEAD             ///< Thread "Dead" state, the process has been terminated
} osThreadStatus;

#endif // _THREADS_H

