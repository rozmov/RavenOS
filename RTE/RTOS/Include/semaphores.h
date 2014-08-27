/*! \file semaphores.h
    \brief This header file defines all semaphore related data
		\details Defines the maximum number of threads a semaphore can support and the maximum numner of semaphores supported.
*/

#ifndef _SEMAPHORES_H
#define _SEMAPHORES_H

#include <stdint.h> 

//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
//--------------------- Semaphore Configuration ----------------------------------
//
//      <o> Number of Threads per Semaphore
//              <1=> 1
//              <2=> 2
//              <3=> 3
//              <4=> 4
//              <5=> 5
//              <6=> 6
//              <7=> 7
//              <8=> 8
//              <9=> 9
//              <10=> 10
//          <i> Specifies the maximum number of threads that can be blocked on or own a semaphore.
//
#define MAX_THREADS_SEM 10 ///< The maximum number of threads that can be blocked or own on a semaphore.

#endif
