/*! \file threads.h
    \brief This header file defines thread related data
		\details Defines the maximum supported threads, default stack size, thread FSM states 
*/

#ifndef _THREADS_H
#define _THREADS_H

#include <stdint.h>

//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
//--------------------- Threads Configuration ----------------------------------
//
//      <o> Number of Threads used at Any Given Time in the OS
//              <1=> 0: (Idle Thread)
//              <2=> 1
//              <3=> 2
//              <4=> 3
//              <5=> 4
//              <6=> 5
//              <7=> 6
//              <8=> 7
//              <9=> 8
//              <10=> 9
//              <11=> 10
//          <i> Specifies the maximum number of threads supported
//
#define MAX_THREADS 7         ///< Maximum number of threads supported
//
//      <o> Default Stack Size for a Given Thread (Bytes)
//              <100=> 100 Bytes
//              <200=> 200 Bytes
//              <300=> 300 Bytes
//              <400=> 400 Bytes
//              <500=> 500 Bytes
//              <600=> 600 Bytes
//              <700=> 700 Bytes
//              <800=> 800 Bytes
//              <900=> 900 Bytes
//              <1000=> 1000 Bytes
//          <i> Specifies the Default Stack Size for a given Thread
//
#define DEFAULT_STACK_SIZE 200 ///< Default Stack Size for a given Thread

typedef enum os_thread_status ///< Thread Status : Running, Blocked or Asleep.
{
	TH_RUNNING,         ///< Thread "Runnning" state, this is the currently running process
	TH_READY,           ///< Thread "Ready" state, the process is in the Ready-to-Run queue
	TH_BLOCKED,         ///< Thread "Blocked" state, the process is blocked on a semaphore
	TH_ASLEEP,          ///< Thread "Asleep" state, the process has finished processing this period and is currently dormant
	TH_DEAD             ///< Thread "Dead" state, the process has been terminated
} osThreadStatus;

#endif // _THREADS_H

