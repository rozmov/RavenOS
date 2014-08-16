/*! \file osObjects.h
    \brief This header file defines all objects when included in a C/C++ source file
		\details Defines the stop_cpu as a breakpoint, threads, semaphores.
*/

#ifndef _OSOBJECTS_H
#define _OSOBJECTS_H

#include "cmsis_os.h"                                   // CMSIS RTOS header file

/*! \def stop_cpu
     Use Breakpoint to stop when error is detected (KEIL MDK specific intrinsic) */
/*! It can be changed to while(1) if needed */
#define stop_cpu   __breakpoint(0)

/// Thread definition
extern void thread0 (void const *argument);             // function prototype
int Init_thread0 (void);

extern void thread1 (void const *argument);             // function prototype
int Init_thread1 (void);

extern void thread2 (void const *argument);             // function prototype
int Init_thread2 (void);

extern void thread3 (void const *argument);             // function prototype
int Init_thread3 (void);

/// Semaphore definition
extern void sem0 (void const *argument);             // function prototype
int Init_Semaphore0 (void);

#endif // _OSOBJECTS_H
